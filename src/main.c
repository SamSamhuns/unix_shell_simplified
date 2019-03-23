/*
   UNIX Shell prototype
   Author Samridha Shrestha
   Feb 2019
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include "cd.h"
#include "pwd.h"
#include "exit.h"
#include "export.h"
#include "common.h"
#include "history.h"

/* main parser func that returns the number of cmd args entered by the user */
int parser(char *user_input, char *parsed_input[], size_t ui_length, struct Node *export_head);
/* main cmd executing func that runs the right built-in or extern cmd */
void run_command (int char_arg_len, int arg_order_exclamation, int number_of_args, struct Node *history_head,
                  struct Node *export_head, FILE *fptr, char *user_input, char *parsed_input[] );
/* searches for parsed_arr[0] file in the path dir_path */
int search_dir (char *dir_path, char *parsed_arr[], int char_arg_len, struct Node * export_head);
/* func to search for cmd in the ENV vars saved by the export func */
int search_in_export_path( struct Node *export_head, char *parsed_arr[], int char_arg_len );
/* returns replaced instances of cmd args where $VAR_NAME replaced with the env var value */
char *parse_env_var_call(char *cmd_argument, int cmd_len, struct Node * export_head);

int fork_and_execve (char *dir_path, char *parsed_arr[], struct Node * export_head);

/* function to handle the main loop of the cmd prompt */
int main(void){
        int char_arg_len = 0; /* length of number of args and cmd entered each time */
        int number_of_args = 0; /* total number of args entered including history.txt contents */
        int arg_order_exclamation = 0; /* arg order for exclamantion mark exec from history */
        // int export_head_env_var_count = 0; /* var to hold count of env variables in shell */
        FILE *fptr = fopen("./history.txt", "a+"); /* open history to read and append cmds to*/
        char *parsed_input[MAX_INPUT_ARR_LEN]; /* array of char ptrs to hold parsed input*/
        char *user_input = calloc(MAX_CMD_INPUT_BUFFER, sizeof(char));
        char initial_pwd_value[MAX_CMD_INPUT_BUFFER+10] = "PWD="; /* string to hold the starting working dir*/

        /* A history_head and a export_head node is initialized
           whoich is equal to NULL */
        struct Node *history_head = NULL;
        struct Node *export_head = NULL;
        history_head = malloc(sizeof(Node));
        history_head->next = NULL;

        export_head = malloc(sizeof(Node));
        export_head->next = malloc(sizeof(Node));
        export_head->next->next = malloc(sizeof(Node));

        if (history_head == NULL || export_head == NULL || export_head->next == NULL ||
            export_head->next->next == NULL ) {
                printf("Out of memory\n");
                return 1;
        }

        strcpy(export_head->content, "PATH=");
        getcwd(user_input, MAX_CMD_INPUT_BUFFER);
        strcpy(export_head->next->content, strcat(initial_pwd_value, user_input));
        strcpy(export_head->next->next->content, "TERM=xterm-256color");
        export_head->next->next->next = NULL;

        /* load history.txt cmds in linked list buffer
           load func returns number of args that have been loaded */
        number_of_args = load_linked_list_history(history_head);

        // main while loop for shell
        while (1) {
                // Checking if memory is available in the heap
                if (user_input == NULL) {
                        printf("Out of memory\n" );
                        return 1;
                }
                printf(">> ");

                /* Get user input and auto append \n at end */
                if (fgets(user_input, MAX_CMD_INPUT_BUFFER, stdin) == NULL) {
                        printf("Error getting user input\n");
                }

                /* if the user just enters a new line char */
                if (!(isalnum(user_input[0])) && strlen(user_input) == 1)  {
                        continue;
                }

                /* Save each entered cmd to a history.txt file */
                if (push_history(history_head, user_input) == 0) {
                        number_of_args += 1;
                }

                /* Parse the user_input into a array of char ptrs holding user cmd args */
                char_arg_len = parser(&user_input[0], parsed_input, strlen(user_input), export_head);

                // Debug info
                if (DEBUG==1) {
                        fprintf(stdout, "1) USER INPUT = %s \n",
                                user_input);
                        for (int i = 0; i<char_arg_len; i++) {
                                printf("\t%d) %s, len = %lu\n",
                                       i+1, parsed_input[i], strlen(parsed_input[i]));
                        }
                }
                /* MAIN CMD RUNS HERE */
                run_command(char_arg_len, arg_order_exclamation, number_of_args, history_head,
                            export_head, fptr, user_input, parsed_input);
        }
        /* Control should not reach here */
        return 0;
}

