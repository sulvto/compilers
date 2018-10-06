//
// Created by sulvto on 18-6-15.
//
#include <DVM_code.h>
#include "DBG.h"
#include "share.h"

extern OpcodeInfo dvm_opcode_info[];


static void dump_type(DVM_Executable *executable, DVM_TypeSpecifier *type) ;

int dvm_dump_instruction(FILE *fp, DVM_Byte *code, int index) {
    OpcodeInfo *info = &dvm_opcode_info[code[index]];
    fprintf(fp, "%4d %s ", index, info->mnemonic);

    int value;

    for (int i = 0; info->parameter[i] != '\0'; i++) {
        switch (info->parameter[i]) {
            case 'b':
                value = code[index + 1];
                fprintf(fp, " %d", value);
                index++;
                break;
            case 's':
            case 'p':
                value = (code[index + 1] << 8) + code[index + 2];
                fprintf(fp, " %d", value);
                index += 2;
                break;
            default:
                DBG_assert(0, ("param..%s, i..%d", info->parameter, i));
        }
    }

    index++;

    return index;
}

static void dump_constant_pool(int count, DVM_ConstantPool *constant_pool) {
	printf("****************** constant pool section ******************\n");
	for (int i = 0; i < count; i++) {
		printf("%05d:", i);
		switch (constant_pool[i].tag) {
			case DVM_CONSTANT_INT:
				printf("int %d\n", constant_pool[i].u.c_int);
				break;
			case DVM_CONSTANT_DOUBLE:
				printf("double %f\n", constant_pool[i].u.c_double);
				break;
			case DVM_CONSTANT_STRING:
				printf("string ");
				dvm_print_wcs_ln(stdout, constant_pool[i].u.c_string);
				break;
			default:
				DBG_assert(0, ("constant_pool.tag..%d\n", constant_pool[i].tag));
		}
	}
	printf("\n");
}

static void dump_parameter_list(DVM_Executable *executable, int count, DVM_LocalVariable *parameter_list) {
	printf("(");
	for (int i = 0; i < count; i++) {
		dump_type(executable, parameter_list[i].type);
		printf(" %s", parameter_list[i].name);
		if (i < count - 1) {
			printf(", ");
		}
	}
	printf(")");

}

static void dump_type(DVM_Executable *executable, DVM_TypeSpecifier *type) {
	switch (type->basic_type) {
		case DVM_VOID_TYPE:
			printf("void ");
			break;
		case DVM_BOOLEAN_TYPE:
			printf("boolean ");
			break;
		case DVM_INT_TYPE:
			printf("int ");
			break;
		case DVM_DOUBLE_TYPE:
			printf("double ");
			break;
		case DVM_STRING_TYPE:
			printf("string ");
			break;
		case DVM_CLASS_TYPE:
			printf("<%s> ", executable->class_definition[type->class_index].name);
			break;
		case DVM_NULL_TYPE:
			printf("null ");
			break;
		case DVM_BASE_TYPE:
		default:
			DBG_assert(0, ("basic_type..%d\n", type->basic_type));
	}

	for (int i = 0; i < type->derive_count; i++) {
		switch (type->derive[i].tag) {
			case DVM_FUNCTION_DERIVE:
				dump_parameter_list(executable,
				                    type->derive[i].u.function_derive.parameter_count,
				                    type->derive[i].u.function_derive.parameter);
				break;
			case DVM_ARRAY_DERIVE:
				printf("[]");
				break;
			default:
				DBG_assert(0, ("derive.tag..%d\n", type->basic_type));
		}
	}
}

static void dump_global_variable(DVM_Executable *executable, int count, DVM_Variable *global_variable) {
	printf("****************** global variable section ******************\n");
	for (int i = 0; i < count; i++) {
		printf("%5d:", i);
		dump_type(executable, global_variable[i].type);
		printf(" %s\n", global_variable[i].name);
	}
	printf("\n");
}

static void dump_types(DVM_Executable *executable, int count, DVM_TypeSpecifier *type) {
	printf("****************** type section ******************\n");
	for (int i = 0; i < count; i++) {
		printf("%5d:", i);
		dump_type(executable, &type[i]);
		printf("\n");
	}
	printf("\n");
}

static void dump_opcode(int size, DVM_Byte *code) {
	for (int index = 0; index < size;) {
		index = dvm_dump_instruction(stdout, code, index);
		printf("\n");
	}
}

static void dump_line_number(int size, DVM_LineNumber *line_number) {
	printf("***** line number *****\n");
	for (int i = 0; i < size; i++) {
		printf("%5d: from %5d size %5d\n",
		       line_number[i].line_number,
		       line_number[i].start_pc,
		       line_number[i].pc_count);
	}
}

static void dump_function(DVM_Executable *executable, int count, DVM_Function *function) {
	printf("****************** function section ******************\n");

	for (int i = 0; i < count; i++) {
		printf("***** [%d] %s *****\n", i, function[i].name);
		dump_type(executable, function[i].type);
		printf(" %s ", function[i].name);
		dump_parameter_list(executable, function[i].parameter_count, function[i].parameter);
		printf("\n");
		if (function[i].is_implemented) {
			if (function[i].code_block.code_size > 0) {
				dump_opcode(function[i].code_block.code_size, function[i].code_block.code);
				dump_line_number(function[i].code_block.line_number_size, function[i].code_block.line_number);
			}
		}
		printf("***** end of %s *****\n\n", function[i].name);
	}
	printf("\n");
}

void dvm_disassemble(DVM_Executable *executable) {
	dump_constant_pool(executable->constant_pool_count, executable->constant_pool);
	dump_global_variable(executable, executable->global_variable_count, executable->global_variable);
	dump_function(executable, executable->function_count, executable->function);
	dump_types(executable, executable->type_specifier_count, executable->type_specifier);
	printf("****************** toplevel ******************\n");
	dump_opcode(executable->code_size, executable->code);
	dump_line_number(executable->line_number_size, executable->line_number);
}

