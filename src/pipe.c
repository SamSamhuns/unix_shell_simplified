#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "pipe.h"
#include "common.h"


/* function that changes the val of corrected_path to path+cmd_name */
/* cmd_to_check might be grep or ps or other cmd
    On success return 0 ans corrected_path is set something like /usr/bin/grep
    Given /usr/bin is in the export path
    On failure returns -1 and corrected_path will be set to NULL */
int search_in_export_path_when_pipes( struct Node *export_head, char *cmd_to_check, int char_arg_len, char *corrected_path) {
	struct Node *cur = export_head;
	while (cur != NULL) {
		static char *export_var_array[2];         /* To hold env var name and value */
		static char *export_var_array_paths[MAX_INPUT_KWRD_LEN];         /* to hold
		                                                                    different paths in the env var values */
		int num_of_paths = 0;         /* number of paths in the env_var */
		int path_iter = 0;         /* for iteration splitting using strtok */
		static char temp_store[MAX_INPUT_KWRD_LEN];         /* temp store to make contents
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



			struct dirent *dir_entry;             // define struct for a ptr for entering directory
			DIR *dir_read = opendir(temp_path_holder);             // returns a ptr of type DIR
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
	/* Corrected path will not be used */
	// corrected_path = NULL;
	return -1;
}

/* Check for errors after pipes have been seen
   returns 0 on no error and -1 on ERRORS found */
int error_whole_arg_check(char *parsed_arr[], int char_arg_len) {
	for (int cmd_pos = 0; cmd_pos < char_arg_len; cmd_pos++) {
		int looplen = strlen(parsed_arr[cmd_pos]);
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
	return 0;     // No errors found
}

/* Checks if piping or redirection has been entered in the input cmd
   And if pipes were present then return value if greater than 1 and reprs the
   total number of pipes found
   if return arr is check_return_arr = check_pipe_rtn_loc(parsed_arr, char_arg_len)
   check_return_arr[0] is the total number of pipes found*/
int check_pipe_rtn_loc(char *parsed_arr[], int char_arg_len, int pipes_loc_arr[]) {
	// static int pipes_loc_arr[MAX_INPUT_ARR_LEN];
	int pipes_loc_index = 0;

	/* Starting elem of arr is the total number of pipes found*/
	pipes_loc_arr[0] = pipes_loc_index;
	int repeat_pipe_check=0;     // To check if pipes were repeated i.e. cat << hello

	for (int i = 0; i < char_arg_len; i++) {
		for (int j = 0; j < strlen(parsed_arr[i]); j++) {
			if (parsed_arr[i][j] == '<' || parsed_arr[i][j] == '>' || parsed_arr[i][j] == '|') {
				if (repeat_pipe_check == 0) {
					/* The 0th index component signifies if pipes were found and how many */
					pipes_loc_arr[0] += 1;
					pipes_loc_arr[++pipes_loc_index] = i;
				}
				/* preventing double add for >> pipes */
				repeat_pipe_check = 1;                 // sets to 1 once pipe has been seen in one cmd arg
			}
		}
		repeat_pipe_check = 0;         // reset to 0 after every cmd arg
	}
	return pipes_loc_arr[0];     // Value is greater than 1f pipes were found
}

/* Check for errors after pipes have been seen
   returns 0 on no error and -1 on ERRORS found */
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
