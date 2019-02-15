/*
   UNIX Shell prototype
   Author Samridha Shrestha
   Feb 2019
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/builtin_cmd_handler.h"

void parser(char ** user_input, char **parsed_input);

// function to handle the main loop of the cmd prompt
int main(void){

        char *user_input = calloc(MAX_CMD_INPUT_BUFFER, sizeof(char));
        char *parsed_input = calloc(MAX_CMD_INPUT_BUFFER, sizeof(char));

        // main while loop for function
        while (1) {
                // Checking if memory is available in the heap
                if (user_input == NULL) {
                        printf("Out of memory\n" );
                        return 1;
                }
                printf(">> ");
                fgets(user_input, MAX_CMD_INPUT_BUFFER, stdin);

                // char *token = strtok(user_input, "");
                //
                // while (token != NULL)
                // {
                //         printf("%s\n", token);
                //         token = strtok(NULL, "-");
                // }
                parser( &user_input, &parsed_input);
                printf("INPUTS %s %s\n", user_input, parsed_input );

                if (strcmp(parsed_input, "exit") == 0) {
                        exit_cmd_handler(&user_input, &parsed_input);
                        exit(0);
                }
                else if (strcmp(user_input, "pwd") == 0) {
                        char *current_dir;
                        pwd_cmd_handler(&current_dir);
                        printf("%s\n", current_dir);
                        free(current_dir);
                }
                else if (strcmp(user_input, "cd") == 0) {
                        char dir_path[MAX_CMD_INPUT_BUFFER];
                        strcpy(dir_path, user_input);
                        cd_cmd_handler(dir_path);
                }
                else if (strcmp(user_input, "export") == 0) {
                        export_cmd_handler();
                }
                else if (strcmp(user_input, "history") == 0) {
                        history_cmd_handler();
                }
                else {
                        printf("Command not recognized\n");
                }
        }
        return 0;
}

void parser(char ** user_input, char **parsed_input) {
        int user_input_len;
        user_input_len = strlen(*user_input);
        strcpy(*parsed_input, *user_input);

}