/* main cmd executing func that runs the right built-in or extern cmd */
void run_command (int char_arg_len, int arg_order_exclamation, int number_of_args, struct Node *history_head,
                  struct Node *export_head, FILE *fptr, char *user_input, char *parsed_input[] ) {
        if ((strcmp(parsed_input[0], "exit") == 0) && char_arg_len==1) {
                /* the addr of the user_input must be passed to free
                   the calloced user_input*/
                exit_cmd_handler(&user_input, history_head, export_head, fptr);
        }
        else if ((strcmp(parsed_input[0], "pwd") == 0) && char_arg_len == 1)  {
                pwd_cmd_handler();
        }
        else if ((strcmp(parsed_input[0], "cd") == 0) && char_arg_len <= 2)  {
                cd_cmd_handler(parsed_input[1], export_head);
        }
        else if ((strcmp(parsed_input[0], "history") == 0) && char_arg_len == 1)  {
                /* Write into the history file before exiting
                   history_cmd_handler(history_head, stdout); */
                history_cmd_handler(number_of_args, history_head);
        }
        else if ((parsed_input[0][0] == '!') && char_arg_len==1 &&
                 (strlen(parsed_input[0]) > 1) ) {
                arg_order_exclamation = 0; /* reset to zero */
                /* loops through the chars of the cmd entered starting from the
                   first pos and creates the arg order for exec in the history linked list */
                for (size_t i = 1; i < strlen(parsed_input[0]); i++) {
                        if ( !(isdigit(parsed_input[0][i])) ) {
                                printf("Command not recognized\n");
                                break;
                        }
                        arg_order_exclamation = (arg_order_exclamation * 10) +
                                                (parsed_input[0][i] - '0');
                }
                if ((arg_order_exclamation > number_of_args) || arg_order_exclamation == 0) {
                        printf("-shell: !%i: event not found\n", arg_order_exclamation);
                        return; /* Do not proceed if event was not found */
                }
                /* Recursive call here (Only happens once) */
                static char *exclaim_parsed_input[MAX_INPUT_ARR_LEN];
                strcpy(user_input, exclaim_cmd_handler(history_head, arg_order_exclamation));
                /* returns the total number of args entered for the choosen cmd from history */
                char_arg_len = parser(user_input, exclaim_parsed_input, strlen(user_input), export_head);

                run_command(char_arg_len, arg_order_exclamation, number_of_args,
                            history_head, export_head, fptr, user_input, exclaim_parsed_input);
        }
        /* if export is called just to display env vars */
        else if ((strcmp(parsed_input[0], "export") == 0) && char_arg_len == 1) {
                print_env_export_handler(export_head);
        }
        /* if export is called to save an env var */
        else if ((strcmp(parsed_input[0], "export") == 0) && char_arg_len > 1) {
                /* save func returns 0 for success and -1 for failure */
                if (save_env_export_handler(parsed_input,
                                            char_arg_len, export_head) == -1) {
                        printf("Export var is illegal\n");
                }
        }
        else {
                /* if no matching cmds found check if the cmd is external
                   -1 return for an unrecognized program */
                if (search_in_export_path(export_head, parsed_input, char_arg_len) == -1) {
                        printf("Command not recognized\n");
                }
        }
}

/* func to search for cmd in the ENV vars saved by the export func
   returns 0 for successful find and -1 for failure */
int search_in_export_path( struct Node *export_head, char *parsed_arr[], int char_arg_len ) {
        struct Node *cur = export_head;
        while (cur != NULL) {
                static char *export_var_array[2]; /* To hold env var name and value */
                static char *export_var_array_paths[MAX_INPUT_KWRD_LEN]; /* to hold
                                                                            different paths in the env var values */
                int num_of_paths = 0; /* number of paths in the env_var */
                int path_iter = 0; /* for iteration splitting using strtok */
                static char temp_store[MAX_INPUT_KWRD_LEN]; /* temp store to make contents
                                                               of the export linked list are not modified */

                strcpy(temp_store, cur->content);
                export_var_array[0] = strtok(temp_store, "=");
                export_var_array[1] = strtok(NULL, "=");
                if (export_var_array[1] == NULL) {
                        cur = cur->next;
                        continue;
                }

                /* split using the colon as the delimiter and store it in
                   export_var_array_paths */
                export_var_array_paths[path_iter] = strtok(export_var_array[1], ":");
                while (export_var_array_paths[path_iter] != NULL) {
                        num_of_paths++;
                        export_var_array_paths[++path_iter] = strtok(NULL, ":");
                }

                for (size_t i = 0; i < num_of_paths; i++) {
                        /* temp_path_holder is required as doing strcat to export_var_array_paths elems
                           destroys the later elements of the export_var_array_paths char ptr array*/
                        char temp_path_holder[MAX_CMD_INPUT_BUFFER];

                        strcpy(temp_path_holder, export_var_array_paths[i]);
                        /* Add a front slash if it is not present in the path name */
                        if ( temp_path_holder[strlen(export_var_array_paths[i])-1] != '/' ) {
                                strcat(temp_path_holder, "/");
                        }
                        int result;
                        result  = search_dir(temp_path_holder, parsed_arr, char_arg_len, export_head);

                        if (result== 0) {
                                return 0;
                        }
                }
                cur = cur->next;
        }
        return -1;
}

