#ifndef COMMON_H // Making sure the header files are not included twice
#define COMMON_H

#define MAX_CMD_INPUT_BUFFER 600
#define MAX_INPUT_ARR_LEN 60 /* max number of cmd args entered each time*/
#define MAX_INPUT_KWRD_LEN 350 /* max len of each cmd line arg */
#define READ_PIPE 0 /* Read end of the IPC pipe */
#define WRITE_PIPE 1 /* Write end of the IPC pipe */
#define DEBUG 0

typedef struct Node {
        char content[MAX_INPUT_KWRD_LEN];
        struct Node *next;
} Node;

/* func to add node to end of linked list */
int push_history( struct Node *head, char cmd[MAX_INPUT_KWRD_LEN]);

/* func to write contents of linked list starting with head
   to the stream FILE *llptr*/
void write_linked_list_history(struct Node *head, FILE *llptr);

/* func to print the contents of the linked list with special formatting */
void print_linked_list_history( int number_of_args, struct Node *head);

/* function to load history.txt cmds
   returns the number of args present in the history.txt file */
int load_linked_list_history(struct Node *head);

/* func to free memory allocated inside the linked lists */
void free_linked_list(struct Node *head);

/* function to add a node to end of the linked list */
void push(struct Node *head, char cmd[MAX_INPUT_KWRD_LEN]);

/* function to print the contents of the export linked list head */
void print_linked_list_export(struct Node *head);

/* pushes new env vars entered using export into export_head */
int push_export( struct Node *head, char env_name_value_comb[MAX_INPUT_KWRD_LEN], char env_var_name[MAX_INPUT_KWRD_LEN],
                 char env_var_value[MAX_INPUT_KWRD_LEN]) ;

#endif
