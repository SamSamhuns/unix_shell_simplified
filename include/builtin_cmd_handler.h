#ifndef CMD_HANDLER_H // Making sure the header files are not included twice
#define CMD_HANDLER_H
#include "common.h"

void exit_cmd_handler(char **user_input_ptr); /* function to handle cmd exit*/
void pwd_cmd_handler(); /* function to handle cmd pwd*/
void cd_cmd_handler(char *arr); /* function to handle cmd cd*/
void export_cmd_handler(void); /* function to handle cmd export*/
void history_cmd_handler(void); /* function to handle cmd history*/
#endif
