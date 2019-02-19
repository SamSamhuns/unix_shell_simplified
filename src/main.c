/*
   UNIX Shell prototype
   Author Samridha Shrestha
   Feb 2019
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include "common.h"
#include "builtin_cmd_handler.h"

/* main parser func that returns the number of cmd args entered by the user */
int parser(char *user_input, char *parsed_input[], size_t ui_length);
/* main cmd executing func that runs the right built-in or extern cmd */
void run_command (int char_arg_len, int arg_order_exclamation, int number_of_args, struct Node *history_head,
                  struct Node *export_head, FILE *fptr, char *user_input, char *parsed_input[] );
/* searches for parsed_arr[0] file in the path dir_path */
int search_dir (char *dir_path, char *parsed_arr[], int char_arg_len);
/* func to search for cmd in the ENV vars saved by the export func */
int search_in_export_path( struct Node *head, char *parsed_arr[], int char_arg_len );

/* function to handle the main loop of the cmd prompt */
int main(void){
        int char_arg_len = 0; /* length of number of args and cmd entered each time */
        int number_of_args = 0; /* total number of args entered including history.txt contents */
        int arg_order_exclamation = 0; /* arg order for exclamantion mark exec from history */
        FILE *fptr = fopen("./history.txt", "a+"); /* open history to read and append cmds to*/
        char *parsed_input[MAX_INPUT_ARR_LEN]; /* array of char ptrs to hold parsed input*/
        char *user_input = calloc(MAX_CMD_INPUT_BUFFER, sizeof(char));

        /* A history_head and a export_head node is initialized
           whoich is equal to NULL */
        struct Node *history_head = NULL;
        struct Node *export_head = NULL;
        history_head = malloc(sizeof(Node));
        export_head = malloc(sizeof(Node));
        if (history_head == NULL || export_head == NULL) {
                printf("Out of memory\n");
                return 1;
        }
        history_head->next = NULL;
        export_head->next = NULL;
        strcpy(export_head->content, "PATH=");

        /* load history.txt cmds in linked list buffer
           load func returns number of args that have been loaded */
        number_of_args = load_linked_list_history(history_head);

        // main while loop for shell
        while (1) {
                // Checking if memory is available in the heap
                if (user_input == NULL) {
                        printf("Out of memory\n" );
                        return 1;
                }
                printf(">> ");

                /* Get user input and auto append \n at end */
                if (fgets(user_input, MAX_CMD_INPUT_BUFFER, stdin) == NULL) {
                        printf("Error getting user input\n");
                }

                /* if the user just enters a new line char */
                if (!(isalnum(user_input[0])) && strlen(user_input) == 1)  {
                        continue;
                }

                /* Save each entered cmd to a history.txt file */
                if (push_history(history_head, user_input) == 0) {
                        number_of_args += 1;
                }

                /* Parse the user_input into a array of char ptrs holding user cmd args */
                char_arg_len = parser(&user_input[0], parsed_input, strlen(user_input));

                // Debug info
                if (DEBUG==1) {
                        fprintf(stdout, "1) USER INPUT = %s \n",
                                user_input);
                        for (int i = 0; i<char_arg_len; i++) {
                                printf("\t%d) %s, len = %lu\n",
                                       i+1, parsed_input[i], strlen(parsed_input[i]));
                        }
                }
                /* MAIN CMD RUNS HERE */
                run_command(char_arg_len, arg_order_exclamation, number_of_args, history_head,
                            export_head, fptr, user_input, parsed_input);
        }
        /* Control should not reach here */
        return 0;
}

