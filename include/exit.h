#ifndef EXIT_H // Making sure the header files are not included twice
#define EXIT_H
#include "common.h"

/* function to handle cmd exit*/
void exit_cmd_handler(char **user_input_ptr,
                      struct Node *history_head, struct Node *export_head, FILE *fptr);
#endif
