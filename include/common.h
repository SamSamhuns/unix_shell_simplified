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
void push( struct Node * head, char cmd[MAX_INPUT_KWRD_LEN]);
/* function to taverse through linked list*/
void traverse_linked_list_stream( struct Node *head, FILE *llptr);
/* function to load history.txt cmds */
void load_linked_list (struct Node *head);


#endif
