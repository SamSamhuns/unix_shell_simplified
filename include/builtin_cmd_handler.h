#ifndef CMD_HANDLER_H // Making sure the header files are not included twice
#define CMD_HANDLER_H
#include "common.h"

void exit_cmd_handler(char **user_input_ptr,
                      struct Node *history_head, struct Node *export_head, FILE *fptr); /* function to handle cmd exit*/
void pwd_cmd_handler(); /* function to handle cmd pwd*/
void cd_cmd_handler(char *arr); /* function to handle cmd cd*/
void history_cmd_handler(int number_of_args, struct Node *head); /* function to handle cmd history*/
char *exclaim_cmd_handler(struct Node *head,
                          int arg_order_exclamation); /* run cmd from history selected by !
                                                         returns name of command to run again */
void print_env_export_handler(struct Node *head); /* function to handle cmd export env prints */
/* function to save env vars entered using export returns sthe status of the save env attempt
0=success, -1=failure */
int save_env_export_handler(char *parsed_arr[], int char_arg_len, struct Node *head);

#endif
