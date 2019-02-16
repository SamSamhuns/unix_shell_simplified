#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "common.h"
#include "builtin_cmd_handler.h"

/* function to handle cmd pwd*/
void pwd_cmd_handler(){
        char pwd_buff[FILENAME_MAX];
        getcwd(pwd_buff, FILENAME_MAX);
        fprintf(stdout, "%s\n",pwd_buff);
}
