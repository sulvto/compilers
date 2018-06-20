//
// Created by sulvto on 18-6-18.
//

#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include "DBG.h"
#include "MEM.h"
#include "DVM_dev.h"
#include "dvm_pri.h"

typedef struct {
	DVM_MessageArgumentType type;
	char    *name;
	union {
		int int_value;
		int double_value;
		char *string_value;
		void *pointer_value;
		int character_value;
	} u;
} MessageArgument;

static void create_message_argument(MessageArgument *argument, va_list ap) {
	int index;
	DVM_MessageArgumentType type;

	while ((type = va_arg(ap, DVM_MessageArgumentType)) != DVM_MESSAGE_ARGUMENT_END) {
		argument[index].type = type;
		argument[index].name = va_arg(ap, int);
		switch (type) {
			case DVM_INT_MESSAGE_ARGUMENT:
				argument[index].u.int_value = va_arg(ap, int);
				break;
			case DVM_DOUBLE_MESSAGE_ARGUMENT:
				argument[index].u.double_value = va_arg(ap, double);
				break;
			case DVM_STRING_MESSAGE_ARGUMENT:
				argument[index].u.string_value = va_arg(ap, char*);
				break;
			case DVM_POINTER_MESSAGE_ARGUMENT:
				argument[index].u.pointer_value = va_arg(ap, void*);
				break;
			case DVM_CHARACTER_MESSAGE_ARGUMENT:
				argument[index].u.character_value = va_arg(ap, int);
				break;
			case DVM_MESSAGE_ARGUMENT_END:
				assert(0);
				break;
			default:
				assert(0);
		}
		index++;
		assert(index < MESSAGE_ARGUMENT_MAX);
	}
}

static void search_argument(MessageArgument *argument_list, char *argument_name, MessageArgument *argument) {
	for (int i = 0; argument_list[i].type != DVM_MESSAGE_ARGUMENT_END; i++) {
		if (strcmp(argument_list[i].name, argument_name)==0) {
			*argument = argument_list[i];
			return;
		}
	}
	assert(0);
}

static void format_message(DVM_ErrorDefinition *format, VString *v, va_list ap) {
	int i;
	char buf[LINE_BUF_SIZE];
	DVM_Char wc_buf[LINE_BUF_SIZE];
	int argument_name_index;
	char argument_name[LINE_BUF_SIZE];
	MessageArgument argument[MESSAGE_ARGUMENT_MAX];
	MessageArgument current_argument;

	create_message_argument(argument, ap);

	DVM_Char *wc_format = dkc_mbstowcs_alloc(NULL, format->format);
	DBG_assert(wc_format != NULL, ("wc_format is null.\n"));

	for (int i = 0; wc_format[i]!= L'\0'; i++) {
		if (wc_format[i] != L'$') {
			dkc_vstr_append_character(v, wc_format[i]);
			continue;
		}
		assert(wc_format[i+1] == L'(');
		i += 2;
		for (argument_name_index = 0; wc_format[i] != L')'; argument_name_index++, i++) {
			argument_name[argument_name_index] = dvm_wctochar(wc_format[i]);
		}
		argument_name[argument_name_index] = '\0';
		assert(wc_format[i] == L')');
		search_argument(argument, argument_name, &current_argument);
		switch (current_argument.type) {
			case DVM_INT_MESSAGE_ARGUMENT:
				sprintf(buf, "%d", current_argument.u.int_value);
				dvm_mbstowcs(buf, wc_buf);
				dkc_vwstr_append_string(v, wc_buf);
				break;
			case DVM_DOUBLE_MESSAGE_ARGUMENT:
				sprintf(buf, "%f", current_argument.u.double_value);
				dvm_mbstowcs(buf, wc_buf);
				dkc_vwstr_append_string(v, wc_buf);
				break;
			case DVM_STRING_MESSAGE_ARGUMENT:
				dvm_mbstowcs(current_argument.u.string_value, wc_buf);
				dkc_vwstr_append_string(v, wc_buf);
				break;
			case DVM_CHARACTER_MESSAGE_ARGUMENT:
				sprintf(buf, "%c", current_argument.u.character_value);
				dvm_mbstowcs(buf, wc_buf);
				dkc_vwstr_append_string(v, wc_buf);
				break;
			case DVM_POINTER_MESSAGE_ARGUMENT:
				sprintf(buf, "%p", current_argument.u.pointer_value);
				dvm_mbstowcs(buf, wc_buf);
				dkc_vwstr_append_string(v, wc_buf);
				break;
			case DVM_MESSAGE_ARGUMENT_END:
				assert(0);
				break;
		}
	}
	MEM_free(wc_format);
}

static void error_v(DVM_Executable *executable, Function *function, int pc, DVM_ErrorDefinition *error_definition, RuntimeError id,
                    va_list ap) {
	VString message;
	int line_number;
	dvm_vstr_clear(&message);
	format_message(&error_definition[id], &message, ap);

	if (pc != NO_LINE_NUMBER_PC) {
		line_number = dvm_conv_pc_to_line_number(executable, function, pc);
		fprintf(stderr, "%s:%d:", executable->path, line_number);
	}
	dvm_print_wcs_ln(stderr, message.string);
}

static void self_check() {
	if (strcmp(dvm_error_message_format[0].format, "dummy") != 0) {
		DBG_panic(("runtime error message error.\n"));
	}
	if (strcmp(dvm_error_message_format[RUNTIME_ERROR_COUNT_PLUS_1].format, "dummy") != 0) {
		DBG_panic(("runtime error message error."
				"RUNTIME_ERROR_COUNT_PLUS_1..%d\n", RUNTIME_ERROR_COUNT_PLUS_1));
	}
}


void dvm_error_i(DVM_Executable *executable, Function *function, int pc, RuntimeError id, ...) {
	va_list ap;
	self_check();
	va_start(ap, id);
	error_v(executable, function, pc, dvm_error_message_format, id, ap);
	va_end(ap);
	exit(1);
}

void dvm_error_n(DVM_VirtualMachine *dvm, DVM_ErrorDefinition *error_definition, RuntimeError id, ...) {
	va_list ap;
	self_check();
	va_start(ap, id);
	error_v(dvm->current_executable->executable, dvm->current_function, dvm->pc, error_definition, id, ap);
	va_end(ap);
	exit(1);
}

int dvm_conv_pc_to_line_number(DVM_Executable *executable, Function *function, int pc) {
	int line_number_size;
	DVM_LineNumber *line_number;

	int ret;

	if (function) {
		line_number = executable->function[function->u.diksam_function.index].line_number;
		line_number_size = executable->function[function->u.diksam_function.index].line_number_size;
	} else {
		line_number = executable->line_number;
		line_number_size = executable->line_number_size;
	}

	for (int i = 0; i < line_number_size; i++) {
		if (pc >= line_number[i].start_pc && pc < line_number[i].start_pc + line_number[i].pc_count) {
			ret = line_number[i].line_number;

		}
	}

	return ret;
}