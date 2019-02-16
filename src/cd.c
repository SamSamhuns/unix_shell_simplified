#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "common.h"
#include "builtin_cmd_handler.h"

/* function to handle cmd cd*/
void cd_cmd_handler(char *arr){
        if (DEBUG == 1)
          printf("Inside cd_cmd_handler %s\n",arr );
        chdir(arr);
}
