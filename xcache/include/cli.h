#ifndef __CLI_H__
#define __CLI_H__

void handle_command(char *command);

void xcore_list_stores(void);
void xctrl_cli_list_meta(void);
void xctrl_export_popularity(char *filename);
void xctrl_export_size(char *filename);
#endif
