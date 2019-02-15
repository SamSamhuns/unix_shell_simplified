#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "../include/common.h"
#include "../include/builtin_cmd_handler.h"

// function to handle exit statements
void exit_cmd_handler(char ** user_input, char ** parsed_input){
        // free the allocated memory that is passed as an arg to this func
        free(*user_input);
        free(*parsed_input);
}
