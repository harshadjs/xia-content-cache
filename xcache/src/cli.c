#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cli.h"

static void handle_cmd_list(char **save_ptr)
{
	char *token = strtok_r(NULL, " \n", save_ptr);

	if(!token) {
		printf("List what?\n");
		return;
	}
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

static void handle_command_export(char **save_ptr)
{
	xctrl_export(save_ptr);
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
		handle_cmd_list(&save_ptr);
	} else if (!strcmp(token, "quit")) {
		exit(0);
	} else if (!strcmp(token, "export")) {
		handle_command_export(&save_ptr);
	} else {
		printf("Unknown command received: %s. Type help to see available commands.\n", command);
	}
}

