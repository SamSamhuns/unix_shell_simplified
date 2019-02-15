#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "../include/common.h"
#include "../include/builtin_cmd_handler.h"

/* function to handle cmd cd*/
void cd_cmd_handler(char *arr){
        chdir(arr);
}
