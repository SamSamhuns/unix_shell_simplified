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

/* Parser function */
int parser(char *user_input, char *parsed_input[], size_t ui_length);
char *exclaim_cmd_handler(struct Node *head, int arg_order_exclamation);
void run_command (int char_arg_len, int arg_order_exclamation, int number_of_args, struct Node *history_head,
                  struct Node *export_head, FILE *fptr, char *user_input, char *parsed_input[] );


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
        // printf("%s %lu\n", export_head->content, strlen(export_head->content));

        /* load history.txt cmds in linked list buffer */
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
                /* MAIN CHECK GOES HERE */
                run_command(char_arg_len, arg_order_exclamation, number_of_args, history_head,
                            export_head, fptr, user_input, parsed_input);
        }
        /* Control should not reach here */
        return 0;
}

void run_command (int char_arg_len, int arg_order_exclamation, int number_of_args, struct Node *history_head,
                  struct Node *export_head, FILE *fptr, char *user_input, char *parsed_input[] ) {
        if ((strcmp(parsed_input[0], "exit") == 0) && char_arg_len==1) {
                /* Save the linked list into the history.txt file */
                write_linked_list_history(history_head, fptr);
                fclose(fptr);
                /* the addr of the user_input must be passed to free
                   the calloced user_input*/
                exit_cmd_handler(&user_input, history_head);
        }
        else if ((strcmp(parsed_input[0], "pwd") == 0) && char_arg_len == 1)  {
                pwd_cmd_handler();
        }
        else if ((strcmp(parsed_input[0], "cd") == 0) && char_arg_len <= 2)  {
                cd_cmd_handler(parsed_input[1]);
        }
        else if ((strcmp(parsed_input[0], "export") == 0) && char_arg_len == 1) {
                export_cmd_handler();
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
                        return;
                }
                /* Recursive call here (Only happens once) */
                char *exclaim_parsed_input[MAX_INPUT_ARR_LEN];
                strcpy(user_input, exclaim_cmd_handler(history_head, arg_order_exclamation));
                char_arg_len = parser(user_input, exclaim_parsed_input, strlen(user_input));

                run_command(char_arg_len, arg_order_exclamation, number_of_args,
                            history_head, export_head, fptr, user_input, exclaim_parsed_input);
        }
        /* if export is called just to display env vars */
        else if ((strcmp(parsed_input[0], "export")) && char_arg_len == 1) {
                return;
        }
        else if ((strcmp(parsed_input[0], "export")) && char_arg_len > 1) {
                return;
        }
        else {
                printf("Command not recognized\n");
        }

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

/* run cmd from history selected by ! */
char *exclaim_cmd_handler(struct Node *head, int arg_order_exclamation) {
        int arg_iterator = 0;
        struct Node *cur = head;
        while ( cur != NULL ) {
                if (arg_iterator == arg_order_exclamation) {
                        return (cur->content);
                        break;
                }
                cur = cur->next;
                arg_iterator++;
        }
        /* Control should not reach here because of the "(arg_order_exclamation > number_of_args)"
           check inside the run_command(...) func */
        return NULL;
}
