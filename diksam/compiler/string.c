//
// Created by sulvto on 18-6-9.
//
#include "MEM.h"
#include "diksamc.h"

#define STRING_ALLOC_SIZE   (256)

static char *st_string_literal_buffer = NULL;
static int st_string_literal_buffer_size = 0;
static int st_string_literal_buffer_alloc_size = 0;

char *dkc_create_identifier(char *string) {
	char *new_string = dkc_malloc(strlen(string) + 1);
	strcpy(new_string, string);

	return new_string;
}

void dkc_open_string_literal(void) {
	st_string_literal_buffer = 0;
}

DVM_Char *dkc_close_string_literal(void) {
	DVM_Char *new_string;

	dkc_add_string_literal('\n');

	int length = dvm_mbstowcs_len(st_string_literal_buffer);
	if (length < 0) {
		dkc_compile_error(dkc_get_current_compiler()->current_line_number,
							BAD_MULTIBYTE_CHARACTER_ERR,
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

void dkc_reset_string_literal_buffer(void) {
	MEM_free(st_string_literal_buffer);
	st_string_literal_buffer = NULL;
	st_string_literal_buffer_size = 0;
	st_string_literal_buffer_alloc_size = 0;
}

int dkc_close_character_literal(void) {
	DVM_Char buf[16];
	dkc_add_string_literal('\0');
	int new_string_len = dvm_mbstowcs_len(st_string_literal_buffer);
	if (new_string_len < 0) {
		dkc_compile_error(dkc_get_current_compiler()->current_line_number, BAD_MULTIBYTE_CHARACTER_ERR,
		                  MESSAGE_ARGUMENT_END);
	} else if (new_string_len > 1) {
		dkc_compile_error(dkc_get_current_compiler()->current_line_number, TOO_LONG_CHARACTER_LITERAL_ERR,
		                  MESSAGE_ARGUMENT_END);
	}

	dvm_mbstowcs(st_string_literal_buffer, buf);

	return buf[0];
}
