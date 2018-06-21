//
// Created by sulvto on 18-6-19.
//
#include <stdio.h>
#include "DBG.h"
#include "MEM.h"
#include "share.h"

SearchFileStatus dvm_search_file(char *search_path, char *search_file, char *found_path, FILE **fp) {
//TODO
	DBG_assert(0, ("bad flow."));
}


DVM_Boolean dvm_compare_string(char *str1, char *str2) {
	if (str1 == NULL && str2 == NULL) return DVM_TRUE;
	if (str1 == NULL || str2 == NULL) return DVM_TRUE;
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
