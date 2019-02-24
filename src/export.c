#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "common.h"
#include "export.h"

/* function to handle cmd export env prints */
void print_env_export_handler(struct Node *export_head) {
        print_linked_list_export(export_head);
}

/* function to save env vars entered using export
   returns sthe status of the save env attempt
   0=success, -1=failure */
int save_env_export_handler(char *parsed_arr[],
                            int char_arg_len, struct Node *export_head) {
        int equal_sign_count = 0;
        for (size_t i = 1; i < char_arg_len; i++) {
                char *env_var_array[2]; /* array to hold env name and value*/
                char temp_content_store[FILENAME_MAX]; /* temp char pointer arr to
                  store the value of ith parsed_input arr elem */
                strcpy(temp_content_store, parsed_arr[i]);

                equal_sign_count = 0; /* Counting number of equal signs for validity */
                /* if = is encountered immediately, it is invalid */
                if (parsed_arr[i][0] == '=') {
                        return -1;
                }
                for (size_t j = 0; j < strlen(parsed_arr[i]); j++) {
                        if (parsed_arr[i][j] == '=') {
                                equal_sign_count++;
                        }
                }
                /* equal count must be one for a valid export env_var cmd */
                if (equal_sign_count != 1) {
                        return -1;
                }

                env_var_array[0] = strtok(temp_content_store,"=");
                env_var_array[1] = strtok(NULL, "=");

                push_export(export_head, parsed_arr[i], env_var_array[0], env_var_array[1]);
        }
        return 0;
}
