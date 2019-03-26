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

/* General pseudo code logic for pipes in loops
   for cmd in cmds
    if there is a next cmd
        pipe(new_fds)
    fork
    if child
        if there is a previous cmd
            dup2(old_fds[0], 0)
            close(old_fds[0])
            close(old_fds[1])
        if there is a next cmd
            close(new_fds[0])
            dup2(new_fds[1], 1)
            close(new_fds[1])
        exec cmd || die
    else
        if there is a previous cmd
            close(old_fds[0])
            close(old_fds[1])
        if there is a next cmd
            old_fds = new_fds
   if there are multiple cmds
    close(old_fds[0])
    close(old_fds[1])
 */

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

int main()
{
        // PARSER PART
        char *parsed_arr[MAX_INPUT_ARR_LEN];
        char *user_input ="!55 < history.txt ";//"| grep main > newer.txt";//"cat < history.txt  > new.txt"; //|  grep exit";// |     grep hoe 2> error.txt";
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

        // PARSER PART END
        char *env_var[] = {"PATH=/bin:/usr/bin", NULL};
        // char *dir_loop[] = {"/bin/", "/usr/bin/"};
        // for (int j = 0; j < char_arg_len; j++) {
        //         printf("STRING being checked is %s\n",parsed_arr[j] );
        //         if (strcmp(parsed_arr[j], "<") == 0 ||
        //             strcmp(parsed_arr[j], ">") == 0 ||
        //             strcmp(parsed_arr[j], "|") == 0 ||
        //             strcmp(parsed_arr[j], ">>") == 0 ||
        //             strcmp(parsed_arr[j], "2>") == 0 ||
        //             strcmp(parsed_arr[j], "1>") == 0) {
        //                 /* pass this check */
        //                 printf("Pipes not checked in path \n" );
        //                 continue;
        //         }
        //
        //         for (int i = 0; i < 2; i++) {
        //                 char checker[100];
        //                 // checker = strdup(dir_loop[i]);
        //                 strcpy(checker, dir_loop[i]);
        //                 strcat(checker, parsed_arr[j]);
        //                 printf("VALUE OF CHECKER is %s\n",checker );
        //                 if (access(checker, X_OK | F_OK)==0) {
        //                         printf("Accessible cmd %s\n",parsed_arr[j] );
        //                         break;
        //                 }
        //                 else {
        //                         printf("%s is not accessible \n",parsed_arr[j] );
        //                 }
        //         }
        // }



        // IMPORTANT PART START

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

        if  (check_pipe_rtn_loc(parsed_arr, char_arg_len, pipes_loc)== 0) {
                if (DEBUG==1) printf("Pipes were not disovered\n" ); // prints
                /* TODO */
                return -1; // Must return -1 to main function
        }
        else {
                for (int i = 1; i < pipes_loc[0]+1; i++) {
                        if (DEBUG==1) printf("Loc of pipes %i\n", pipes_loc[i] );
                }
        }
        printf("Num of args %i and num of pipe is %i\n",char_arg_len, pipes_loc[0] ); // 11
        remaining_pipes_to_be_handled = pipes_loc[0];
        int cmd_pos = pipes_loc[1]; // index location of first pipe cmd

        /* This declarartion might be optional */
        /* TRY_AND_CATCH(pipe(old_fds), "pipe"); */

        /* Checking for errors */
        if (error_whole_arg_check(parsed_arr, char_arg_len) == 0 ) {
                printf("No errors found\n");
        } else {
                printf("ERRORS FOUND in piped argument\n");
                return -1;
        }

        /* it is assumed that spaces have been provided between cmds */
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
                                // stdin redirection
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

                                        /* Checking for built-in cmds
                                            parsed_arr[0] is always checked
                                            and only 7 built-ins checked for now*/
                                        for (int i = 0; i < 7; i++) {
                                                if (strcmp(built_in_cmd_arr[i], parsed_arr[0]) == 0)
                                                {
                                                        printf("FOUND!!!\n");
                                                        exit(1);
                                                }
                                                if (strlen(built_in_cmd_arr[i]) == 1) {
                                                       if ( parsed_arr[0][0] == '!') {
                                                         printf("exclamantion found %c sdf\n", parsed_arr[0][0]);
                                                       }
                                                       exit(1);
                                                }
                                        }

                                        /* For running execve in the given form
                                           execve(path, cmd_to_run, env_var);
                                           grep main < history.txt */
                                        start_index = 0;
                                        end_index = cmd_pos;
                                        char *cmd_to_run[MAX_INPUT_ARR_LEN];
                                        for (int i = 0; i < end_index; i++) {
                                                cmd_to_run[i] = parsed_arr[i];
                                        }
                                        cmd_to_run[end_index] = NULL;

                                        //////////////////////////////////////////////////////////////////////
                                        //////////// TODO MUST BE CHANGED IN MAIN FUNC ///////////////////////
                                        /////////////////////SUPPORT FOR BUILT IN/////////////////////////////


                                        char *path = "/usr/bin/grep";

                                        fclose(stdin_fread);
                                        execve(path, cmd_to_run, env_var);
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
                                        FILE *stdout_fwrite = fopen(parsed_arr[cmd_pos+1], "w");

                                        /* if there was a previous cmd
                                           i.e. cat < history.txt > new.txt */
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
                                                for (int i = 0; i < end_index; i++) {
                                                        cmd_to_run[i] = parsed_arr[i];
                                                }
                                                cmd_to_run[end_index] = NULL;

                                                //////////////////////////////////////////////////////////////////////
                                                //////////// TODO MUST BE CHANGED IN MAIN FUNC ///////////////////////
                                                /////////////////////SUPPORT FOR BUILT IN/////////////////////////////
                                                char *path = "/bin/ps";

                                                fclose(stdout_fwrite);
                                                execve(path, cmd_to_run, env_var);
                                                perror("execve");
                                                exit(1);
                                        }

                                        /* if there is a need for next piping
                                           This section might be optional here */
                                        if (remaining_pipes_to_be_handled > 0) {
                                                dup2(fileno(stdout_fwrite), WRITE);
                                                close(new_fds[READ]);
                                                close(new_fds[WRITE]);
                                        }

                                        /* This case > is the last pipe cat < history.txt > new.txt
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
                                /* piping  | */
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
                                           i.e. ps -a | grep main
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
                                                           execve(path, cmd_to_run, env_var); */
                                                        start_index = 0;
                                                        end_index = cmd_pos;
                                                        char *cmd_to_run[MAX_INPUT_ARR_LEN];
                                                        for (int i = 0; i < end_index; i++) {
                                                                cmd_to_run[i] = parsed_arr[i];
                                                        }
                                                        cmd_to_run[end_index] = NULL;

                                                        //////////////////////////////////////////////////////////////////////
                                                        //////////// TODO MUST BE CHANGED IN MAIN FUNC ///////////////////////
                                                        /////////////////////SUPPORT FOR BUILT IN/////////////////////////////

                                                        char *path = "/bin/ps";

                                                        execve(path, cmd_to_run, env_var);
                                                        perror("execve");
                                                        exit(1);

                                                        // execve();
                                                        // perror("execve");
                                                        // exit(1);
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

                                                        //////////////////////////////////////////////////////////////////////
                                                        //////////// TODO MUST BE CHANGED IN MAIN FUNC ///////////////////////
                                                        /////////////////////SUPPORT FOR BUILT IN/////////////////////////////
                                                        char *cmd_to_run[MAX_INPUT_ARR_LEN];
                                                        cmd_to_run[0] = "grep";
                                                        cmd_to_run[1] = "main";
                                                        cmd_to_run[2] = NULL;
                                                        char *path = "/usr/bin/grep";

                                                        execve(path, cmd_to_run, env_var);
                                                        perror("execve");
                                                        exit(1);

                                                        // execve();
                                                        // perror("execve");
                                                        // exit(1);
                                                }
                                        }

                                        //////////////////////////////////////////////////////////////////////
                                        //////////// TODO MUST BE CHANGED IN MAIN FUNC ///////////////////////
                                        /////////////////////SUPPORT FOR BUILT IN/////////////////////////////
                                        char *cmd_to_run[MAX_INPUT_ARR_LEN];
                                        cmd_to_run[0] = "grep";
                                        cmd_to_run[1] = "main";
                                        cmd_to_run[2] = NULL;
                                        char *path = "/usr/bin/grep";

                                        execve(path, cmd_to_run, env_var);
                                        perror("execve");
                                        exit(1);
                                        // execve(); // run execve
                                        // perror("execve");
                                        // exit(1);
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
                                /*TODO*/
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
                /* Parent always closes old fds */


                /* Parent checks if child process terminated correctly */
                int rtnStatus;
                waitpid(fork_pid, &rtnStatus, 0); // Parent process waits here for child to terminate.

                // Verify child process terminated without error.
                if (rtnStatus == 0) {
                        printf("Child terminated normally.\n");
                }
                if (rtnStatus == 1) {
                        printf("Child terminated with an error.\n");
                        return -1;
                }

        }
        /* Control should not reach here */
        if (DEBUG==1) {printf("CONTORL SHOULD NOT GET HERE \n");}
        return -2;
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


