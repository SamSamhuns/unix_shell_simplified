#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "common.h"
#include "builtin_cmd_handler.h"

/* function to handle cmd pwd*/
void pwd_cmd_handler(){
        if (DEBUG==1)
          printf("6) %d %d \n", FILENAME_MAX, MAX_CMD_INPUT_BUFFER);
        char pwd_buff[FILENAME_MAX];
        getcwd(pwd_buff, FILENAME_MAX);
        fprintf(stdout, "%s\n",pwd_buff);
}
