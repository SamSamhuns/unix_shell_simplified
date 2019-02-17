#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "common.h"
#include "builtin_cmd_handler.h"

// function to handle exit statements
void exit_cmd_handler(char **user_input_ptr, struct Node *head){
        // free the allocated memory that is passed as an arg to this func
        free(*user_input_ptr);
        /* freeing memory allocated inside the linked lists */
        free_linked_list(head);
        exit(0);
}