/* searches for parsed_arr[0] file in the path dir_path
   returns 0 for sucessful find and -1 for failure */
int search_dir (char *dir_path, char *parsed_arr[], int char_arg_len,  struct Node *export_head) {
        struct dirent *dir_entry; // define struct for a ptr for entering directory
        DIR *dir_read = opendir(dir_path); // returns a ptr of type DIR
        // pid_t pid, wait_pid; // to hold process pids
        // int status = 0;

        /* if the directory specified by the path couldn't be opened */
        if (dir_read == NULL) {
                // printf("Error: could not open path directory\n");
                return -1;
        }

        /* Adding a NULL ptr to the end of parsed_arr for argv being sent to excev */
        parsed_arr[char_arg_len] = NULL;

        if ( (parsed_arr[0][0] == '.' && parsed_arr[0][1] == '/') || parsed_arr[0][0] == '/') {
                if (fork_and_execve(parsed_arr[0], parsed_arr, export_head) == 0) {
                        closedir(dir_read);
                        return 0;
                }
                closedir(dir_read);
                return -1;
        }

        while ((dir_entry = readdir(dir_read)) != NULL) {
                if ((strcmp(dir_entry->d_name, parsed_arr[0])) == 0) {

                        /* EXECV OPERATION */
                        // parsed_arr[0] is the external cmd that has been entered i.e. ls
                        strcat(dir_path, parsed_arr[0]);

                        if (fork_and_execve(dir_path, parsed_arr, export_head) == 0) {
                                closedir(dir_read);
                                return 0;
                        }
                }
        }
        closedir(dir_read);
        return -1;
}


/* parsing function that returns the number of cmd line args entered by user */
int parser(char *user_input, char *parsed_arr[], size_t ui_length, struct Node *export_head) {
        /* int to hold umber of cmd line args */
        int char_arg_len=0;
        /* temp cmd argument holder to hold the return values from strtok() */
        char *temp_cmd_arg_holder[MAX_CMD_INPUT_BUFFER];

        /* removing newline char from end that fgets adds */
        if (user_input[ui_length-1] == '\n') {
                user_input[ui_length-1] = '\0';
        }

        /* to prevent temp_input char from being destroyed after func exit */
        static char temp_input[MAX_CMD_INPUT_BUFFER];
        strcpy(temp_input, user_input);

        int ptr_pos = 0;

        /* strtok to delim the func with ' ' and save it in parsed_arr */
        temp_cmd_arg_holder[ptr_pos] = strtok(temp_input, " ");
        parsed_arr[ptr_pos] = parse_env_var_call(temp_cmd_arg_holder[ptr_pos],
                                                 strlen(temp_cmd_arg_holder[ptr_pos]),
                                                 export_head);
        while (parsed_arr[ptr_pos] != NULL)
        {
                char_arg_len++;
                temp_cmd_arg_holder[++ptr_pos] = strtok(NULL, " ");
                // parsed_arr[++ptr_pos] = strtok(NULL, " ");
                if (temp_cmd_arg_holder[ptr_pos] == NULL) {
                        // printf("NULL encountered insdie loop\n");
                        break;
                }
                parsed_arr[ptr_pos] = parse_env_var_call(temp_cmd_arg_holder[ptr_pos],
                                                         strlen(temp_cmd_arg_holder[ptr_pos]),
                                                         export_head);
                if (DEBUG == 1) {
                        printf("\t%i)INSIDE MAIN PARSER LOOP %s\n",char_arg_len, parsed_arr[ptr_pos]);
                }

        }
        return char_arg_len; /* return number of space separated cmd parts entered */
}

