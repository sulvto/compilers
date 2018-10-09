//
// Created by sulvto on 18-6-10.
//
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "MEM.h"
#include "DBG.h"
#include "diksamc.h"

extern char *yytext;
extern ErrorDefinition dkc_error_message_format[];

typedef struct {
	MessageArgumentType type;
	char *name;
	union {
		int int_value;
		double double_value;
		char *string_value;
		void *pointer_value;
		int character_value;
	} u;
} MessageArgument;

static void create_message_argument(MessageArgument *argument, va_list ap) {
	int index;
	MessageArgumentType type;

	while ((type = va_arg(ap, MessageArgumentType)) != MESSAGE_ARGUMENT_END) {
		argument[index].type = type;
		argument[index].name = va_arg(ap, int);
		switch (type) {
			case INT_MESSAGE_ARGUMENT:
				argument[index].u.int_value = va_arg(ap, int);
				break;
			case DOUBLE_MESSAGE_ARGUMENT:
				argument[index].u.double_value = va_arg(ap, double);
				break;
			case STRING_MESSAGE_ARGUMENT:
				argument[index].u.string_value = va_arg(ap, char*);
				break;
			case POINTER_MESSAGE_ARGUMENT:
				argument[index].u.pointer_value = va_arg(ap, void*);
				break;
			case CHARACTER_MESSAGE_ARGUMENT:
				argument[index].u.character_value = va_arg(ap, int);
				break;
			case MESSAGE_ARGUMENT_END:
				assert(0);
				break;
			default:
				assert(0);
		}
		index++;
		assert(index < MESSAGE_ARGUMENT_MAX);
	}
	printf("create_message_argument end\n");
}

static void search_argument(MessageArgument *argument_list, char *argument_name, MessageArgument *argument) {
	for (int i = 0; argument_list[i].type != MESSAGE_ARGUMENT_END; i++) {
		if (strcmp(argument_list[i].name, argument_name)==0) {
			*argument = argument_list[i];
			return;
		}
	}
	assert(0);
}

static void format_message(int line_number, ErrorDefinition *format, VWString *v, va_list ap) {
	int i;
	char buf[LINE_BUF_SIZE];
	DVM_Char wc_buf[LINE_BUF_SIZE];
	int argument_name_index;
	char argument_name[LINE_BUF_SIZE];
	MessageArgument argument[MESSAGE_ARGUMENT_MAX];
	MessageArgument current_argument;

	create_message_argument(argument, ap);

	DVM_Char *wc_format = dkc_mbstowcs_alloc(line_number, format->format);
	DBG_assert(wc_format != NULL, ("wc_format is null.\n"));

	for (int i = 0; wc_format[i] != L'\0'; i++) {
		if (wc_format[i] != L'$') {
			dkc_vwstr_append_character(v, wc_format[i]);
			continue;
		}
		assert(wc_format[i + 1] == L'(');
		i += 2;
		for (argument_name_index = 0; wc_format[i] != L')'; argument_name_index++, i++) {
			argument_name[argument_name_index] = dvm_wctochar(wc_format[i]);
		}
		argument_name[argument_name_index] = '\0';
		assert(wc_format[i] == L')');
		search_argument(argument, argument_name, &current_argument);
		switch (current_argument.type) {
			case INT_MESSAGE_ARGUMENT:
				sprintf(buf, "%d", current_argument.u.int_value);
				dvm_mbstowcs(buf, wc_buf);
				dkc_vwstr_append_string(v, wc_buf);
				break;
			case DOUBLE_MESSAGE_ARGUMENT:
				sprintf(buf, "%f", current_argument.u.double_value);
				dvm_mbstowcs(buf, wc_buf);
				dkc_vwstr_append_string(v, wc_buf);
				break;
			case STRING_MESSAGE_ARGUMENT:
				dvm_mbstowcs(current_argument.u.string_value, wc_buf);
				dkc_vwstr_append_string(v, wc_buf);
				break;
			case CHARACTER_MESSAGE_ARGUMENT:
				sprintf(buf, "%c", current_argument.u.character_value);
				dvm_mbstowcs(buf, wc_buf);
				dkc_vwstr_append_string(v, wc_buf);
				break;
			case POINTER_MESSAGE_ARGUMENT:
				sprintf(buf, "%p", current_argument.u.pointer_value);
				dvm_mbstowcs(buf, wc_buf);
				dkc_vwstr_append_string(v, wc_buf);
				break;
			case MESSAGE_ARGUMENT_END:
				assert(0);
				break;
			default:
				assert(0);
		}
	}
	MEM_free(wc_format);
}

static void self_check() {
	if  (strcmp(dkc_error_message_format[0].format, "dummy") != 0) {
		DBG_panic(("compile error message format error.\n"));
	}
	if (strcmp(dkc_error_message_format[COMPILE_ERROR_COUNT_PLUS_1].format, "dummy") != 0) {
		DBG_panic(("compile error message format error. COMPILE_ERROR_COUNT_PLUS_1..%d\n", COMPILE_ERROR_COUNT_PLUS_1));
	}
}

void dkc_compile_error(int line_number, CompileError id, ...) {
	va_list ap;
	VWString message;
	self_check();
	va_start(ap, id);

	dkc_vwstr_clear(&message);
	format_message(line_number, &dkc_error_message_format[id], &message, ap);
	fprintf(stderr, "%s:%3d:", dkc_get_current_compiler()->path, line_number);
	dvm_print_wcs_ln(stderr, message.string);
	va_end(ap);

	exit(1);
}

int yyerror(char const *str) {
	dkc_compile_error(dkc_get_current_compiler()->current_line_number, PARSE_ERR,
	                  STRING_MESSAGE_ARGUMENT, "token", yytext,
	                  MESSAGE_ARGUMENT_END);

	return 0;
}