#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "include/common.h"

#define READ 0
#define WRITE 1

/* Checks if piping or redirection has been entered in the input cmd
   And if pipes were present then return value if greater than 1 and reprs the
   total number of pipes found
   if return arr is check_return_arr = check_pipe_rtn_loc(parsed_arr, char_arg_len)
   check_return_arr[0] is the total number of pipes found*/
int check_pipe_rtn_loc(char *parsed_arr[], int char_arg_len, int pipes_loc_arr[]);
/* Check for errors after pipes have been seen
   returns 0 on no error and -1 on ERRORS found */
int error_check_pipe(char *parsed_arr[], int char_arg_len, int cur_index);
/* Function to check for errors in parsed_arr
   returns 0 for no error and -1 for errors */
int error_whole_arg_check(char *parsed_arr[], int char_arg_len);

/* function that changes the val of corrected_path to path+cmd_name */
/* cmd_to_check might be grep or ps or other cmd
    On success return 0 ans corrected_path is set something like /usr/bin/grep
    Given /usr/bin is in the export path
    On failure returns -1 and corrected_path will be set to NULL */
int search_in_export_path_when_pipes( struct Node *export_head, char *cmd_to_check, int char_arg_len, char *corrected_path);

// main func needs parsed_arr, char_arg_len export_head
int main()
{
        // PARSER PART
        char *parsed_arr[MAX_INPUT_ARR_LEN];
        char *user_input ="cat < history.txt | grep man | grep foo | grep dog | grep bar";
        printf("User input is %s\n", user_input);
        int ptr_pos = 0;
        int char_arg_len=0;

        static char temp_input[MAX_CMD_INPUT_BUFFER];
        strcpy(temp_input, user_input);
        char *temp_cmd_arg_holder[MAX_CMD_INPUT_BUFFER];
        temp_cmd_arg_holder[ptr_pos] = strtok(temp_input, " ");
        parsed_arr[ptr_pos] = temp_cmd_arg_holder[0];
        while (parsed_arr[ptr_pos] != NULL)
        {
                char_arg_len++;
                temp_cmd_arg_holder[++ptr_pos] = strtok(NULL, " ");
                if (temp_cmd_arg_holder[ptr_pos] == NULL) {
                        break;
                }
                parsed_arr[ptr_pos] = temp_cmd_arg_holder[ptr_pos];

        }

        // Creating test export head
        struct Node *export_head = NULL;
        export_head = malloc(sizeof(Node));
        export_head->next = malloc(sizeof(Node));

        strcpy(export_head->content, "PATH=/bin:/usr/bin");
        strcpy(export_head->next->content, "TERM=xterm-256color");
        export_head->next->next = NULL;


        //////////////////////////////////////////////////////////////////////
        //////////// NOW THE REAL PART SRTAS /////////////////////////////////
        //////////////////////////////////////////////////////////////////////

        // PARSER PART END

        /* setting up the correct value for the env_var for execve
           i.e. char *env_var[] = {"PATH=/bin:/usr/bin", NULL}; */
        int env_var_index= 0;
        char *env_var[MAX_INPUT_ARR_LEN];

        // generating the env_var variable
        struct Node *cur = export_head;
        while ( cur != NULL ) {
                env_var[env_var_index] = cur->content;
                env_var_index += 1;
                cur = cur->next;
        }
        env_var[env_var_index] = NULL;

        /* indexes to denote what section of args from parsed_arr to use for each pipe*/
        int looplen = 0;
        pid_t wait_pid;
        int fork_pid;
        int child_status = 0;
        int start_index = 0; // for delimiting start of execve cmd_args
        int end_index = 0; // for delimiting start of execve cmd_args
        int pipes_loc[MAX_INPUT_ARR_LEN];
        int cur_pipe_being_handled = 1; // (Not 0 indexed) Index of current pipe being handled starts woth 1
        int remaining_pipes_to_be_handled = -1;
        int new_fds[2]; // for piping
        int old_fds[2]; // for piping
        char *built_in_cmd_arr[] = {"cd", "pwd", "exit", "pwd", "history", "export", "!", "\0"};

        parsed_arr[char_arg_len] = NULL;

        /* Checking if pipes exist */
        if  (check_pipe_rtn_loc(parsed_arr, char_arg_len, pipes_loc)== 0) {
                if (DEBUG==1) printf("Pipes were not disovered\n" ); // prints
                return -1; // Must return -1 to main function
        }
        else {
                for (int i = 1; i < pipes_loc[0]+1; i++) {
                        if (DEBUG==1) printf("Loc of pipes %i\n", pipes_loc[i] );
                }
        }
        if (DEBUG) {printf("Num of args %i and num of pipe is %i\n",char_arg_len, pipes_loc[0] ); }// 11
        remaining_pipes_to_be_handled = pipes_loc[0];
        int cmd_pos = pipes_loc[1]; // index location of first pipe cmd

        /* This declarartion might be optional */
        /* TRY_AND_CATCH(pipe(old_fds), "pipe"); */

        /* Checking for errors */
        if (error_whole_arg_check(parsed_arr, char_arg_len) != 0 ) {
                return -1;
        }

        /* Main loop it is assumed that spaces have been provided between cmds */
        /* When space is provided between redirects and pipes i.e. cat > file.txt
           instead of cat>file.txt */
        for (int i = 1; i < pipes_loc[0]+2; i++) {
                if (remaining_pipes_to_be_handled > 0) {
                        TRY_AND_CATCH(pipe(new_fds), "pipe");
                }
                else {
                        /* no more pipes to handle */
                        /* return with success */
                        /* Wait till all child processes are terminated */
                        close(new_fds[READ]);
                        close(new_fds[WRITE]);
                        close(old_fds[READ]);
                        close(old_fds[WRITE]);

                        /** wait till all children have terminated */
                        while ((wait_pid=wait(&child_status)) > 0);
                        return 0;
                }

                /* pipes_loc has the index of pipes [4, 1, 3, 6, 9]
                   pipes_loc[0] is the number of pipes so we start with i = 1
                   we access each pipe directly skipping other commands */
                cmd_pos = pipes_loc[i];
                looplen = strlen(parsed_arr[cmd_pos]);

                if (DEBUG == 1) {
                        printf("%s\n",parsed_arr[cmd_pos] );
                }

                if (looplen == 1) {
                        if (parsed_arr[cmd_pos][0] == '<') {
                                ///////////////////////////* stdin redirection *//////////////////////////////////////////
                                cur_pipe_being_handled += 1;
                                remaining_pipes_to_be_handled -= 1;

                                fork_pid = fork();
                                if ( fork_pid < 0 ) {
                                        perror("fork");
                                        exit(1);
                                }
                                /* Child process */
                                else if (fork_pid == 0) {
                                        FILE *stdin_fread = fopen(parsed_arr[cmd_pos+1], "r");

                                        /* All dup2 should be done in the child */
                                        dup2(fileno(stdin_fread), READ);

                                        /* < stdin redirect will always be the first pipe */

                                        /* if there is a next pipe cmd */
                                        if (remaining_pipes_to_be_handled > 0) {
                                                dup2(new_fds[WRITE], WRITE);
                                                close(new_fds[READ]);
                                                close(new_fds[WRITE]);
                                        }

                                        /* For running execve in the given form
                                           execve(path, cmd_to_run, env_var);
                                           grep main < history.txt */
                                        start_index = 0;
                                        end_index = cmd_pos;
                                        char *cmd_to_run[MAX_INPUT_ARR_LEN];
                                        for (int i = start_index; i < end_index; i++) {
                                                cmd_to_run[i] = parsed_arr[i];
                                        }
                                        cmd_to_run[end_index] = NULL;

                                        /* Checking for built-in cmds
                                            parsed_arr[0] is always checked
                                            and only 7 built-ins checked for now*/

                                        for (int i = 0; i < 7; i++) {
                                                if (strcmp(built_in_cmd_arr[i], parsed_arr[0]) == 0)
                                                {
                                                        printf("FOUND!!!\n");
                                                        fclose(stdin_fread);
                                                        exit(1);
                                                }
                                                if (strlen(built_in_cmd_arr[i]) == 1) {
                                                        if ( parsed_arr[0][0] == '!') {
                                                                printf("exclamantion found %c sdf\n", parsed_arr[0][0]);
                                                                fclose(stdin_fread);
                                                                exit(1);
                                                        }
                                                }
                                        }

                                        /* Seeting up the correct path
                                           if the cmd_to_check i.e. grep is found in env_var path
                                           corrected_path is set to /usr/bin/grep */
                                        char *cmd_to_check = parsed_arr[0]; // Assuming for stdin redirect parsed_arr[0] is alwyas the first
                                        char corrected_path[MAX_INPUT_ARR_LEN];
                                        if (search_in_export_path_when_pipes(export_head, cmd_to_check, char_arg_len, corrected_path) == 0) {
                                                if (DEBUG==1) {
                                                        printf("Command was found and full corrected path is %s and cmd_to_check is %s\n", corrected_path, cmd_to_check);
                                                }
                                        }
                                        /* Error in cmds and not found in path so return -1 */
                                        else {
                                                if (DEBUG==1) {printf("Command was not found and cmd_to_check is %s \n", cmd_to_check );}
                                                exit(1);
                                        }

                                        fclose(stdin_fread);
                                        execve(&corrected_path[0], cmd_to_run, env_var);
                                        perror("execve");
                                        exit(1);
                                }
                                /* Parent */
                                else {
                                        /* if there is a need for next piping */
                                        if (remaining_pipes_to_be_handled > 0) {
                                                old_fds[READ] = new_fds[READ];
                                                old_fds[WRITE] = new_fds[WRITE];
                                        }
                                }
                        }
                        else if (parsed_arr[cmd_pos][0] == '>') {
                                ///////////////////////////* redirecting stdout *//////////////////////////////////////////
                                cur_pipe_being_handled += 1;
                                remaining_pipes_to_be_handled -= 1;

                                fork_pid = fork();
                                if ( fork_pid < 0 ) {
                                        perror("fork");
                                        exit(1);
                                }
                                /* Child process */
                                else if (fork_pid == 0) {
                                        FILE *stdout_fwrite = fopen(parsed_arr[cmd_pos+1], "w");

                                        /* if there was a previous cmd
                                           i.e. grep main < history.txt > new.txt */
                                        if (cur_pipe_being_handled > 2) {
                                                dup2(old_fds[READ], READ);
                                                close(old_fds[READ]);
                                                close(old_fds[WRITE]);
                                        }
                                        /* if > is the first pipe
                                           i.e. ps -a > process.txt */
                                        else {
                                                dup2(fileno(stdout_fwrite), WRITE);

                                                /* For running execve in the given form
                                                   execve(path, cmd_to_run, env_var); */
                                                start_index = 0;
                                                end_index = cmd_pos;
                                                char *cmd_to_run[MAX_INPUT_ARR_LEN];
                                                for (int i = start_index; i < end_index; i++) {
                                                        cmd_to_run[i] = parsed_arr[i];
                                                }
                                                cmd_to_run[end_index] = NULL;

                                                /* Seeting up the correct path
                                                   if the cmd_to_check i.e. grep is found in env_var path
                                                   corrected_path is set to /usr/bin/grep or /bin/ps */
                                                char *cmd_to_check = parsed_arr[0]; // Assuming for first stdoutredirect parsed_arr[0] is alwyas the first
                                                char corrected_path[MAX_INPUT_ARR_LEN];
                                                if (search_in_export_path_when_pipes(export_head, cmd_to_check, char_arg_len, corrected_path) == 0) {
                                                        if (DEBUG==1) {
                                                                printf("Command was found and full corrected path is %s and cmd_to_check is %s\n", corrected_path, cmd_to_check);
                                                        }
                                                }
                                                /* Error in cmds and not found in path so return -1 */
                                                else {
                                                        if (DEBUG==1) {
                                                                printf("Command was not found and cmd_to_check is %s \n", cmd_to_check );
                                                        }
                                                        exit(1);
                                                }

                                                fclose(stdout_fwrite);
                                                execve(&corrected_path[0], cmd_to_run, env_var);
                                                perror("execve");
                                                exit(1);
                                        }

                                        /* If there is a need for next piping if > is not the last pipe
                                            i.e. cat < history.txt > process.txt 2> error.txt */
                                        if (remaining_pipes_to_be_handled > 0) {
                                                dup2(new_fds[WRITE], WRITE);
                                                close(new_fds[READ]);
                                                close(new_fds[WRITE]);
                                        }

                                        /* In This case > is the last pipe
                                           i.e. cat < history.txt > new.txt
                                           getc(stdin) gets stdin one char at a time till EOF
                                           fgets() cannot be used as it terminates at newline chars */
                                        int ch = getc(stdin);
                                        while (ch != EOF && ch != '\0')
                                        {
                                                /* save from stdin to stdout stream */
                                                putc(ch, stdout_fwrite);
                                                ch = getc(stdin);
                                        }

                                        if (feof(stdin)) {
                                                if (DEBUG==1) {printf("End of file reached.\n"); }
                                        }
                                        else {
                                                if (DEBUG ==1 ) {printf("Something went wrong.\n");}
                                                exit(1);
                                        }
                                        fclose(stdout_fwrite);
                                        exit(0);
                                }
                                /* Parent */
                                else {
                                        /* if there was a previous cmd */
                                        if (cur_pipe_being_handled > 2) {
                                                close(old_fds[READ]);
                                                close(old_fds[WRITE]);
                                        }
                                        /* if there is a need for next piping */
                                        if (remaining_pipes_to_be_handled > 0) {
                                                old_fds[READ] = new_fds[READ];
                                                old_fds[WRITE] = new_fds[WRITE];
                                        }
                                }
                        }
                        else if (parsed_arr[cmd_pos][0] == '|') {
                                ////////////////////////////////* piping  | *//////////////////////////////////////////
                                /* redirecting stdout */
                                cur_pipe_being_handled += 1;
                                remaining_pipes_to_be_handled -= 1;

                                fork_pid = fork();
                                if ( fork_pid < 0 ) {
                                        perror("fork");
                                        exit(1);
                                }
                                /* Child process */
                                else if (fork_pid == 0) {
                                        /* if there is a need for next piping */
                                        if (remaining_pipes_to_be_handled > 0) {
                                                dup2(new_fds[WRITE], WRITE);
                                                close(new_fds[READ]);
                                                close(new_fds[WRITE]);
                                        }
                                        /* if there was a previous cmd / pipe
                                           i.e. cat < history.txt | grep main
                                                ps | grep apache2 | grep 2 (For second pipe here)*/
                                        if (cur_pipe_being_handled > 2) {
                                                dup2(old_fds[READ], READ);
                                                close(old_fds[READ]);
                                                close(old_fds[WRITE]);
                                        }
                                        /* if | is the first pipe
                                           i.e. ps -a | grep bash
                                           For this we need a grandchild process */
                                        else {
                                                int lead_pipe_fds[2];
                                                TRY_AND_CATCH(pipe(lead_pipe_fds), "pipe");

                                                // Creating a grandchild here
                                                pid_t sub_child_pid = fork();

                                                if (sub_child_pid < 0) {
                                                        perror("fork");
                                                        exit(1);
                                                }
                                                /* Grandchild */
                                                else if (sub_child_pid == 0 ) {
                                                        dup2(lead_pipe_fds[WRITE_PIPE], WRITE);
                                                        close(lead_pipe_fds[READ]);
                                                        close(lead_pipe_fds[WRITE]);

                                                        /* For running execve in the given form
                                                           execve(path, cmd_to_run, env_var);
                                                           for cmd before |
                                                           ps -a | grep bash */
                                                        start_index = 0;
                                                        end_index = cmd_pos;
                                                        char *cmd_to_run[MAX_INPUT_ARR_LEN];
                                                        for (int i = start_index; i < end_index; i++) {
                                                                cmd_to_run[i] = parsed_arr[i];
                                                        }
                                                        cmd_to_run[end_index] = NULL;

                                                        /* Seeting up the correct path
                                                           if the cmd_to_check i.e. grep is found in env_var path
                                                           corrected_path is set to /usr/bin/grep or /bin/ps */
                                                        char *cmd_to_check = parsed_arr[0]; // Assuming for first stdoutredirect parsed_arr[0] is alwyas the first
                                                        char corrected_path[MAX_INPUT_ARR_LEN];
                                                        if (search_in_export_path_when_pipes(export_head, cmd_to_check, char_arg_len, corrected_path) == 0) {
                                                                if (DEBUG==1) {
                                                                        printf("Command was found and full corrected path is %s and cmd_to_check is %s\n", corrected_path, cmd_to_check);
                                                                }
                                                        }
                                                        /* Error in cmds and not found in path so return -1 */
                                                        else {
                                                                if (DEBUG==1) {
                                                                        printf("Command was not found and cmd_to_check is %s \n", cmd_to_check );
                                                                }
                                                                exit(1);
                                                        }

                                                        execve(&corrected_path[0], cmd_to_run, env_var);
                                                        perror("execve");
                                                        exit(1);
                                                }
                                                /*  Grandchild's Parent */
                                                else {
                                                        dup2(lead_pipe_fds[READ], READ);
                                                        close(lead_pipe_fds[READ]);
                                                        close(lead_pipe_fds[WRITE]);

                                                        int retrnStatus;
                                                        waitpid(fork_pid, &retrnStatus, 0); // Parent process waits here for child to terminate.
                                                        // Verify child process terminated without error.
                                                        if (retrnStatus == 0) {
                                                                // printf("Sub Child terminated normally.\n");
                                                        }
                                                        if (retrnStatus == 1) {
                                                                // printf("Sub Child terminated with an error.\n");
                                                                exit(1);
                                                        }

                                                        /* For running execve in the given form
                                                           execve(path, cmd_to_run, env_var);
                                                           for cmd before |
                                                           ps -a | grep bash */
                                                        start_index = cmd_pos + 1;

                                                        /* if no other pipes afer first pipe */
                                                        if ( remaining_pipes_to_be_handled <= 0) {
                                                                end_index = char_arg_len;
                                                        }
                                                        /* if other pipes remina to be executed
                                                           i.e. ps -a | grep bash > output.txt
                                                           pipes_loc = {2, 2, 5}*/
                                                        else {
                                                                end_index = pipes_loc[cur_pipe_being_handled];
                                                        }

                                                        char *cmd_to_run[MAX_INPUT_ARR_LEN];
                                                        int cmd_added_index = 0; // to denote index of cmd_to_run
                                                        for (int i = start_index; i < end_index; i++) {
                                                                cmd_to_run[cmd_added_index] = parsed_arr[i];
                                                                cmd_added_index += 1;
                                                        }
                                                        cmd_to_run[cmd_added_index] = NULL;

                                                        /* Seeting up the correct path
                                                           if the cmd_to_check i.e. grep is found in env_var path
                                                           corrected_path is set to /usr/bin/grep or /bin/ps */
                                                        char *cmd_to_check = parsed_arr[start_index]; // Assuming for first stdoutredirect parsed_arr[0] is alwyas the first
                                                        char corrected_path[MAX_INPUT_ARR_LEN];
                                                        if (search_in_export_path_when_pipes(export_head, cmd_to_check, char_arg_len, corrected_path) == 0) {
                                                                if (DEBUG==1) {
                                                                        printf("Command was found and full corrected path is %s and cmd_to_check is %s\n", corrected_path, cmd_to_check);
                                                                }
                                                        }
                                                        /* Error in cmds and not found in path so return -1 */
                                                        else {
                                                                if (DEBUG==1) {
                                                                        printf("Command was not found and cmd_to_check is %s \n", cmd_to_check );
                                                                }
                                                                exit(1);
                                                        }
                                                        execve(&corrected_path[0], cmd_to_run, env_var);
                                                        perror("execve");
                                                        exit(1);
                                                }
                                        }

                                        /* For calculating the start index */
                                        start_index = cmd_pos + 1;

                                        /* if no other pipes afer pipe
                                           i.e. cat < history.txt | grep main
                                           pipes_loc = {2, 1, 3}*/
                                        if ( remaining_pipes_to_be_handled <= 0) {
                                                end_index = char_arg_len;
                                        }
                                        /* if other pipes remain to be executed
                                           i.e. cat < history.txt | grep main | grep foo
                                           pipes_loc = {3, 1, 3, 6}
                                           end_index will be set to 6 */
                                        else {
                                                end_index = pipes_loc[cur_pipe_being_handled];
                                        }

                                        char *cmd_to_run[MAX_INPUT_ARR_LEN];
                                        int cmd_added_index = 0;    // to denote index of cmd_to_run
                                        for (int i = start_index; i < end_index; i++) {
                                                cmd_to_run[cmd_added_index] = parsed_arr[i];
                                                cmd_added_index += 1;
                                        }
                                        cmd_to_run[cmd_added_index] = NULL;

                                        /* Seeting up the correct path
                                           if the cmd_to_check i.e. grep is found in env_var path
                                           corrected_path is set to /usr/bin/grep or /bin/ps */
                                        char *cmd_to_check = parsed_arr[start_index]; // Assuming for first stdoutredirect parsed_arr[0] is alwyas the first
                                        char corrected_path[MAX_INPUT_ARR_LEN];
                                        if (search_in_export_path_when_pipes(export_head, cmd_to_check, char_arg_len, corrected_path) == 0) {
                                                if (DEBUG==1) {
                                                        printf("Command was found and full corrected path is %s and cmd_to_check is %s\n", corrected_path, cmd_to_check);
                                                }
                                        }
                                        /* Error in cmds and not found in path so return -1 */
                                        else {
                                                if (DEBUG==1) {
                                                        printf("Command was not found and cmd_to_check is %s \n", cmd_to_check );
                                                }
                                                exit(1);
                                        }

                                        execve( &corrected_path[0], cmd_to_run, env_var);
                                        perror("execve");
                                        exit(1);
                                }
                                /* Parent */
                                else {
                                        /* if there was a previous cmd */
                                        if (cur_pipe_being_handled > 2) {
                                                close(old_fds[READ]);
                                                close(old_fds[WRITE]);
                                        }
                                        /* if there is a need for next piping */
                                        if (remaining_pipes_to_be_handled > 0) {
                                                old_fds[READ] = new_fds[READ];
                                                old_fds[WRITE] = new_fds[WRITE];
                                        }
                                }
                        }
                        else {
                                if (DEBUG == 1) {
                                        printf("Error in one char redirection\n" );
                                }
                                close(new_fds[READ]);
                                close(new_fds[WRITE]);
                                close(old_fds[READ]);
                                close(old_fds[WRITE]);
                                /* Must return to main function here*/
                                return -1;
                        }
                }
                /* Again when space is provided between redirects and pipes i.e. cat >> file.txt
                   instead of cat>>file.txt */
                /* TO DO This section should only run once */
                else if (looplen == 2) {
                        if (parsed_arr[cmd_pos][0] == '2' && parsed_arr[cmd_pos][1] == '>') {
                                // stderr redirection 2>
                        }
                        else if (parsed_arr[cmd_pos][0] == '1' && parsed_arr[cmd_pos][1] == '>') {
                                // stdout redirection 1>
                        }
                        else if (parsed_arr[cmd_pos][0] == '>' && parsed_arr[cmd_pos][1] == '>') {
                                // stdout append redirection >>
                        }
                        else {
                                /*Error in redirection*/
                                if (DEBUG == 1) {
                                        printf("Error in two char redirection\n" );
                                }
                                close(new_fds[READ]);
                                close(new_fds[WRITE]);
                                close(old_fds[READ]);
                                close(old_fds[WRITE]);
                                /* Must return to main function here*/
                                return -1;
                        }
                }
                /* Parent checks if child process terminated correctly */
                int rtnStatus;
                waitpid(fork_pid, &rtnStatus, 0); // Parent process waits here for child to terminate.

                // Verify child process terminated without error.
                if (rtnStatus == 0) {
                        if (DEBUG == 1) {
                                printf("Child terminated normally.\n");
                        }
                }
                if (rtnStatus != 0) {
                        if (DEBUG == 1) {
                                printf("Child terminated with an error.\n");
                        }
                        return -1;
                }
        }
        /* Control should not reach here */
        if (DEBUG==1) {printf("CONTORL SHOULD NOT GET HERE \n");}
        return -2;
}

