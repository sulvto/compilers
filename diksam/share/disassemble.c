//
// Created by sulvto on 18-6-15.
//
#include <DVM_code.h>
#include "DBG.h"
#include "share.h"

extern OpcodeInfo dvm_opcode_info[];

static FILE *disassemble_fp;

// ############## dump instruction ##################

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
// ############## dump instruction ##################


// ############## disassemble ##################

static void dump_type(DVM_Executable *executable, DVM_TypeSpecifier *type) ;

static void dump_constant_pool(int count, DVM_ConstantPool *constant_pool) {
	fprintf(disassemble_fp, "****************** constant pool section ******************\n");
	for (int i = 0; i < count; i++) {
		fprintf(disassemble_fp, "%05d:", i);
		switch (constant_pool[i].tag) {
			case DVM_CONSTANT_INT:
				fprintf(disassemble_fp, "int %d\n", constant_pool[i].u.c_int);
				break;
			case DVM_CONSTANT_DOUBLE:
				fprintf(disassemble_fp, "double %f\n", constant_pool[i].u.c_double);
				break;
			case DVM_CONSTANT_STRING:
				fprintf(disassemble_fp, "string ");
				dvm_print_wcs_ln(disassemble_fp, constant_pool[i].u.c_string);
				break;
			default:
				DBG_assert(0, ("constant_pool.tag..%d\n", constant_pool[i].tag));
		}
	}
	fprintf(disassemble_fp, "\n");
}

static void dump_parameter_list(DVM_Executable *executable, int count, DVM_LocalVariable *parameter_list) {
	fprintf(disassemble_fp, "(");
	for (int i = 0; i < count; i++) {
		dump_type(executable, parameter_list[i].type);
		fprintf(disassemble_fp, " %s", parameter_list[i].name);
		if (i < count - 1) {
			fprintf(disassemble_fp, ", ");
		}
	}
	fprintf(disassemble_fp, ")");

}

static void dump_type(DVM_Executable *executable, DVM_TypeSpecifier *type) {
	switch (type->basic_type) {
		case DVM_VOID_TYPE:
			fprintf(disassemble_fp, "void ");
			break;
		case DVM_BOOLEAN_TYPE:
			fprintf(disassemble_fp, "boolean ");
			break;
		case DVM_INT_TYPE:
			fprintf(disassemble_fp, "int ");
			break;
		case DVM_DOUBLE_TYPE:
			fprintf(disassemble_fp, "double ");
			break;
		case DVM_STRING_TYPE:
			fprintf(disassemble_fp, "string ");
			break;
		case DVM_CLASS_TYPE:
			fprintf(disassemble_fp, "<%s> ", executable->class_definition[type->class_index].name);
			break;
		case DVM_NULL_TYPE:
			fprintf(disassemble_fp, "null ");
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
				fprintf(disassemble_fp, "[]");
				break;
			default:
				DBG_assert(0, ("derive.tag..%d\n", type->basic_type));
		}
	}
}

static void dump_global_variable(DVM_Executable *executable, int count, DVM_Variable *global_variable) {
	fprintf(disassemble_fp, "****************** global variable section ******************\n");
	for (int i = 0; i < count; i++) {
		fprintf(disassemble_fp, "%5d:", i);
		dump_type(executable, global_variable[i].type);
		fprintf(disassemble_fp, " %s\n", global_variable[i].name);
	}
	fprintf(disassemble_fp, "\n");
}

static void dump_types(DVM_Executable *executable, int count, DVM_TypeSpecifier *type) {
	fprintf(disassemble_fp, "****************** type section ******************\n");
	for (int i = 0; i < count; i++) {
		fprintf(disassemble_fp, "%5d:", i);
		dump_type(executable, &type[i]);
		fprintf(disassemble_fp, "\n");
	}
	fprintf(disassemble_fp, "\n");
}

static void dump_opcode(int size, DVM_Byte *code) {
	for (int index = 0; index < size;) {
		index = dvm_dump_instruction(disassemble_fp, code, index);
		fprintf(disassemble_fp, "\n");
	}
}

static void dump_line_number(int size, DVM_LineNumber *line_number) {
	fprintf(disassemble_fp, "***** line number *****\n");
	for (int i = 0; i < size; i++) {
		fprintf(disassemble_fp, "%5d: from %5d size %5d\n",
		       line_number[i].line_number,
		       line_number[i].start_pc,
		       line_number[i].pc_count);
	}
}

static void dump_function(DVM_Executable *executable, int count, DVM_Function *function) {
	fprintf(disassemble_fp, "****************** function section ******************\n");

	for (int i = 0; i < count; i++) {
		fprintf(disassemble_fp, "***** [%d] %s *****\n", i, function[i].name);
		dump_type(executable, function[i].type);
		fprintf(disassemble_fp, " %s ", function[i].name);
		dump_parameter_list(executable, function[i].parameter_count, function[i].parameter);
		fprintf(disassemble_fp, "\n");
		if (function[i].is_implemented) {
			if (function[i].code_block.code_size > 0) {
				dump_opcode(function[i].code_block.code_size, function[i].code_block.code);
				dump_line_number(function[i].code_block.line_number_size, function[i].code_block.line_number);
			}
		}
		fprintf(disassemble_fp, "***** end of %s *****\n\n", function[i].name);
	}
	fprintf(disassemble_fp, "\n");
}

void dvm_disassemble(DVM_Executable *executable) {
    disassemble_fp = fopen("./disassemble.txt", "w");
    
	dump_constant_pool(executable->constant_pool_count, executable->constant_pool);
	dump_global_variable(executable, executable->global_variable_count, executable->global_variable);
	dump_function(executable, executable->function_count, executable->function);
	dump_types(executable, executable->type_specifier_count, executable->type_specifier);
	fprintf(disassemble_fp, "****************** toplevel ******************\n");
	dump_opcode(executable->top_level.code_size, executable->top_level.code);
	dump_line_number(executable->top_level.line_number_size, executable->top_level.line_number);

    fclose(disassemble_fp);
}
// ############## disassemble ##################
