#ifndef COMMON_H // Making sure the header files are not included twice
#define COMMON_H

#define MAX_CMD_INPUT_BUFFER 500
#define MAX_INPUT_ARR_LEN 50 /* max number of cmd args entered each time*/
#define MAX_INPUT_KWRD_LEN 200 /* max len of each cmd line arg */
#define DEBUG 0

typedef struct Node {
        char content[MAX_INPUT_KWRD_LEN];
        struct Node *next;
} Node;

/* func to add node to end of linked list */
int push( struct Node *head, char cmd[MAX_INPUT_KWRD_LEN]);

/* func to write contents of linked list starting with head
   to the stream FILE *llptr*/
void write_linked_list(struct Node *head, FILE *llptr);

/* func to print the contents of the linked list with special formatting */
void print_linked_list( int number_of_args, struct Node *head);

/* function to load history.txt cmds
   returns the number of args present in the history.txt file */
int load_linked_list (struct Node *head);

/* func to free memory allocated inside the linked lists */
void free_linked_list (struct Node *head);

#endif
