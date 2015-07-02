/* ts=4 */
/*
** Copyright 2011 Carnegie Mellon University
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**    http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include "Xsocket.h"
#include "../api/xcache.h"
#include "dagaddr.hpp"
#include <assert.h>

#define MAX_XID_SIZE 100
#define VERSION "v1.0"
#define TITLE "XIA Basic FTP client"
#define NAME "www_s.basicftp.aaa.xia"
#define CHUNKSIZE 1024
#define REREQUEST 3

#define NUM_CHUNKS	10
#define NUM_PROMPTS	2

// global configuration options
int verbose = 1;
bool quick = false;

char s_ad[MAX_XID_SIZE];
char s_hid[MAX_XID_SIZE];

char my_ad[MAX_XID_SIZE];
char my_hid[MAX_XID_SIZE];

int getFile(int sock, char *p_ad, char* p_hid, const char *fin, const char *fout);


/*
** write the message to stdout unless in quiet mode
*/
void say(const char *fmt, ...)
{
	if (verbose) {
		va_list args;

		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
	}
}

/*
** always write the message to stdout
*/
void warn(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vfprintf(stdout, fmt, args);
	va_end(args);

}

/*
** write the message to stdout, and exit the app
*/
void die(int ecode, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vfprintf(stdout, fmt, args);
	va_end(args);
	fprintf(stdout, "%s: exiting\n", TITLE);
	exit(ecode);
}

int sendCmd(int sock, const char *cmd)
{ 
	int n;
 	warn("Sending Command: %s \n", cmd);
	if ((n = Xsend(sock, cmd,  strlen(cmd), 0)) < 0) {
		Xclose(sock);
		 die(-1, "Unable to communicate\n");
	}

	return n;
}

int getChunkCount(int sock, char *reply, int sz)
{
  int n=-1;

  if ((n = Xrecv(sock, reply, sz, 0))  < 0) {
    Xclose(sock);
    die(-1, "Unable to communicate with the server\n");
  }

  if (strncmp(reply, "OK:", 3) != 0) {
    warn( "%s\n", reply);
    return -1;
  }

  reply[n] = 0;

  return n;
}



int buildChunkDAGs(sockaddr_x addresses[], char *chunks, char *p_ad, char *p_hid, int n_chunks)
{
	char *p = chunks;
	char *next;
	int n = 0;
    Node n_src;
    Node n_ad(Node::XID_TYPE_AD, p_ad);
    Node n_hid(Node::XID_TYPE_HID, p_hid);
	char *dag;
	
	
	// build the list of chunks to retrieve
	while ((next = strchr(p, ' '))) {
		*next = 0;

        Node n_cid(Node::XID_TYPE_CID, p);
        Graph primaryIntent = n_src * n_cid;
        Graph gFallback = n_src * n_ad * n_hid * n_cid;
        Graph gAddr = primaryIntent + gFallback;

        gAddr.fill_sockaddr(&addresses[n]);

		n++;
		p = next + 1;

	}
	dag = (char *)malloc(512);
    Node n_cid(Node::XID_TYPE_CID, p);
    Graph primaryIntent = n_src * n_cid;
    Graph gFallback = n_src * n_ad * n_hid * n_cid;
    Graph gAddr = primaryIntent + gFallback;

    gAddr.fill_sockaddr(&addresses[n]);

	n++;
	return n;
}

int getListedChunks(int csock, FILE *fd, char *chunks, char *p_ad, char *p_hid, int n_chunks)
{
  sockaddr_x chunkAddresses[n_chunks];
  struct xcacheChunk chunk;
  char data[XIA_MAXCHUNK];
  int len;
  int status;
  int n = -1;
	
  n = buildChunkDAGs(chunkAddresses, chunks, p_ad, p_hid, n_chunks);
	
  for(int i = 0; i < n; i++) {
    if(XcacheGetChunk(NULL, &chunk, &chunkAddresses[i], sizeof(chunkAddresses[i]), 0) < 0) {
      die(-1, "XcacheGetChunk Failed\n");
    }
    fwrite(chunk.buf, 1, chunk.len, fd);
  }
  
  return n;
}


//	This is used both to put files and to get files since in case of put I still have to request the file.
//	Should be fixed with push implementation

int getFile(int sock, char *p_ad, char* p_hid, const char *fin, const char *fout)
{
	int chunkSock;
	int offset;
	char cmd[512];
	char reply[512];
	int status = 0;
	
	//TODO: check the arguments to be correct
	
	// send the file request
	sprintf(cmd, "get %s",  fin);
	sendCmd(sock, cmd);

	// get back number of chunks in the file
	if (getChunkCount(sock, reply, sizeof(reply)) < 1){
		warn("could not get chunk count. Aborting. \n");
		return -1;
	}

	int count = atoi(&reply[4]);

	if ((chunkSock = Xsocket(AF_XIA, XSOCK_CHUNK, 0)) < 0)
		die(-1, "unable to create chunk socket\n");

	FILE *f = fopen(fout, "w");

	offset = 0;
	while (offset < count) {
		int num = NUM_CHUNKS;
		if (count - offset < num)
			num = count - offset;

		// tell the server we want a list of <num> cids starting at location <offset>
		sprintf(cmd, "block %d:%d", offset, num);
		sendCmd(sock, cmd);

		if (getChunkCount(sock, reply, sizeof(reply)) < 1){
			warn("could not get chunk count. Aborting. \n");
			return -1;
		}
		offset += NUM_CHUNKS;
		if (getListedChunks(chunkSock, f, &reply[4], p_ad, p_hid, num) < 0) {
			status= -1;
			break;
		}
	}
	
	fclose(f);

	if (status < 0) {
		unlink(fin);
	}

	say("Received file %s\n", fout);
	sendCmd(sock, "done");
	Xclose(chunkSock);
	return status;
}