/* func to parse and return valur of env vars entered as cmd arguments
   in the form $ENV_VAR_NAME i.e. cd $JAVA */
char *parse_env_var_call(char *cmd_argument, int cmd_len, struct Node * export_head) {
        /* stores name of var after the $ sign. i.e. PATH for $PATH*/
        char parsed_argument[cmd_len];
        /* stores content of export_linked_list env_var name and val combinations
           i.e. PATH=/usr/bin:/usr */
        char temp_export_linked_list_store[MAX_CMD_INPUT_BUFFER];

        /* immediately return the same NULL str is a NULL string is passed*/
        if (cmd_argument == NULL) {
                return cmd_argument;
        }

        /* if the cmd_arg does start with a $ */
        if (cmd_argument[0] == '$') {
                /* save contents of cmd_argument[1:] to parsed_argument */
                for (size_t i = 1; i < cmd_len; i++) {
                        parsed_argument[i-1] = cmd_argument[i];
                }
                parsed_argument[cmd_len-1] = '\0';

                /* loop through export linked list to see of the env_var exists */
                struct Node * cur = export_head;
                while (cur != NULL) {
                        char *var_name_val[2]; /* char arg array to hold var name and val */
                        /* So as to not modify the actual contents of the export ln lst*/
                        strcpy(temp_export_linked_list_store, cur->content);
                        var_name_val[0] = strtok(temp_export_linked_list_store, "="); // holds var name i.e. PATH
                        var_name_val[1] = strtok(NULL, "="); //hold var_vale i.e. /usr/bin for PATH var

                        /* if the user entered env var and the export linked list saved env var match
                           return the val of the saved env_var from the export linked list */
                        if (strcmp(parsed_argument, var_name_val[0]) == 0) {
                                return var_name_val[1];
                        }
                        cur = cur->next;
                }
        }
        return &cmd_argument[0]; /* return unchanged string if not a env var call */
}

/* function forks the current process to create a child process and run execv in the child
   returns 0 for successful run, -1 for negative run or error */
int fork_and_execve (char *dir_path, char *parsed_arr[], struct Node *export_head) {
        pid_t pid, wait_pid; // to hold process pids
        int status = 0;
        int return_status = 0; // return status is communicated to the parent, 0 by default
        int fd_pipe[2]; // int array to hold the file decrp ends for pipe
        char *env_var_holder[MAX_ENV_VAR_NUMBER]; // char ptr array to hold the env vars

        int env_var_iterator = 0;
        struct Node *cur = export_head;
        while (cur != NULL) {
                // strcpy(env_var_holder[env_var_iterator], cur->content);
                env_var_holder[env_var_iterator] = cur->content;
                env_var_iterator++;
                cur = cur->next;
        }
        env_var_holder[env_var_iterator] = NULL;

        /* creating pipe for IPC */
        if (pipe(fd_pipe) == -1) {
                fprintf(stderr, "Pipe creation error");
                return -1;
        }

        pid = fork();
        if (pid < 0) {
                printf("Fork error\n");
                return -1;
        }
        /* inside the CHILD, load a new c file using execv call */
        else if (pid == 0) {
                close(fd_pipe[READ_PIPE]); // closing the read end of pipe in the parent

                if (execve(dir_path, parsed_arr, env_var_holder) == -1 ) {
                        return_status = -1; /* As child cannot return before exiting */
                        /* Error in execv */
                        // fprintf(stderr, "execv couldn't load %s\n", dir_path);
                }
                /* writes in the pipe to the parent process to send the return status */
                write(fd_pipe[WRITE_PIPE], &return_status, sizeof(return_status));
                close(fd_pipe[WRITE_PIPE]);
                /* IMPORTANT */
                /* Must run to stop extraneous child process if execv failed */
                exit(-1);
        }
        /* inside the PARENT */
        else {
                close(fd_pipe[WRITE_PIPE]); // closing write end of the pipe in the parent
                read(fd_pipe[READ_PIPE], &return_status, sizeof(return_status)); // reading the return status from the child
                close(fd_pipe[READ_PIPE]); // closing read end

                /* Parent waits till all child processes have been reaped
                   wait_pid will be < 0 once all child processes have terminated */
                while ((wait_pid = wait(&status)) > 0);
                if (DEBUG == 1) {
                        printf("Return status from PARENT is %i\n",return_status);
                }

                return return_status;
        }
        return 0; /* Control should not reach here */
}
