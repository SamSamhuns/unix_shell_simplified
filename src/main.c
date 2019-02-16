/*
   UNIX Shell prototype
   Author Samridha Shrestha
   Feb 2019
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "common.h"
#include "builtin_cmd_handler.h"
#define MAX_INPUT_ARR_LEN 50 /* max number of cmd args entered each time*/

/* Parser function */
int parser(char *user_input, char *parsed_input[], size_t ui_length);

/* function to handle the main loop of the cmd prompt */
int main(void){
        FILE *fptr = fopen("./history.txt", "a"); /* open history to read and save cmds to*/
        char *parsed_input[MAX_INPUT_ARR_LEN]; /* array of char ptrs to hold parsed input*/
        char *user_input = calloc(MAX_CMD_INPUT_BUFFER, sizeof(char));

        // main while loop for shell
        while (1) {
                int char_arg_len = 0;
                // Checking if memory is available in the heap
                if (user_input == NULL) {
                        printf("Out of memory\n" );
                        return 1;
                }
                printf(">> ");

                /* Get user input and auto append \n at end */
                if (fgets(user_input, MAX_CMD_INPUT_BUFFER, stdin) == NULL ) {
                        printf("Error getting user input\n");
                }

                /* if the user just enters a new line char */
                if (!(isalnum(user_input[0])) && strlen(user_input) == 1)  {
                        continue;
                }

                /* Save each entered cmd to a history.txt file */
                fprintf(fptr, "%s", user_input);

                /* Parse the user_input into a array of char ptrs holding user cmd args */
                char_arg_len = parser(&user_input[0], parsed_input, strlen(user_input));

                // Debug info
                if (DEBUG==1) {
                        fprintf(stdout, "3) USER INPUTED %s \nPARSED INPUT[0] %s and LENGTH %lu\n", user_input, parsed_input[0], strlen(parsed_input[0]));

                        fprintf(stdout, "4) parsed_input_array\n" );
                        for (int i = 0; i<char_arg_len; i++) {
                                printf("%s\n",parsed_input[i] );
                        }
                }

                if ((strcmp(parsed_input[0], "exit") == 0) && char_arg_len==1) {
                        /* the addr of the user_input must be passed to free
                           the calloced user_input*/
                        fclose(fptr);
                        exit_cmd_handler(&user_input);
                }
                else if ((strcmp(parsed_input[0], "pwd") == 0) && char_arg_len==1)  {
                        pwd_cmd_handler();
                }
                else if ((strcmp(parsed_input[0], "cd") == 0) && char_arg_len<=2)  {
                        cd_cmd_handler(parsed_input[1]);
                }
                else if ((strcmp(parsed_input[0], "export") == 0) && char_arg_len==1) {
                        export_cmd_handler();
                }
                else if ((strcmp(parsed_input[0], "history") == 0) && char_arg_len==1)  {
                        history_cmd_handler();
                }
                else {
                        printf("Command not recognized\n");
                }
        }
        return 0;
}

/* parsing function that returns the number of cmd line args entered by user */
int parser(char *user_input, char *parsed_arr[], size_t ui_length) {
        /* int to hold umber of cmd line args */
        int char_arg_len=0;

        if (user_input[ui_length-1] == '\n') {
                user_input[ui_length-1] = '\0';
        }

        /* to prevent temp_input char from being destroyed after func exit */
        static char temp_input[MAX_CMD_INPUT_BUFFER];
        strcpy(temp_input, user_input);

        int ptr_pos = 0;
        /* strtok to delim the func with ' ' and save it in parsed_arr */
        parsed_arr[ptr_pos] = strtok(temp_input, " ");
        while (parsed_arr[ptr_pos] != NULL)
        {
                char_arg_len++;
                parsed_arr[++ptr_pos] = strtok(NULL, " ");
        }
        return char_arg_len;
}