/* cmd_to_check might be grep or ps or other cmd
    On success return 0 ans corrected_path is set something like /usr/bin/grep
    Given /usr/bin is in the export path
    otherwise functions returns -1 and corrected_path will be set to NULL */
int search_in_export_path_when_pipes( struct Node *export_head, char *cmd_to_check, int char_arg_len, char *corrected_path) {
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

                        /* NOW temp_path_holder is /bin/ or /usr/bin*/
                        //search_dir_when_pipes
                        // (temp_path_holder, cmd_to_check, char_arg_len, export_head, corrected_path);



                        struct dirent *dir_entry; // define struct for a ptr for entering directory
                        DIR *dir_read = opendir(temp_path_holder); // returns a ptr of type DIR
                        // pid_t pid, wait_pid; // to hold process pids
                        // int status = 0;

                        /* if the directory specified by the path couldn't be opened */
                        if (dir_read == NULL) {
                                // printf("Error: could not open path directory\n");
                                return -1;
                        }


                        if ( (cmd_to_check[0] == '.' && cmd_to_check[1] == '/') || cmd_to_check[0] == '/') {
                                /* CHECK FOR SEGFAULT ERROS HERE */
                                strcpy(corrected_path, cmd_to_check);
                                return 0;
                        }

                        while ((dir_entry = readdir(dir_read)) != NULL) {
                                if ((strcmp(dir_entry->d_name, cmd_to_check)) == 0) {

                                        // cmd_to_check is the external cmd that has been entered i.e. grep
                                        // Now temp_path_holder = something like /usr/bin/grep
                                        strcat(temp_path_holder, cmd_to_check);

                                        /* CHECK FOR SEGFAULT ERROS HERE */
                                        strcpy(corrected_path, temp_path_holder);
                                        return 0;
                                }
                        }
                        closedir(dir_read);
                }
                cur = cur->next;
        }
        corrected_path = NULL;
        return -1;
}