int initializeClient(const char *name)
{
	int sock, rc;
	sockaddr_x dag;
	socklen_t daglen;
	char sdag[1024];
	char IP[MAX_XID_SIZE];

    // lookup the xia service 
	daglen = sizeof(dag);
    if (XgetDAGbyName(name, &dag, &daglen) < 0)
		die(-1, "unable to locate: %s\n", name);


	// create a socket, and listen for incoming connections
	if ((sock = Xsocket(AF_XIA, SOCK_STREAM, 0)) < 0)
		 die(-1, "Unable to create the listening socket\n");
    
	if (Xconnect(sock, (struct sockaddr*)&dag, daglen) < 0) {
		Xclose(sock);
		 die(-1, "Unable to bind to the dag: %s\n", dag);
	}

	rc = XreadLocalHostAddr(sock, my_ad, MAX_XID_SIZE, my_hid, MAX_XID_SIZE, IP, MAX_XID_SIZE);

	if (rc < 0) {
		Xclose(sock);
		 die(-1, "Unable to read local address.\n");
	} else{
		warn("My AD: %s, My HID: %s\n", my_ad, my_hid);
	}
	
	// save the AD and HID for later. This seems hacky
	// we need to find a better way to deal with this
	Graph g(&dag);
	strncpy(sdag, g.dag_string().c_str(), sizeof(sdag));
//   	say("sdag = %s\n",sdag);
	char *ads = strstr(sdag,"AD:");
	char *hids = strstr(sdag,"HID:");
// 	i = sscanf(ads,"%s",s_ad );
// 	i = sscanf(hids,"%s", s_hid);
	
	if(sscanf(ads,"%s",s_ad ) < 1 || strncmp(s_ad,"AD:", 3) !=0){
		die(-1, "Unable to extract AD.");
	}
		
	if(sscanf(hids,"%s", s_hid) < 1 || strncmp(s_hid,"HID:", 4) !=0 ){
		die(-1, "Unable to extract AD.");
	}

	warn("Service AD: %s, Service HID: %s\n", s_ad, s_hid);
	return sock;
}

void usage(){
	say("usage: get|put <source file> <dest name>\n");
}

bool file_exists(const char * filename)
{
    if (FILE * file = fopen(filename, "r")){
	fclose(file);
	return true;
	}
    return false;
}

int main(int argc, char **argv)
{	

	const char *name;
	int sock = -1;
	char fin[512], fout[512];
	char cmd[512], reply[512];
	int params = -1;
	
	say ("\n%s (%s): started\n", TITLE, VERSION);
	
	if( argc == 1){
		say ("No service name passed, using default: %s\nYou can also pass --quick to execute a couple of default commands for quick testing. Requires s.txt to exist. \n", NAME);
		sock = initializeClient(NAME);
		usage();
	} else if (argc == 2){
		if( strcmp(argv[1], "--quick") == 0){
			quick = true;
			name=NAME;
		} else{
			name = argv[1];
			usage();
		}
		say ("Connecting to: %s\n", name);
		sock = initializeClient(name);

	} else if (argc == 3){
		if( strcmp(argv[1], "--quick") == 0 ){
			quick = true;
			name = argv[2];
			say ("Connecting to: %s\n", name);
			sock = initializeClient(name);
			usage();
		} else{
			die(-1, "xftp [--quick] [SID]");
		}
		
	} else{
		die(-1, "xftp [--quick] [SID]"); 
	}
	
	int i = 0;


	
//		This is for quick testing with a couple of commands

	while(i < NUM_PROMPTS){
		say(">>");
		cmd[0] = '\n';
		fin[0] = '\n';
		fout[0] = '\n';
		params = -1;
		
		if(quick){
			if( i==0 )
				strcpy(cmd, "put s.txt r.txt");
			else if( i==1 )
				strcpy(cmd, "get r.txt sr.txt\n");
			i++;
		}else{
			fgets(cmd, 511, stdin);
		}

//		enable this if you want to limit how many times this is done
// 		i++;
		
		if (strncmp(cmd, "get", 3) == 0){
			params = sscanf(cmd,"get %s %s", fin, fout);
			
			if(params !=2 ){
				sprintf(reply, "FAIL: invalid command (%s)\n", cmd);
				warn(reply);
				usage();
				continue;
			}
			
			if(strcmp(fin, fout) == 0){
				warn("Since both applications write to the same folder (local case) the names should be different.\n");
				continue;
			}
			
			getFile(sock, s_ad, s_hid, fin, fout);
			
		}
	}
	return 1;
}
