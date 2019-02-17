#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "common.h"
#include "builtin_cmd_handler.h"

/* function to add node to the end of the linked list*/
void push( struct Node *head, char cmd[MAX_INPUT_KWRD_LEN]) {
        struct Node *cur = head;
        /* loop until the end of the linked list */
        while ( cur->next != NULL ) {
                cur = cur->next;
        }
        cur->next = malloc(sizeof(Node));
        strcpy(cur->next->content, cmd);
        cur->next->next = NULL;

}

/* function to taverse through linked list and output to stream*/
void write_linked_list(struct Node *head, FILE *llptr) {
        struct Node *cur = head;

        while ( cur->next != NULL ) {
                cur = cur->next;
                /* Write into the file ptr */
                fprintf( llptr, "%s", cur->content);
        }
}

/* function to taverse through linked list and output to stream*/
void print_linked_list( int number_of_args, struct Node *head) {
        struct Node *cur = head;
        int arg_order = 1;
        int number_buffer = 1;

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
int load_linked_list (struct Node *head) {
        int number_of_args = 0;
        FILE *read_fptr;
        char temp_cmd_buffer[MAX_INPUT_KWRD_LEN];

        /* if the file does not exist*/
        if ((read_fptr = fopen("./history.txt", "r")) == NULL) {
                printf("Error Opening File history.txt");
                exit(1);
        }

        while ( fgets(temp_cmd_buffer, MAX_INPUT_KWRD_LEN, read_fptr)) {
                push(head, temp_cmd_buffer);
                number_of_args++;
        }
        return number_of_args;
}

/* func to free memory allocated inside the linked lists */
void free_linked_list (struct Node *head) {
        struct Node *cur = head;

        while (cur != NULL ) {
                struct Node *temp = cur;
                cur = cur->next;
                free (temp);
        }
        head = NULL;
}
