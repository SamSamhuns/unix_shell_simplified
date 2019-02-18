#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "common.h"
#include "builtin_cmd_handler.h"

/* function to add a node to end of the linked list */
void push(struct Node *head, char cmd[MAX_INPUT_KWRD_LEN]) {
        struct Node * cur = head;
        /* loop till the last node */
        while ( cur->next != NULL ) {
                cur = cur->next;
        }
        cur->next = malloc(sizeof(Node));
        strcpy(cur->next->content, cmd);
        cur->next->next = NULL;
}

/* function to add node to the end of the history linked list
   returns -1 if an ! is enocuntered first otherwise returns 0 */
int push_history( struct Node *head, char cmd[MAX_INPUT_KWRD_LEN]) {
        /* Check for the first appearing exclamation mark */
        for (size_t i = 0; i < MAX_INPUT_KWRD_LEN; i++) {
                if (cmd[i] == ' ') {
                        continue;
                }
                /* for the first char in cmd that is not a space */
                else {
                        /* for exclamantion do not write to history linked list */
                        if (cmd[i] == '!') {
                                return -1;
                        }
                        break;
                }
        }
        push(head, cmd);
        return 0;
}

/* function to taverse through linked list and output to FILE stream */
void write_linked_list_history(struct Node *head, FILE *llptr) {
        struct Node *cur = head;

        while ( cur->next != NULL ) {
                cur = cur->next;
                /* Write into the file ptr */
                fprintf( llptr, "%s", cur->content);
        }
}

/* function to taverse through  history linked list and print to stdout
   number _of_args is the total number of arguments entered as of running this func */
void print_linked_list_history(int number_of_args, struct Node *head) {
        struct Node *cur = head;
        int arg_order = 1; /* Order in which the arguments were entered in the past */
        int number_buffer = 1; /* number_buffer dictates the pretty format for printing */

        number_of_args /= 10;
        while ( number_of_args > 0) {
                number_of_args /= 10;
                number_buffer += 1;
        }

        while ( cur->next != NULL ) {
                cur = cur->next;
                /* Write into the file ptr */
                /* Setting the width with a var number_buffer to format output*/
                fprintf( stdout, "%*d %s", number_buffer, arg_order, cur->content);
                arg_order++;
        }
}

/* function to load history.txt cmds
   returns the number of args present in the history.txt file */
int load_linked_list_history(struct Node *head) {
        int number_of_args = 0;
        FILE *read_fptr;
        char temp_cmd_buffer[MAX_INPUT_KWRD_LEN];

        /* if the file does not exist*/
        if ((read_fptr = fopen("./history.txt", "r")) == NULL) {
                printf("Error Opening File history.txt");
                exit(1);
        }

        /* read from the opened read_fptr and save into the history linked list */
        while (fgets(temp_cmd_buffer, MAX_INPUT_KWRD_LEN, read_fptr)) {
                push_history(head, temp_cmd_buffer);
                number_of_args++;
        }
        fclose(read_fptr);
        return number_of_args;
}

/* func to free memory allocated inside the linked lists */
void free_linked_list(struct Node *head) {
        struct Node *cur = head;

        while (cur != NULL ) {
                struct Node *temp = cur;
                cur = cur->next;
                free (temp);
        }
        head = NULL;
}

/* function to print the contents of the export linked list head */
void print_linked_list_export(struct Node *head) {
        struct Node * cur = head;
        while (cur != NULL) {
                printf("%s\n", cur->content);
                cur = cur->next;
        }
}

/* pushes new env vars entered using export into export_head */
int push_export( struct Node *head, char env_name_value_comb[MAX_INPUT_KWRD_LEN],
                 char env_var_name[MAX_INPUT_KWRD_LEN], char env_var_value[MAX_INPUT_KWRD_LEN]) {
        struct Node *cur = head;

        /* loop through entire linked list */
        while (cur != NULL) {
                char temp_content_store[MAX_INPUT_KWRD_LEN];
                strcpy(temp_content_store, cur->content); /* copy contents of cur Node
                  to temp_content_store so that the original contents of the Node aren't altered */
                char *env_var_array[2]; /* var to hold export var name and val */
                /* Breaking down the var using = */
                env_var_array[0] = strtok(temp_content_store,"=");
                env_var_array[1] = strtok(NULL, "=");

                /* if the var names are equal in the saved export_linked_list and the user cmd */
                if (strcmp(env_var_array[0], env_var_name) == 0) {
                        /* if existing var name has NULL val. i.e. PATH= */
                        if (env_var_array[1] == NULL || env_var_value == NULL ) {
                                strcpy(cur->content, env_name_value_comb);
                                return 0;
                        }
                        /* if the both names are same, env_var already exists so do nothing*/
                        else if (strcmp(env_var_array[1], env_var_value) == 0) {
                                return -1;
                        }
                        /* if just the env_var names match but not the values */
                        else {
                                strcpy(cur->content, env_name_value_comb);
                                return 0;
                        }
                }
                cur = cur->next;
        }
        /* Only run this when new env_var are entered using export */
        push(head, env_name_value_comb);
        return 0;
}
