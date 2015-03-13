#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cli.h"

static void handle_cmd_list(char *token, char *command, char *save_ptr)
{
	if(!strcmp(token, "stores")) {
		xcore_list_stores();
	} else if (!strcmp(token, "policies")) {
//		cli_list_policies();
	} else if (!strcmp(token, "meta")) {
		xctrl_cli_list_meta();
	} else {
		printf("List supports: stores, policies, meta\n");
	}
}

void handle_command(char *command)
{
	char *save_ptr, *token;

	token = strtok_r(command, " \n", &save_ptr);
	if(!token)
		return;

	if(!strcmp(token, "help")) {
		printf("Following are the available commands:\n");
	} else if (!strcmp(token, "list")) {
		token = strtok_r(NULL, " \n", &save_ptr);
		if(!token) {
			printf("List what?\n");
		} else {
			handle_cmd_list(token, command, save_ptr);
		}
	} else if (!strcmp(token, "quit")) {
		exit(0);
	} else {
		printf("Unknown command received: %s. Type help to see available commands.\n", command);
	}
}

