#ifndef CMD_HANDLER_H // Making sure the header files are not included twice
#define CMD_HANDLER_H

#define MAX_CMD_INPUT_BUFFER 100

void exit_cmd_handler(char ** user_input, char ** parsed_input); /* function to handle cmd exit*/
void pwd_cmd_handler(char **arr); /* function to handle cmd pwd*/
void cd_cmd_handler(char *arr); /* function to handle cmd cd*/
void export_cmd_handler(void); /* function to handle cmd export*/
void history_cmd_handler(void); /* function to handle cmd history*/
#endif