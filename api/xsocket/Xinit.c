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
/*!
 @internal
 @file Xinit.c
 @brief Implements internal support functions
*/

#include <sys/types.h>
#include <unistd.h>

#include "Xsocket.h"
#include "Xinit.h"
#include "Xutil.h"
#include <stdlib.h>
#include "minIni.h"
#include <libgen.h>
#include <limits.h>
#include <pthread.h>
#include <errno.h>

using namespace std;

extern "C" {

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

	/*!
	** @brief Specify the location of the XSockets configuration file.
	**
	** Specifies the name of a config file to read, and (re)loads the
	** conf file.
	**
	** @returns void
	*/
    void set_conf(const char *filename, const char* sectionname)
    {
		pthread_mutex_lock(&lock);

		char root[BUF_SIZE];

		snprintf(__XSocketConf::master_conf, BUF_SIZE, "%s%s", XrootDir(root, BUF_SIZE), "/etc/xsockconf.ini");
        __InitXSocket::read_conf(filename, sectionname);
		__XSocketConf::initialized=1;
		pthread_mutex_unlock(&lock);
    }

} /* extern C */

/*!
** @brief Retreive a pointer to the config settings structure.
**
** Returns a pointer to the conf object containing the addresses of the API
** client's IP address and the IP addresses used for communicating with click.
** When first called, a mutex is locked and the config setting are loaded from
** file. Subsequent calls do not incur the overhead of a mutex operation.
**
** @returns pointer to the gobal XSocketConf structure
*/
struct __XSocketConf* get_conf()
{

	if (__XSocketConf::initialized == 0) {

		pthread_mutex_lock(&lock);

		if (__XSocketConf::initialized == 0) {
			__InitXSocket();
		}

		__XSocketConf::initialized=1;
		pthread_mutex_unlock(&lock);
	}
	return &_conf;
}

/*!
** @brief constructor for the Xsockets library config settings.
**
** Creates the config object and loads the settings from the config file.
**
** NOTE: document the conf file format
**
*/
__InitXSocket::__InitXSocket()
{
	const char *inifile = getenv("XSOCKCONF");

	memset(_conf.click_port, 0, __PORT_LEN);

	if (inifile==NULL) {
		inifile = "xsockconf.ini";
	}
	const char *section_name = NULL;
	char buf[PATH_MAX+1];
	int rc;

	char root[BUF_SIZE];
	snprintf(__XSocketConf::master_conf, BUF_SIZE, "%s%s", XrootDir(root, BUF_SIZE), "/etc/xsockconf.ini");

	if ((rc = readlink("/proc/self/exe", buf, sizeof(buf) - 1)) != -1) {
		section_name = basename(buf);
		buf[rc] = 0;
	}

	const char * section_name_env  = getenv("XSOCKCONF_SECTION");
	if (section_name_env)
		section_name = section_name_env;

	// NOTE: unlikely, but what happens if section_name is NULL?
	read_conf(inifile, section_name);
}

/*!
** @brief loads the specified config file and section into the global config object
**
** @warning As currently implemented, this is not thread safe if called directly.
**
** @returns void
*/
void __InitXSocket::read_conf(const char *inifile, const char *section_name)
{
	char host[256];

	if (ini_gets(section_name, "host", "", host, sizeof(host), inifile) > 0) {
		// local ini file specified a host entry in the master
		// look for the specified host entry in the master conf file, and return the default port if not found
		ini_gets(host, "click_port", DEFAULT_CLICKPORT, _conf.click_port, __PORT_LEN , __XSocketConf::master_conf);

	} else if (ini_gets(section_name, "click_port", "", _conf.click_port, __PORT_LEN , inifile) == 0) {
		// look for a port entry under the section specified in the local ini file
		// if not found, look for that section in the master ini file
		ini_gets(section_name, "click_port", DEFAULT_CLICKPORT, _conf.click_port, __PORT_LEN , __XSocketConf::master_conf);
  	}
}

struct __XSocketConf _conf;

/*!
** @brief Print the contents of the configuration block.
**
** C helper function that calls the __InitXSocket::print_conf() routine.
**
** @returns void
*/
void print_conf()
{
	__InitXSocket::print_conf();
}

/*!
** @brief print the loaded settings for the click communications channel
**
** Prints the address of the XAPI side IP address as well as the IP addresses
** used by click for the control and data channels.
**
** @returns void
*/
void __InitXSocket::print_conf()
{
  printf("click_port %s\n", _conf.click_port);
}
int  __XSocketConf::initialized=0;
char __XSocketConf::master_conf[BUF_SIZE];

