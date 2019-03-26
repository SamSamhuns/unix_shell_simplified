#ifndef PIPE_H // Making sure the header files are not included twice
#define PIPE_H
#include "common.h"

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

#endif
