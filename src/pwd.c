#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "common.h"
#include "pwd.h"

/* function to handle cmd pwd */
void pwd_cmd_handler(){
	char pwd_buff[MAX_CMD_INPUT_BUFFER];
	getcwd(pwd_buff, MAX_CMD_INPUT_BUFFER);
	fprintf(stdout, "%s\n",pwd_buff);
}
