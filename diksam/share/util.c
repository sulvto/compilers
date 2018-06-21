//
// Created by sulvto on 18-6-19.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "DBG.h"
#include "MEM.h"
#include "share.h"

SearchFileStatus dvm_search_file(char *search_path, char *search_file, char *found_path) {
	char dir_path[FILENAME_MAX];

	int search_file_len = strlen(search_file);
	for (int sp_index = 0, dp_index = 0; ; sp_index++) {
		if (search_path[sp_index] == FILE_PATH_SEPARATOR || search_path[sp_index] == '\0') {
			if (dp_index + 1 + search_file_len >= FILENAME_MAX-1) {
				return SEARCH_FILE_PATH_TOO_LONG;
			}

			if (dp_index > 0 && dir_path[dp_index - 1] != FILE_SEPARATOR) {
				dir_path[dp_index] = FILE_SEPARATOR;
				dp_index++;
			}

			strcpy(&dir_path[dp_index], search_file);
			FILE *fp_tmp = fopen(dir_path, "r");
			if (fp_tmp != NULL) {
				fclose(fp_tmp);
				strcpy(found_path, dir_path);
				return SEARCH_FILE_SUCCESS;
			}

			dp_index = 0;

			if (search_path[sp_index] == '\0') {
				return SEARCH_FILE_NOT_FOUND;
			}
		} else {
			char *home;
			if (dp_index == 0 && search_path[sp_index] == '~' && (home = getenv("HOME")) != NULL) {
				strcpy(&dir_path[dp_index], home);
				dp_index += strlen(home);
			} else {
				if (dp_index >= FILENAME_MAX - 1) {
					return SEARCH_FILE_PATH_TOO_LONG;
				}
				dir_path[dp_index] = search_path[sp_index];
				dp_index++;
			}
		}
	}

	DBG_assert(0, ("bad flow.\n"));
}


DVM_Boolean dvm_compare_string(char *str1, char *str2) {
	if (str1 == NULL && str2 == NULL) return DVM_TRUE;
	if (str1 == NULL || str2 == NULL) return DVM_FALSE;
	return strcmp(str1, str2) == 0;
}

DVM_Boolean dvm_compare_package_name(char *p1, char *p2) {
	return dvm_compare_string(p1, p2);
}

char *dvm_create_method_function_name(char *class_name, char *method_name){
	int class_name_length = strlen(class_name);
	int method_name_length = strlen(method_name);
	char *result = MEM_malloc(class_name_length + method_name_length + 2);
	sprintf(result, "%s#%s", class_name, method_name);

	return result;
}

void dvm_strncpy(char *dest, char *src, int buf_size) {
	int i;
	for (i = 0; i < (buf_size - 1) && src[i] != '\0'; i++) {
		dest[i] = src[i];
	}
	dest[i] = '\0';
}
