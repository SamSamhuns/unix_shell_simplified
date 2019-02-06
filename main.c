#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmd_handler.h"

#define MAX_CMD_INPUT_BUFFER 100

// function to handle the main loop of the cmd prompt
int main(void){
        char user_input[MAX_CMD_INPUT_BUFFER];

        // main while loop for function
        while (1) {
                printf(">> ");
                scanf("%s", user_input);

                if ( strcmp(user_input, "exit") == 0) {
                        exit_cmd_handler();
                }
        }
        return 0;
}
