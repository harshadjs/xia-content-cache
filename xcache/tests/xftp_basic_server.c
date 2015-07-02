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
#include "../api/xcache.h"
#include "Xsocket.h"
#include "dagaddr.hpp"
#include <assert.h>

#define VERSION "v1.0"
#define TITLE "XIA Basic FTP Server"

#define MAX_XID_SIZE 100
// #define DAG  "RE %s %s %s"
#define SID "SID:00000000dd41b924c1001cfa1e1117a812492434"
#define NAME "www_s.basicftp.aaa.xia"

#define CHUNKSIZE 1024
#define NUM_CHUNKS	10
#define REREQUEST 3

int verbose = 1;
char myAD[MAX_XID_SIZE];
char myHID[MAX_XID_SIZE];
char my4ID[MAX_XID_SIZE];


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

void *recvCmd(void *socketid)
{
	int i, n, count = 0;
	struct xcacheChunk *info = NULL;
	char command[XIA_MAXBUF];
	char reply[XIA_MAXBUF];
	int sock = *((int*)socketid);
	char *fname;
	char fin[512], fout[512];
//  	char **params;
	char ad[MAX_XID_SIZE];
	char hid[MAX_XID_SIZE];
    struct xcacheSlice slice;

	//ChunkContext contains size, ttl, policy, and contextID which for now is PID
    if(XcacheAllocateSlice(&slice, 20000000, 1000, 0) < 0)
      die(-2, "Unable to allocate slice\n");

	while (1) {
      say("waiting for command\n");
		
      memset(command, '\0', strlen(command));
      memset(reply, '\0', strlen(reply));
		
      if ((n = Xrecv(sock, command, 1024, 0))  < 0) {
        warn("socket error while waiting for data, closing connection\n");
        break;
      }
      //Sender does the chunking and then should start the sending commands
      if (strncmp(command, "get", 3) == 0) {
        fname = &command[4];
        say("client requested file %s\n", fname);

        if (info) {
          // clean up the existing chunks first
        }
		
        info = NULL;
        count = 0;
			
        say("chunking file %s\n", fname);
			
        //Chunking is done by the XputFile which itself uses XputChunk, and fills out the info
        if ((count = XcachePutFile(&slice, fname, CHUNKSIZE, &info)) < 0) {
          warn("unable to serve the file: %s\n", fname);
          sprintf(reply, "FAIL: File (%s) not found", fname);
        } else {
          sprintf(reply, "OK: %d", count);
        }
        say("%s\n", reply);
			
			
        //Just tells the receiver how many chunks it should expect.
        if (Xsend(sock, reply, strlen(reply), 0) < 0) {
          warn("unable to send reply to client\n");
          break;
        }
		//Sending the blocks cids
      }
      else if (strncmp(command, "block", 5) == 0) {
        char *start = &command[6];
        char *end = strchr(start, ':');
        if (!end) {
          // we have an invalid command, return error to client
          sprintf(reply, "FAIL: invalid block command");
        } else {
          *end = 0;
          end++;
          // FIXME: clean up naming, e is now a count
          int s = atoi(start);
          int e = s + atoi(end);
          strcpy(reply, "OK:");
          for(i = s; i < e && i < count; i++) {
            strcat(reply, " ");
            strcat(reply, info[i].cid);
          }
        }
        printf("%s\n", reply);
        if (Xsend(sock, reply, strlen(reply), 0) < 0) {
          warn("unable to send reply to client\n");
          break;
        }

      }
      else if (strncmp(command, "done", 4) == 0) {
        say("done sending file: removing the chunks from the cache\n");
        /* for (i = 0; i < count; i++) */
        /*   XremoveChunk(ctx, info[i].cid); */
        /* XfreeChunkInfo(info); */
        info = NULL;
        count = 0;
		
      }
	}
	
	/* if (info) */
	/* 	XfreeChunkInfo(info); */
	/* XfreeCacheSlice(ctx); */
	Xclose(sock);
	pthread_exit(NULL);
}


//Just registering the service and openning the necessary sockets
int registerReceiver()
{
  int sock;
  say ("\n%s (%s): started\n", TITLE, VERSION);

  // create a socket, and listen for incoming connections
  if ((sock = Xsocket(AF_XIA, SOCK_STREAM, 0)) < 0)
    die(-1, "Unable to create the listening socket\n");

  // read the localhost AD and HID
  if (XreadLocalHostAddr(sock, myAD, sizeof(myAD), myHID, sizeof(myHID), my4ID, sizeof(my4ID)) < 0 )
    die(-1, "Reading localhost address\n");

  struct addrinfo *ai;
  //FIXME: SID is hardcoded
  if (Xgetaddrinfo(NULL, SID, NULL, &ai) != 0)
    die(-1, "getaddrinfo failure!\n");

  sockaddr_x *dag = (sockaddr_x*)ai->ai_addr;
  //FIXME NAME is hard coded
  if (XregisterName(NAME, dag) < 0 )
    die(-1, "error registering name: %s\n", NAME);

  if (Xbind(sock, (struct sockaddr*)dag, sizeof(dag)) < 0) {
    Xclose(sock);
    die(-1, "Unable to bind to the dag: %s\n", dag);
  }

  Graph g(dag);
  say("listening on dag: %s\n", g.dag_string().c_str());
  return sock;
  
}

void *blockingListener(void *socketid)
{
  int sock = *((int*)socketid);
  int acceptSock;
  while (1) {
    say("Waiting for a client connection\n");
   		
    if ((acceptSock = Xaccept(sock, NULL, NULL)) < 0)
      die(-1, "accept failed\n");

    say("connected\n");
		
    // handle the connection in a new thread
    pthread_t client;
	pthread_create(&client, NULL, recvCmd, (void *)&acceptSock);
  }
	
  Xclose(sock); // we should never reach here!
  return NULL;
}

int main()
{	
	int sock = registerReceiver();

	blockingListener((void *)&sock);

	return 0;
}