int error_whole_arg_check(char *parsed_arr[], int char_arg_len) {
        int looplen=0;
        for (int cmd_pos = 0; cmd_pos < char_arg_len; cmd_pos++) {
                looplen = strlen(parsed_arr[cmd_pos]);
                if ( looplen == 1 ) {
                        if (parsed_arr[cmd_pos][0] == '<' ||
                            parsed_arr[cmd_pos][0] == '>' ||
                            parsed_arr[cmd_pos][0] == '|') {
                                if (error_check_pipe(parsed_arr, char_arg_len, cmd_pos) == 0 ) {
                                        if (DEBUG == 1) {
                                                printf("No errors\n");
                                        }
                                }
                                else {
                                        if (DEBUG == 1) {
                                                printf("-shell: syntax error in stdin redirection\n" );
                                        }
                                        return -1;
                                }
                        }
                }
                else if ( looplen == 2 ) {
                        if ((parsed_arr[cmd_pos][0] == '>' && parsed_arr[cmd_pos][1] == '>') ||
                            ((parsed_arr[cmd_pos][0] == '2' && parsed_arr[cmd_pos][1] == '>')) ||
                            ((parsed_arr[cmd_pos][0] == '1' && parsed_arr[cmd_pos][1] == '>'))) {
                                if (error_check_pipe(parsed_arr, char_arg_len, cmd_pos) == 0 ) {
                                        printf("No errors\n");
                                }
                                else {
                                        if (DEBUG == 1) {
                                                printf("-shell: syntax error in stdin redirection\n" );
                                        }
                                        return -1;
                                }
                        }
                }
                /* check for conditions like gre<p */
                else {
                        for (int j = 0; j < looplen; j++) {
                                if (parsed_arr[cmd_pos][j] == '<' ||
                                    parsed_arr[cmd_pos][j] == '>' ||
                                    parsed_arr[cmd_pos][j] == '|') {
                                        return -1;
                                }
                        }
                }
        }
        return 0; // No errors found
}