// cat < history.txt | grep exit 2> error.txt
// FILE *reader = fopen("history.txt", "r");
// char *grep_cmd[]={"/usr/bin/grep", "exit", NULL};
// char *cat[] = {"/bin/cat", NULL};
// int catc;
// int grepc;
// int fd1[2];
// // int fd2[2];
// pipe(fd1);
// // pipe(fd2[2]);
//
//
//
// if (fork() == 0) {
//         dup2(fileno(reader), READ);
//         dup2(fd1[WRITE], WRITE);
//
//         close(fd1[READ]);
//         close(fd1[WRITE]);
//
//         execv(cat[0], cat);
//         perror("exec");
// }
//
// if (fork() == 0) {
//         dup2(fd1[READ], READ);
//
//         close(fd1[READ]);
//         close(fd1[WRITE]);
//
//         execv(grep_cmd[0], grep_cmd);
//         perror("exec");
// }
// close(fd1[READ]);
// close(fd1[WRITE]);
// fclose(reader);


//Implementing
//cat history.txt  | grep exit | grep hoe > FINAL.txt
// FILE *fptr = fopen("FINAL.txt", "w");
// char *cat_cmd[]={"/bin/cat", "history.txt", NULL};
// char *grep_cmd[]={"/usr/bin/grep", "exit", NULL};
// char *grep2_cmd[]={"/usr/bin/grep", "hoe", NULL};
// int cat_child;
// int grep_child;
// int grep2_child;
//
// int fd_pipe1[2];
// int fd_pipe2[2];
//
// pipe(fd_pipe1);
// pipe(fd_pipe2);
//
// cat_child = fork();
// if(cat_child == 0 ) {
//
//         dup2(fd_pipe1[WRITE], WRITE);
//
//         close(fd_pipe1[READ]);
//         close(fd_pipe1[WRITE]);
//
//         execv(cat_cmd[0], cat_cmd);
//         perror("exec");
// }
//
// grep_child = fork();
// if(grep_child == 0 ) {
//
//         dup2(fd_pipe1[READ], READ);
//         dup2(fd_pipe2[WRITE], WRITE);
//
//         close(fd_pipe1[WRITE]);
//         close(fd_pipe1[READ]);
//         close(fd_pipe2[WRITE]);
//         close(fd_pipe2[READ]);
//
//         execv(grep_cmd[0], grep_cmd);
//         perror("exec");
// }
// close(fd_pipe1[READ]);
// close(fd_pipe1[WRITE]);
//
// grep2_child = fork();
// if(grep2_child == 0) {
//         dup2(fd_pipe2[READ], READ);
//         dup2(fileno(fptr), WRITE);
//
//         close(fd_pipe2[WRITE]);
//         close(fd_pipe2[READ]);
//
//         execv(grep2_cmd[0], grep2_cmd);
//         perror("exec");
// }
// fclose(fptr);
// close(fd_pipe2[READ]);
// close(fd_pipe2[WRITE]);
