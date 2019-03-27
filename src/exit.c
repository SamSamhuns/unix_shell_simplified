#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "common.h"
#include "exit.h"

// function to handle exit statements
void exit_cmd_handler(char **user_input_ptr,
                      struct Node *history_head, struct Node *export_head, FILE *fptr){
	/* Save the linked list into the history.txt file */
	write_linked_list_history(history_head, fptr);

	// free the allocated memory that is passed as an arg to this func
	free(*user_input_ptr);
	/* freeing memory allocated inside the linked lists */
	free_linked_list(history_head);
	free_linked_list(export_head);
	fclose(fptr);     /* free the file pointer */

	exit(0);
}