int check_pipe_rtn_loc(char *parsed_arr[], int char_arg_len, int pipes_loc_arr[]) {
        // static int pipes_loc_arr[MAX_INPUT_ARR_LEN];
        int pipes_loc_index = 0;

        /* Starting elem of arr is the total number of pipes found*/
        pipes_loc_arr[0] = pipes_loc_index;
        int repeat_pipe_check=0; // To check if pipes were repeated i.e. cat << hello

        for (int i = 0; i < char_arg_len; i++) {
                for (int j = 0; j < strlen(parsed_arr[i]); j++) {
                        if (parsed_arr[i][j] == '<' || parsed_arr[i][j] == '>' || parsed_arr[i][j] == '|') {
                                if (repeat_pipe_check == 0) {
                                        /* The 0th index component signifies if pipes were found and how many */
                                        pipes_loc_arr[0] += 1;
                                        pipes_loc_arr[++pipes_loc_index] = i;
                                }
                                /* preventing double add for >> pipes */
                                repeat_pipe_check = 1; // sets to 1 once pipe has been seen in one cmd arg
                        }
                }
                repeat_pipe_check = 0; // reset to 0 after every cmd arg
        }
        return pipes_loc_arr[0]; // Value is greater than 1f pipes were found
}

int error_check_pipe(char *parsed_arr[], int char_arg_len, int cur_index) {
        if (cur_index == char_arg_len-1 ) {
                /* Error in std in redirection
                   < provided with no following file i.e. cat < or ls | */
                return -1;
        }
        /* More error checking might be done here */
        /* first arg cannot be a pipe */
        if( parsed_arr[0][0] == '<' ||
            parsed_arr[0][0] == '>' ||
            parsed_arr[0][0] == '|') {
                return -1;
        }
        if ( (parsed_arr[cur_index+1][0] == '<' ||
              parsed_arr[cur_index+1][0] == '>' ||
              parsed_arr[cur_index+1][0] == '|')) {
                /* GIVEN a pipe has been found before
                   Checking for errors like cat > | file
                 */
                return -1;
        }
        /* No error in piping input */
        return 0;
}
