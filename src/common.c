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
void traverse_linked_list_stream(struct Node *head, FILE *llptr) {
        struct Node *cur = head;
        while ( cur->next != NULL ) {
                cur = cur->next;
                /* Write into the file ptr */
                fprintf( llptr, "%s", cur->content);
        }
}

/* function to load history.txt cmds */
void load_linked_list (struct Node *head) {
        FILE *read_fptr;
        char temp_cmd_buffer[MAX_INPUT_KWRD_LEN];

        /* if the file does not exist*/
        if ((read_fptr = fopen("./history.txt", "r")) == NULL) {
          printf("Error Opening File history.txt");
          exit(1);
        }

        while ( fgets(temp_cmd_buffer, MAX_INPUT_KWRD_LEN, read_fptr)) {
                push(head, temp_cmd_buffer);
        }
}
