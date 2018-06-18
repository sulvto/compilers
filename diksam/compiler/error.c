//
// Created by sulvto on 18-6-10.
//

#include <stdarg.h>
#include "diksamc.h"

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

static void format_message(int line_number, ErrorDefinition *format, VString *v, va_list ap) {
	int i;
	char buf[LINE_BUF_SIZE];
	DVM_Char wc_buf[LINE_BUF_SIZE];
	int argument_name_index;
	char argument_name[LINE_BUF_SIZE];
	MessageArgument argument[MESSAGE_ARGUMENT_MAX];
	MessageArgument current_argument;

	create_message_argument(argument_name, ap);

	DVM_Char *wc_format = dkc_mbstowcs_alloc(line_number, format->format);
	DBG_assert(wc_format != NULL, ("wc_format is null.\n"));

	for (int i = 0; wc_format[i]!= L'\0'; i++) {
		if (wc_format[i] != L'$') {
			dkc_vstr_append_character(v, wc_format[i]);
			continue;
		}
		assert(wc_format[i+1] == L'(');
		i += 2;
		for (argument_name_index = 0; wc_format[i] != L')'; argument_name_index++, i++) {
			// TODO
		}
	}
}

static self_check() {
	if  (strcmp(dkc_error_message_format[0].format, "dummy") != 0) {
		DBG_panic(("compile error message format error.\n"));
	}
	if (strcmp(dkc_error_message_format[COMPILE_ERROR_COUNT_PLUS_1].format, "dummy") != 0) {
		DBG_panic(("compile error message format error. COMPILE_ERROR_COUNT_PLUS_1..%d\n", COMPILE_ERROR_COUNT_PLUS_1));	
	}
}

void dkc_compiler_error(int line_number, CompileError id, ...) {
	va_list ap;
	VString message;
	self_check();
	va_start(ap, id);

	dkc_cstr_clear(&message);
	format_message(line_number, &dkc_error_message_format[id], &message, ap);
	fprintf(stderr, "%3d:", line_number);
	dvm_printf_wcs_ln(stderr, message.string);
	va_end(ap);

	exit(1);
}
