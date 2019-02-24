#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "common.h"
#include "pwd.h"
#include "export.h"

/* function to handle cmd pwd */
void pwd_cmd_handler( struct Node *head ){
        char pwd_buff[MAX_CMD_INPUT_BUFFER]; // var to hold path of cur directory
        char export_var_pathname_buffer[MAX_CMD_INPUT_BUFFER*2] = "PWD="; // export var buffer
        char *parsed_input[2]; // char ptr array to hold the tokenized export arguments

        getcwd(pwd_buff, MAX_CMD_INPUT_BUFFER);
        fprintf(stdout, "%s\n",pwd_buff);
        strcat(export_var_pathname_buffer, pwd_buff); /* add cur directory path to var holding
          "PWD=" + current_working_dir_path  */

        /* setting parsed_input as being tokenizing with space as the delimiter */
        parsed_input[0] = "export";
        parsed_input[1] = export_var_pathname_buffer;

        /* parsed_input = {"export", "PWD=/path"} */
        save_env_export_handler(parsed_input, 2, head);
}
