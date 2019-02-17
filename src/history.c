#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "common.h"
#include "builtin_cmd_handler.h"

/* function to handle cmd history*/
void history_cmd_handler( struct Node *head ) {
  /* func to print to stdout the contens of the linked list */
  traverse_linked_list_stream(head, stdout);
}
