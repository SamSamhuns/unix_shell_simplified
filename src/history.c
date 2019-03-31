#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "common.h"
#include "history.h"

/* function to handle cmd history*/
void history_cmd_handler(int number_of_args, struct Node *head ) {
	/* func to print to stdout the contens of the linked list */
	print_linked_list_history(number_of_args, head);
}

/* run cmd from history selected by !
   returns name of command to run again */
char *exclaim_cmd_handler(struct Node *head, int arg_order_exclamation) {
	int arg_iterator = 0;
	struct Node *cur = head;
	while ( cur != NULL ) {
		if (arg_iterator == arg_order_exclamation) {
			return (cur->content);
		}
		cur = cur->next;
		arg_iterator++;
	}
	/* Control should not reach here because of the "(arg_order_exclamation > number_of_args)"
	   check inside the run_command(...) func in main.c */
	return NULL;
}
