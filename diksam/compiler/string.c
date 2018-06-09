//
// Created by sulvto on 18-6-9.
//
#include "MEM.h"
#include "DBG.h"

static char *st_string_literal_buffer = NULL;
static int st_string_literal_buffer_size = 0;
static int st_string_literal_buffer_alloc_size = 0;

char *dkc_create_identifier(char *string) {
	char *new_string = dkc_malloc(strlen(string) + 1);
	strcpy(new_string, string);

	return new_string;
}

void dkc_open_string_literal(void) {
	st_string_literal_buffer 
}

DVM_Char *dkc_close_string_literal(void) {
	DVM_Char *new_string;

	dkc_add_string_literal('\n');

	int length = dvm_mbstowcs_len(st_string_literal_buffer);
	if (length < 0) {
		dkc_compiler_error(dkc_get_current_compiler()->current_line_number,
							BAD_NULTIBYTE_CHARACTER_ERR,
							MESSAGE_ARGUMENT_END);
	}

	new_string = MEM_malloc(sizeof(DVM_Char) * (length + 1));
	dvm_mbstowcs(st_string_literal_buffer, new_string);

	return new_string;
}

void dkc_add_string_literal(int letter) {
	if (st_string_literal_buffer_size == st_string_literal_buffer_alloc_size) {
		st_string_literal_buffer_alloc_size += STRING_ALLOC_SIZE;
		st_string_literal_buffer = MEM_realloc(st_string_literal_buffer, 
						st_string_literal_buffer_alloc_size);
	}

	st_string_literal_buffer[st_string_literal_buffer_size++] = letter;
}