/* main cmd executing func that runs the right built-in or extern cmd */
void run_command (int char_arg_len, int arg_order_exclamation, int number_of_args, struct Node *history_head,
                  struct Node *export_head, FILE *fptr, char *user_input, char *parsed_input[] ) {
        if ((strcmp(parsed_input[0], "exit") == 0) && char_arg_len==1) {
                /* the addr of the user_input must be passed to free
                   the calloced user_input*/
                exit_cmd_handler(&user_input, history_head, export_head, fptr);
        }
        else if ((strcmp(parsed_input[0], "pwd") == 0) && char_arg_len == 1)  {
                pwd_cmd_handler();
        }
        else if ((strcmp(parsed_input[0], "cd") == 0) && char_arg_len <= 2)  {
                cd_cmd_handler(parsed_input[1]);
        }
        else if ((strcmp(parsed_input[0], "history") == 0) && char_arg_len == 1)  {
                /* Write into the history file before exiting
                   history_cmd_handler(history_head, stdout); */
                history_cmd_handler(number_of_args, history_head);
        }
        else if ((parsed_input[0][0] == '!') && char_arg_len==1 &&
                 (strlen(parsed_input[0]) > 1) ) {
                arg_order_exclamation = 0; /* reset to zero */
                /* loops through the chars of the cmd entered starting from the
                   first pos and creates the arg order for exec in the history linked list */
                for (size_t i = 1; i < strlen(parsed_input[0]); i++) {
                        if ( !(isdigit(parsed_input[0][i])) ) {
                                printf("Command not recognized\n");
                                break;
                        }
                        arg_order_exclamation = (arg_order_exclamation * 10) +
                                                (parsed_input[0][i] - '0');
                }
                if ((arg_order_exclamation > number_of_args) || arg_order_exclamation == 0) {
                        printf("-shell: !%i: event not found\n", arg_order_exclamation);
                        return; /* Do not proceed if event was not found */
                }
                /* Recursive call here (Only happens once) */
                static char *exclaim_parsed_input[MAX_INPUT_ARR_LEN];
                strcpy(user_input, exclaim_cmd_handler(history_head, arg_order_exclamation));
                /* returns the total number of args entered for the choosen cmd from history */
                char_arg_len = parser(user_input, exclaim_parsed_input, strlen(user_input));

                run_command(char_arg_len, arg_order_exclamation, number_of_args,
                            history_head, export_head, fptr, user_input, exclaim_parsed_input);
        }
        /* if export is called just to display env vars */
        else if ((strcmp(parsed_input[0], "export") == 0) && char_arg_len == 1) {
                print_env_export_handler(export_head);
        }
        /* if export is called to save an env var */
        else if ((strcmp(parsed_input[0], "export") == 0) && char_arg_len > 1) {
                /* save func returns 0 for success and -1 for failure */
                if (save_env_export_handler(parsed_input,
                                            char_arg_len, export_head) == -1) {
                        printf("Export var is illegal\n");
                }
        }
        else {
                /* if no matching cmds found check if the cmd is external
                  -1 return for an unrecognized program */
                if (search_in_export_path(export_head, parsed_input, char_arg_len) == -1) {
                        printf("Command not recognized\n");
                }
        }
}

/* func to search for cmd in the ENV vars saved by the export func
  returns 0 for successful find and -1 for failure */
int search_in_export_path( struct Node *head, char *parsed_arr[], int char_arg_len ) {
        struct Node *cur = head;
        while (cur != NULL) {
                static char *export_var_array[2]; /* To hold env var name and value */
                static char *export_var_array_paths[MAX_INPUT_KWRD_LEN]; /* to hold
                  different paths in the env var values */
                int num_of_paths = 0; /* number of paths in the env_var */
                int path_iter = 0; /* for iteration splitting using strtok */
                static char temp_store[MAX_INPUT_KWRD_LEN]; /* temp store to make contents
                  of the export linked list are not modified */

                strcpy(temp_store, cur->content);
                export_var_array[0] = strtok(temp_store, "=");
                export_var_array[1] = strtok(NULL, "=");
                if (export_var_array[1] == NULL) {
                        cur = cur->next;
                        continue;
                }

                /* split using the colon as the delimiter and store it in
                  export_var_array_paths */
                export_var_array_paths[path_iter] = strtok(export_var_array[1], ":");
                while (export_var_array_paths[path_iter] != NULL) {
                        num_of_paths++;
                        export_var_array_paths[++path_iter] = strtok(NULL, ":");
                }

                for (size_t i = 0; i < num_of_paths; i++) {
                        /* Add a front slash if it is not present in the path name */
                        if ( export_var_array_paths[i][strlen(export_var_array_paths[i])-1] != '/' ) {
                                strcat(export_var_array_paths[i], "/");
                        }
                        if (search_dir(export_var_array_paths[i], parsed_arr, char_arg_len) == 0) {
                                return 0;
                        }
                }
                cur = cur->next;
        }
        return -1;
}

/* searches for parsed_arr[0] file in the path dir_path
  returns 0 for sucessful find and -1 for failure */
int search_dir (char *dir_path, char *parsed_arr[], int char_arg_len) {
        struct dirent *dir_entry; // define struct for a ptr for entering directory
        DIR *dir_read = opendir(dir_path); // returns a ptr of type DIR

        /* if the directory specified by the path couldn't be opened */
        if (dir_read == NULL) {
                // printf("Error: could not open path directory\n");
                return -1;
        }

        while ((dir_entry = readdir(dir_read)) != NULL) {
                if ((strcmp(dir_entry->d_name, parsed_arr[0])) == 0) {
                        /* PRINTING EXTERN CMDS HERE */
                        // strcat(dir_path, "/");
                        strcat(dir_path, parsed_arr[0]);
                        printf("%s is an external command (%s)\n", parsed_arr[0], dir_path);
                        if (char_arg_len > 1) {
                                printf("command arguments:\n");
                                for (size_t i = 1; i < char_arg_len; i++) {
                                        printf("%s\n", parsed_arr[i]);
                                }
                        }
                        return 0;
                }
        }
        closedir(dir_read);
        return -1;
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

        return char_arg_len; /* return number of space separated cmd parts entered */
}
