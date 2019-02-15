#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "../include/common.h"
#include "../include/builtin_cmd_handler.h"

/* function to handle cmd pwd*/
void pwd_cmd_handler(char **arr){
        char *buff = calloc(FILENAME_MAX, sizeof (char));
        getcwd(buff, FILENAME_MAX);
        *arr = buff;
}
