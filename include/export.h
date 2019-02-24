#ifndef EXPORT_H // Making sure the header files are not included twice
#define EXPORT_H
#include "common.h"

void print_env_export_handler(struct Node *head); /* function to handle cmd export env prints */
/* function to save env vars entered using export returns sthe status of the save env attempt
0=success, -1=failure */
int save_env_export_handler(char *parsed_arr[], int char_arg_len, struct Node *head);

#endif
