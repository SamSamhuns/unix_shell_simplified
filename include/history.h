#ifndef HISTORY_H // Making sure the header files are not included twice
#define HISTORY_H
#include "common.h"

void history_cmd_handler(int number_of_args, struct Node *head); /* function to handle cmd history*/
char *exclaim_cmd_handler(struct Node *head,
                          int arg_order_exclamation); /* run cmd from history selected by !
                                                         returns name of command to run again */
#endif
