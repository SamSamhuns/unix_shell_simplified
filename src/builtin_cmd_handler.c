#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "../include/builtin_cmd_handler.h"

// function to handle exit statements
void exit_cmd_handler(char ** user_input, char ** parsed_input){
        // free the allocated memory that is passed as an arg to this func
        free(*user_input);
        free(*parsed_input);
}

/* function to handle cmd pwd*/
void pwd_cmd_handler(char **arr){
        char *buff = calloc(FILENAME_MAX, sizeof (char));
        getcwd(buff, FILENAME_MAX);
        *arr = buff;
}

/* function to handle cmd cd*/
void cd_cmd_handler(char *arr){
        chdir(arr);
}

/* function to handle cmd export*/
void export_cmd_handler(void){
}
/* function to handle cmd history*/
void history_cmd_handler(void){
}
