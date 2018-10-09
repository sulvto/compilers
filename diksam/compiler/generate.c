//
// Created by sulvto on 18-6-6.
//

#include <stdio.h>
#include <stdarg.h>
#include <DVM_code.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "diksamc.h"

extern OpcodeInfo dvm_opcode_info[];

#define OPCODE_ALLOC_SIZE       (256)
#define LABEL_TABLE_ALLOC_SIZE  (256)

typedef struct {
	int label_address;
} LabelTable;

typedef struct {
    int             size;
    int             alloc_size;
    DVM_Byte        *code;
    int             label_table_size;
    int             label_table_alloc_size;
    LabelTable      *label_table;
    int             line_number_size;
    DVM_LineNumber  *line_number;
} OpcodeBuf;


static void generate_statement_list(DVM_Executable *executable,
                                    Block *current_block,
                                    StatementList *statement_list,
                                    OpcodeBuf *opcode_buf);


static void generate_expression(DVM_Executable *executable,
                                Block *current_block, Expression *expression,
                                OpcodeBuf *opcode_buf);

static DVM_TypeSpecifier *copy_type_specifier(TypeSpecifier *src);


static DVM_LocalVariable *copy_parameter_list(ParameterList *src,
				int *parameter_count_p)  ;


static void copy_type_specifier_no_alloc(TypeSpecifier *src, DVM_TypeSpecifier *dest) {
	dest->basic_type = src->basic_type;
	if (src->basic_type == DVM_CLASS_TYPE) {
		dest->class_index = src->u.class_ref.class_index;
	} else {
		dest->class_index = -1;
	}

	TypeDerive *derive;
	int derive_count = 0;
	for (derive = src->derive; derive; derive = derive->next) {
		derive_count++;
	}

	dest->derive_count = derive_count;
	dest->derive = MEM_malloc(sizeof(DVM_TypeDerive) * derive_count);

	int param_count;
	int i = 0;
	for (i = 0, derive = src->derive; derive; derive = derive->next, i++) {
		switch (derive->tag) {
			case FUNCTION_DERIVE:
				dest->derive[i].tag = DVM_FUNCTION_DERIVE;
				dest->derive[i].u
						.function_derive.parameter = copy_parameter_list(derive->u.function_derive.parameter_list,
				                                                         &param_count);
				dest->derive[i].u.function_derive.parameter_count = param_count;
				break;
			case ARRAY_DERIVE:
				dest->derive[i].tag = DVM_ARRAY_DERIVE;
				break;
			default:
				DBG_assert(0, ("derive->tag..%d\n", derive->tag));
		}
	}
}

DVM_TypeSpecifier *dkc_copy_type_specifier(TypeSpecifier *src) {
	DVM_TypeSpecifier *dest = MEM_malloc(sizeof(DVM_TypeSpecifier));
	copy_type_specifier_no_alloc(src, dest);
	return dest;
}

static DVM_Executable *alloc_executable(PackageName *package_name) {
	DVM_Executable *executable = MEM_malloc(sizeof(DVM_Executable));
	executable->package_name = dkc_package_name_to_string(package_name);
	executable->is_required = DVM_FALSE;
	executable->constant_pool_count = 0;
	executable->constant_pool = NULL;
	executable->global_variable_count = 0;
	executable->global_variable = NULL;
	executable->function_count = 0;
	executable->function = NULL;
	executable->type_specifier_count = 0;
	executable->type_specifier = NULL;
	executable->code_size = 0;
	executable->code = NULL;

	return executable;
}

static DVM_LocalVariable *copy_local_variables(FunctionDefinition *function_def,
                                               int parameter_count) {
	int local_variable_count = function_def->local_variable_count - parameter_count;

	DVM_LocalVariable *dest = MEM_malloc(sizeof(DVM_LocalVariable) * local_variable_count);

	for (int i = 0; i < local_variable_count; i++) {
		dest[i].name =
				MEM_strdup(function_def->local_variable[i + parameter_count]->name);
		dest[i].type =
				dkc_copy_type_specifier(function_def->local_variable[i + parameter_count]->type);
	}

	return dest;
}

static DVM_LocalVariable *copy_parameter_list(ParameterList *src,
				int *parameter_count_p)  {
	int parameter_count = 0;

	for (ParameterList *parameter = src; parameter; parameter = parameter->next) {
		parameter_count++;
	}

	*parameter_count_p = parameter_count;

	DVM_LocalVariable *dest =
			MEM_malloc(sizeof(DVM_LocalVariable) * parameter_count);

	int i;
	ParameterList *parameter;
	for (parameter = src, i = 0; parameter; parameter = parameter->next, i++) {
		dest[i].name = MEM_strdup(parameter->name);
		dest[i].type = copy_type_specifier(parameter->type);
	}

	return dest;
}

static DVM_TypeSpecifier *copy_type_specifier(TypeSpecifier *src) {
	DVM_TypeSpecifier *dest = MEM_malloc(sizeof(DVM_TypeSpecifier));
	int derive_count = 0;
	TypeDerive *derive;
	int parameter_count;
	int i;

	dest->basic_type = src->basic_type;

	for (derive = src->derive; derive; derive = derive->next) {
		derive_count++;
	}

	dest->derive_count = derive_count;
	dest->derive = MEM_malloc(sizeof(DVM_TypeDerive) * derive_count);
	for (i = 0, derive = src->derive; derive; derive = derive->next, i++) {
		switch (derive->tag) {
			case FUNCTION_DERIVE:
				dest->derive[i].tag = DVM_FUNCTION_DERIVE;
				dest->derive[i].u.function_derive.parameter = copy_parameter_list(derive->u.function_derive.parameter_list, &parameter_count);
				dest->derive[i].u.function_derive.parameter_count = parameter_count;
				break;
            case ARRAY_DERIVE:
                dest->derive[i].tag  =DVM_ARRAY_DERIVE;
                // TODO
                break;
			default:
				DBG_assert(0, ("derive->tag..%d\n", derive->tag));
		}
	}

	return dest;
}

static void add_global_variable(DKC_Compiler *compiler,
				DVM_Executable *executable) {
	DeclarationList *declaration;
	int variable_count = 0;
	for (declaration = compiler->declaration_list; declaration; declaration = declaration->next) {
		variable_count++;
	}
	executable->global_variable_count = variable_count;
	executable->global_variable = MEM_malloc(sizeof(DVM_Variable) * variable_count);

	int i;
	for (declaration = compiler->declaration_list, i = 0; declaration; declaration = declaration->next, i++) {
		executable->global_variable[i].name = MEM_strdup(declaration->declaration->name);
		executable->global_variable[i].type = dkc_copy_type_specifier(declaration->declaration->type);
	}
}

static void copy_function(FunctionDefinition *src, DVM_Function *dest) {
	dest->type = copy_type_specifier(src->type);
	dest->name = MEM_strdup(src->name);
	dest->parameter = copy_parameter_list(src->parameter,
                                         &dest->parameter_count);

    if (src->block) {
        dest->local_variable =
				copy_local_variables(src, dest->parameter_count);
        dest->local_variable_count =
				src->local_variable_count - dest->parameter_count;
    } else {
        dest->local_variable = NULL;
        dest->local_variable_count = 0;
    }
}

static void add_line_number(OpcodeBuf *opcode_buf, int line_number, int start_pc) {
	if (opcode_buf->line_number == NULL ||
	    (opcode_buf->line_number[opcode_buf->line_number_size - 1].line_number != line_number)) {
		opcode_buf->line_number = MEM_realloc(opcode_buf->line_number,
		                                      sizeof(DVM_LineNumber) * (opcode_buf->line_number_size + 1));
		opcode_buf->line_number[opcode_buf->line_number_size].line_number = line_number;
		opcode_buf->line_number[opcode_buf->line_number_size].start_pc = start_pc;
		opcode_buf->line_number[opcode_buf->line_number_size].pc_count = opcode_buf->size - start_pc;
		opcode_buf->line_number_size++;
	} else {
		opcode_buf->line_number[opcode_buf->line_number_size - 1].pc_count += opcode_buf->size - start_pc;
	}
}

static void generate_code(OpcodeBuf *opcode_buf,
				int line_number, DVM_Opcode code, ...) {
	va_list ap;
	char *parameter;
	int parameter_count;
	int start_pc;

	va_start(ap, code);

	parameter = dvm_opcode_info[(int) code].parameter;
	parameter_count = strlen(parameter);
	if (opcode_buf->alloc_size < opcode_buf->size + 1 + (parameter_count * 2)) {
		opcode_buf->code =
				MEM_realloc(opcode_buf->code,
								opcode_buf->alloc_size + OPCODE_ALLOC_SIZE);
		opcode_buf->alloc_size += OPCODE_ALLOC_SIZE;
	}

	start_pc = opcode_buf->size;
	opcode_buf->code[opcode_buf->size++] = code;
	for (int i = 0; parameter[i] != '\0'; i++) {
		unsigned int value = va_arg(ap, int);
		switch (parameter[i]) {
			case 'b':	// byte
				opcode_buf->code[opcode_buf->size++] = (DVM_Byte) value;
				break;
			case 's':	// short
				opcode_buf->code[opcode_buf->size] = (DVM_Byte)(value >> 8);
				opcode_buf->code[opcode_buf->size + 1] = (DVM_Byte)(value & 0xff);
				opcode_buf->size += 2;
				break;
			case 'p':	// constant pool index
				opcode_buf->code[opcode_buf->size] = (DVM_Byte)(value >> 8);
                opcode_buf->code[opcode_buf->size + 1] = (DVM_Byte)(value & 0xff);
                opcode_buf->size += 2;
                break;
			default :
				DBG_assert(0, ("parameter,,%s, i..%d\n", parameter, i));
		}
	}

	add_line_number(opcode_buf, line_number, start_pc);

	va_end(ap);
}

static void generate_boolean_expression(DVM_Executable *executable,
				Expression *expression, OpcodeBuf *opcode_buf) {
	if (expression->u.boolean_value) {
		generate_code(opcode_buf, expression->line_number, DVM_PUSH_INT_1BYTE, 1);
	} else {
		generate_code(opcode_buf, expression->line_number, DVM_PUSH_INT_1BYTE, 0);
	}
}

/**
 *
 *
 * @return insert index
 */
static int add_constant_pool(DVM_Executable *executable,
				DVM_ConstantPool *constant_pool) {
	executable->constant_pool =
			MEM_realloc(executable->constant_pool,
						sizeof(DVM_ConstantPool)
						* (executable->constant_pool_count + 1));
	executable->constant_pool[executable->constant_pool_count] = *constant_pool;
	int ret = executable->constant_pool_count;
	executable->constant_pool_count++;

	return ret;
}


static void generate_int_expression(DVM_Executable *executable,
				Expression *expression, OpcodeBuf *opcode_buf) {
	DVM_ConstantPool constant_pool;
	int constant_pool_index;

	if (expression->u.int_value >= 0 && expression->u.int_value < 256) {
		generate_code(opcode_buf, expression->line_number, DVM_PUSH_INT_1BYTE, expression->u.int_value);
	} else if (expression->u.int_value >= 0 && expression->u.int_value < 65536) {
		generate_code(opcode_buf, expression->line_number, DVM_PUSH_INT_2BYTE, expression->u.int_value);
	} else {
		constant_pool.tag = DVM_CONSTANT_INT;
		constant_pool.u.c_int = expression->u.int_value;
		constant_pool_index = add_constant_pool(executable, &constant_pool);

		generate_code(opcode_buf, expression->line_number, DVM_PUSH_INT, constant_pool_index);
	}
}

static void generate_double_expression(DVM_Executable *executable, Expression *expression, OpcodeBuf *opcode_buf) {
	if (expression->u.double_value == 0.0)	 {
		generate_code(opcode_buf, expression->line_number, DVM_PUSH_DOUBLE_0);
	} else if (expression->u.double_value == 1.0){
		generate_code(opcode_buf, expression->line_number, DVM_PUSH_DOUBLE_1);
	} else {
		DVM_ConstantPool constant_pool;
		constant_pool.tag = DVM_CONSTANT_DOUBLE;
		constant_pool.u.c_double = expression->u.double_value;
		int constant_pool_index = add_constant_pool(executable, &constant_pool);

		generate_code(opcode_buf, expression->line_number, DVM_PUSH_DOUBLE, constant_pool_index);
	}
}

static void generate_string_expression(DVM_Executable *executable, Expression *expression, OpcodeBuf *opcode_buf) {
	DVM_ConstantPool constant_pool;
	constant_pool.tag = DVM_CONSTANT_STRING;
	constant_pool.u.c_string = expression->u.string_value;
	int constant_pool_index = add_constant_pool(executable, &constant_pool);

	generate_code(opcode_buf, expression->line_number, DVM_PUSH_STRING, constant_pool_index);
}

static int get_opcode_type_offset(TypeSpecifier *type) {
	if (type->derive != NULL) {
		DBG_assert(type->derive->tag == ARRAY_DERIVE, ("type->derive->tag..%d", type->derive->tag));
		return 2;
	}
	switch (type->basic_type) {
		case DVM_VOID_TYPE:
			DBG_assert(0, ("basic_type is void"));
			break;
		case 	DVM_BOOLEAN_TYPE:
		case 	DVM_INT_TYPE:
			return 0;
		case 	DVM_DOUBLE_TYPE:
			return 1;
		case 	DVM_STRING_TYPE:
		case 	DVM_CLASS_TYPE:
			return 2;
		case 	DVM_NULL_TYPE:
		case 	DVM_BASE_TYPE:
		default:
			DBG_assert(0, ("basic_type..%d", type->basic_type));
	}

	return 0;
}

static void generate_identifier(Declaration *declaration, OpcodeBuf *opcode_buf, int line_number) {
	if (declaration->is_local) {
		generate_code(opcode_buf, line_number, DVM_PUSH_STACK_INT + get_opcode_type_offset(declaration->type),
		              declaration->variable_index);
	} else {
		generate_code(opcode_buf, line_number, DVM_PUSH_STATIC_INT + get_opcode_type_offset(declaration->type),
		              declaration->variable_index);
	}
}

static void generate_identifier_expression(DVM_Executable *executable, Block *current_block, Expression *expression, OpcodeBuf *opcode_buf) {
	switch (expression->u.identifier.kind){
		case FUNCTION_IDENTIFIER:
			generate_code(opcode_buf, expression->line_number, DVM_PUSH_FUNCTION, expression->u.identifier.u.function.function_index);
			break;
		case VARIABLE_IDENTIFIER:
			generate_identifier(expression->u.identifier.u.declaration, opcode_buf, executable->line_number);
			break;
		default:
			DBG_panic(("bad default. kind..%d", expression->u.identifier.kind));
	}
}

static void generate_pop_to_identifier(Declaration *declaration, int line_number, OpcodeBuf *opcode_buf) {
	if (declaration->is_local) {
		generate_code(opcode_buf,
					line_number,
					DVM_POP_STACK_INT + get_opcode_type_offset(declaration->type),
					declaration->variable_index);
	} else {
		generate_code(opcode_buf,
					line_number,
					DVM_POP_STATIC_INT + get_opcode_type_offset(declaration->type),
					declaration->variable_index);
	}
}

static void generate_pop_to_member(DVM_Executable *executable, Block *block, 
            Expression *expression, OpcodeBuf *opcode_buf) {
    MemberDeclaration *member = expression->u.member_expression.declaration;
    if (member->kind == METHOD_MEMBER) {
        dkc_compile_error(expression->line_number, ASSIGN_TO_METHOD_ERR, 
        STRING_MESSAGE_ARGUMENT, "member_name", member->u.method.function_definition->name, 
        MESSAGE_ARGUMENT_END);
    }
    generate_expression(executable, block, expression->u.member_expression.expression, opcode_buf);
    generate_code(opcode_buf,
                expression->line_number,
                DVM_POP_FIELD_INT + get_opcode_type_offset(member->u.field.type), member->u.field.field_index);
}

static void generate_pop_to_lvalue(DVM_Executable *executable, Block *block, 
            Expression *expression, OpcodeBuf *opcode_buf) {
	if (expression->kind == IDENTIFIER_EXPRESSION) {
        generate_pop_to_identifier(expression->u.identifier.u.declaration, 
                expression->line_number, opcode_buf);
	} else if (expression->kind == INDEX_EXPRESSION) {
        generate_expression(executable, block, expression->u.index_expression.array, opcode_buf);
        generate_expression(executable, block, expression->u.index_expression.index, opcode_buf);
		generate_code(opcode_buf,
					expression->line_number,
					DVM_POP_ARRAY_INT + get_opcode_type_offset(expression->type));
	} else {
        DBG_assert(expression->kind == MEMBER_EXPRESSION, ("expression->kind..%d", expression->kind));
        generate_pop_to_member(executable, block, expression, opcode_buf);
    }
}

static void generate_assign_expression(DVM_Executable *executable,
				Block *current_block, Expression *expression,
				OpcodeBuf *opcode_buf, DVM_Boolean is_toplevel) {
	if (expression->u.assign_expression.operator != NORMAL_ASSIGN) {
		generate_identifier_expression(executable,
		                               current_block, expression->u.assign_expression.left,
						opcode_buf);
	}
	generate_expression(executable, current_block,
					expression->u.assign_expression.operand, opcode_buf);
	switch (expression->u.assign_expression.operator) {
		case NORMAL_ASSIGN:
			break;
		case ADD_ASSIGN:
			generate_code(opcode_buf,
						expression->line_number,
						DVM_ADD_INT + get_opcode_type_offset(expression->type));
			break;
		case SUB_ASSIGN:
			generate_code(opcode_buf,
						expression->line_number,
						DVM_SUB_INT + get_opcode_type_offset(expression->type));
			break;
		case MUL_ASSIGN:
			generate_code(opcode_buf,
						expression->line_number,
						DVM_MUL_INT + get_opcode_type_offset(expression->type));
			break;
		case DIV_ASSIGN:
			generate_code(opcode_buf,
						expression->line_number,
						DVM_DIV_INT + get_opcode_type_offset(expression->type));
			break;
		case MOD_ASSIGN:
			generate_code(opcode_buf,
						expression->line_number,
						DVM_MOD_INT + get_opcode_type_offset(expression->type));
			break;
		default:
			DBG_assert(0, ("operator..%d\n", expression->u.assign_expression.operator));
	}

	if (!is_toplevel) {
		generate_code(opcode_buf, expression->line_number, DVM_DUPLICATE);
	}
	generate_pop_to_lvalue(executable, current_block, expression->u.assign_expression.left, opcode_buf);
}

static int get_binary_expression_offset(Expression *left, Expression *right, DVM_Opcode code) {
	int offset;
	if ((left->kind == NULL_EXPRESSION && right->kind != NULL_EXPRESSION)
	    ||(left->kind != NULL_EXPRESSION && right->kind == NULL_EXPRESSION)) {
		offset = 2;
	} else if ((code == DVM_EQ_INT || code == DVM_NE_INT) && dkc_is_string(left->type)) {
		offset = 3;
	} else {
		offset = get_opcode_type_offset(left->type);
	}

	return offset;
}

static void generate_binary_expression(DVM_Executable *executable, Block *current_block, Expression *expression,
                                       DVM_Opcode code, OpcodeBuf *opcode_buf) {

	generate_expression(executable, current_block,
	                    expression->u.binary_expression.left, opcode_buf);
	generate_expression(executable, current_block,
	                    expression->u.binary_expression.right, opcode_buf);

	int offset = get_binary_expression_offset(expression->u.binary_expression.left,
	                                          expression->u.binary_expression.right, code);


	generate_code(opcode_buf, expression->line_number, code + offset);
}

static int get_label(OpcodeBuf *opcode_buf) {
	if (opcode_buf->label_table_alloc_size < opcode_buf->label_table_size + 1) {
		opcode_buf->label_table =
				MEM_realloc(opcode_buf->label_table,
								(opcode_buf->label_table_alloc_size + LABEL_TABLE_ALLOC_SIZE) * sizeof(LabelTable));
		opcode_buf->label_table_alloc_size += LABEL_TABLE_ALLOC_SIZE;
	}

	return opcode_buf->label_table_size++;
}

static void set_label(OpcodeBuf *opcode_buf, int label) {
	opcode_buf->label_table[label].label_address = opcode_buf->size;
}

static void generate_logical_add_expression(DVM_Executable *executable,
				Block *current_block, Expression *expression,
				OpcodeBuf *opcode_buf) {
	int false_label = get_label(opcode_buf);
	generate_expression(executable, current_block,
					expression->u.binary_expression.left, opcode_buf);
	generate_code(opcode_buf, expression->line_number, DVM_DUPLICATE);
	generate_code(opcode_buf, expression->line_number, DVM_JUMP_IF_TRUE,
					false_label);
	generate_expression(executable, current_block,
					expression->u.binary_expression.right, opcode_buf);
	generate_code(opcode_buf, expression->line_number, DVM_LOGICAL_AND);

	set_label(opcode_buf, false_label);
}

static void generate_logical_or_expression(DVM_Executable *executable,
				Block *current_block, Expression *expression,
				OpcodeBuf *opcode_buf) {
	int true_label = get_label(opcode_buf);
	generate_expression(executable, current_block,
					expression->u.binary_expression.left, opcode_buf);
	generate_code(opcode_buf, expression->line_number, DVM_DUPLICATE);
	generate_code(opcode_buf, expression->line_number, DVM_JUMP_IF_TRUE, true_label);
	generate_expression(executable, current_block,
					expression->u.binary_expression.right, opcode_buf);
	generate_code(opcode_buf, expression->line_number, DVM_LOGICAL_OR);

	set_label(opcode_buf, true_label);
}

static void generate_inc_dec_expression(DVM_Executable *executable,
				Block *current_block, Expression *expression,
				ExpressionKind kind, OpcodeBuf *opcode_buf,
				DVM_Boolean is_toplevel) {
	generate_expression(executable, current_block,
					expression->u.inc_dec.operand, opcode_buf);

	if (kind == INCREMENT_EXPRESSION) {
		generate_code(opcode_buf, expression->line_number, DVM_INCREMENT);
	} else {
		generate_code(opcode_buf, expression->line_number, DVM_DECREMENT);
	}

	if (!is_toplevel) {
		generate_code(opcode_buf, expression->line_number, DVM_DUPLICATE);
	}

	generate_pop_to_lvalue(executable, current_block, expression->u.inc_dec.operand, opcode_buf);
}

static void generate_cast_expression(DVM_Executable *executable,
				Block *current_block, Expression *expression,
				OpcodeBuf *opcode_buf) {
	generate_expression(executable, current_block,
					expression->u.cast.operand, opcode_buf);

	switch (expression->u.cast.type) {
		case INT_TO_DOUBLE_CAST:
			generate_code(opcode_buf, expression->line_number,
							DVM_CAST_INT_TO_DOUBLE);
			break;
		case DOUBLE_TO_INT_CAST:
			generate_code(opcode_buf, expression->line_number,
							DVM_CAST_DOUBLE_TO_INT);
			break;
		case BOOLEAN_TO_STRING_CAST:
			generate_code(opcode_buf, expression->line_number,
							DVM_CAST_BOOLEAN_TO_STRING);
			break;
		case INT_TO_STRING_CAST:
        	generate_code(opcode_buf, expression->line_number,
							DVM_CAST_INT_TO_STRING);
        	break;
		case DOUBLE_TO_STRING_CAST:
        	generate_code(opcode_buf, expression->line_number,
							DVM_CAST_DOUBLE_TO_STRING);
        	break;
		default:
			DBG_assert(0, ("expr->u.cast.type..%d", expression->u.cast.type));
	}
}

static void generate_push_argument(DVM_Executable *executable, Block *current_block,
                                   ArgumentList *argument, OpcodeBuf *opcode_buf) {
	for (ArgumentList *pos = argument; pos; pos = pos->next) {
		generate_expression(executable, current_block, pos->expression, opcode_buf);
	}
}


static int get_method_index(MemberExpression *member) {
	int method_index;
	if (dkc_is_array(member->expression->type) || dkc_is_string(member->expression->type)) {
		method_index = member->method_index;
	} else {
		DBG_assert(member->declaration->kind == METHOD_MEMBER,
		           ("member->declaration->kind..%d", member->declaration->kind));
		method_index = member->declaration->u.method.method_index;
	}

	return method_index;
}

static void generate_method_call_expression(DVM_Executable *executable, Block *current_block,
                                            Expression *expression, OpcodeBuf *opcode_buf) {
	MemberExpression *member = &expression->u.function_call_expression.function->u.member_expression;
	int method_index = get_method_index(member);
	generate_push_argument(executable, current_block, expression->u.function_call_expression.argument, opcode_buf);
	generate_expression(executable, current_block,
	                    expression->u.function_call_expression.function->u.member_expression.expression, opcode_buf);
	generate_code(opcode_buf, expression->line_number, DVM_PUSH_METHOD, method_index);
	generate_code(opcode_buf, expression->line_number, DVM_INVOKE);
}

static void generate_function_call_expression(DVM_Executable *executable, Block *current_block,
                                              Expression *expression, OpcodeBuf *opcode_buf) {
	FunctionCallExpression *function_call_expression = &expression->u.function_call_expression;
	if (function_call_expression->function->kind == MEMBER_EXPRESSION &&
	    ((dkc_is_array(function_call_expression->function->u.member_expression.expression->type) ||
	      dkc_is_string(function_call_expression->function->u.member_expression.expression->type)) ||
	     (function_call_expression->function->u.member_expression.declaration->kind == METHOD_MEMBER))) {
		generate_method_call_expression(executable, current_block, expression, opcode_buf);
	} else {
		generate_push_argument(executable, current_block, function_call_expression->argument, opcode_buf);
		generate_expression(executable, current_block, function_call_expression->function, opcode_buf);
        if (dkc_is_delegate(function_call_expression->function->type)) {
		    generate_code(opcode_buf, expression->line_number, DVM_INVOKE_DELEGATE);
        } else {
		generate_code(opcode_buf, expression->line_number, DVM_INVOKE);
	}
}
}

static void generate_member_expression(DVM_Executable *executable, Block *current_block,
                                       Expression *expression, OpcodeBuf *opcode_buf) {
	MemberDeclaration *member = expression->u.member_expression.declaration;
	if (member->kind == FIELD_MEMBER) {
		generate_expression(executable, current_block, expression->u.member_expression.expression, opcode_buf);
		generate_code(opcode_buf, expression->line_number,
		              DVM_PUSH_FIELD_INT + get_opcode_type_offset(expression->type),
		              member->u.field.field_index);
	} else {
		DBG_assert(member->kind == METHOD_MEMBER, ("member->kind..%d", member->kind));
		dkc_compile_error(expression->line_number, METHOD_IS_NOT_CALLED_ERR,
		                  STRING_MESSAGE_ARGUMENT, "member_name", member->u.method.function_definition->name,
		                  MESSAGE_ARGUMENT_END);
	}
}

static void generate_null_expression(DVM_Executable *executable, Block *current_block,
                                     Expression *expression, OpcodeBuf *opcode_buf) {
	generate_code(opcode_buf, expression->line_number, DVM_PUSH_NULL);
}

static FunctionDefinition *get_current_function(Block *block) {
	Block *pos;
	for (pos = block; pos->type != FUNCTION_BLOCK; pos = pos->outer_block);

	return pos->parent.function.function;
}

static int count_parameter(ParameterList *src) {
	ParameterList *parameter;
	int parameter_count = 0;
	for (parameter = src; parameter; parameter = parameter->next) {
		parameter_count++;
	}

	return parameter_count;
}

static void generate_this_expression(DVM_Executable *executable, Block *current_block,
                                     Expression *expression, OpcodeBuf *opcode_buf) {
	FunctionDefinition *function_definition = get_current_function(current_block);
	int parameter_count = count_parameter(function_definition->parameter);
	generate_code(opcode_buf, expression->line_number, DVM_PUSH_STACK_OBJECT, parameter_count);
}

static void generate_super_expression(DVM_Executable *executable, Block *current_block,
                                      Expression *expression, OpcodeBuf *opcode_buf) {
	FunctionDefinition *function_definition = get_current_function(current_block);
	int parameter_count = count_parameter(function_definition->parameter);
	generate_code(opcode_buf, expression->line_number, DVM_PUSH_STACK_OBJECT, parameter_count);
	generate_code(opcode_buf, expression->line_number, DVM_SUPER);
}

static void generate_new_expression(DVM_Executable *executable, Block *current_block,
                                      Expression *expression, OpcodeBuf *opcode_buf) {
	int parameter_count = count_parameter(
			expression->u.new_expression.method_declaration->u.method.function_definition->parameter);
	generate_code(opcode_buf, expression->line_number, DVM_NEW, expression->u.new_expression.class_index);
	generate_push_argument(executable, current_block, expression->u.new_expression.argument, opcode_buf);
	generate_code(opcode_buf, expression->line_number, DVM_DUPLICATE_OFFSET, parameter_count);
	generate_code(opcode_buf, expression->line_number, DVM_PUSH_METHOD,
	              expression->u.new_expression.method_declaration->u.method.method_index);
	generate_code(opcode_buf, expression->line_number, DVM_INVOKE);
	generate_code(opcode_buf, expression->line_number, DVM_POP);
}

static void generate_index_expression(DVM_Executable *executable, Block *current_block,
                                      Expression *expression, OpcodeBuf *opcode_buf) {
	generate_expression(executable, current_block, expression->u.index_expression.array, opcode_buf);
	generate_expression(executable, current_block, expression->u.index_expression.index, opcode_buf);

	generate_code(opcode_buf, expression->line_number, DVM_PUSH_ARRAY_INT + get_opcode_type_offset(expression->type));
}

static void generate_array_literal_expression(DVM_Executable *executable, Block *current_block,
                                              Expression *expression, OpcodeBuf *opcode_buf) {
	DBG_assert(expression->type->derive && expression->type->derive->tag == ARRAY_DERIVE,
	           ("array literal is not array."));
	int count = 0;
	for (ExpressionList *pos = expression->u.array_literal; pos; pos = pos->next) {
		generate_expression(executable, current_block, pos->expression, opcode_buf);
		count++;
	}

	DBG_assert(count>0,("empty array literal"));

	generate_code(opcode_buf, expression->line_number,
	              DVM_NEW_ARRAY_LITERAL_INT + get_opcode_type_offset(expression->u.array_literal->expression->type),
	              count);
}

static void generate_instanceof_expression(DVM_Executable *executable, Block *current_block,
                                           Expression *expression, OpcodeBuf *opcode_buf) {
	generate_expression(executable, current_block, expression->u.instanceof_expression.operand, opcode_buf);
	generate_code(opcode_buf, expression->line_number, DVM_INSTANCEOF,
	              expression->u.instanceof_expression.type->u.class_ref.class_index);
}

static void generate_down_cast_expression(DVM_Executable *executable, Block *current_block,
                                          Expression *expression, OpcodeBuf *opcode_buf) {
	generate_expression(executable, current_block, expression->u.down_cast.operand, opcode_buf);
	generate_code(opcode_buf, expression->line_number, DVM_DOWN_CAST,
	              expression->u.down_cast.type->u.class_ref.class_index);
}

static void generate_up_cast_expression(DVM_Executable *executable, Block *current_block,
                                        Expression *expression, OpcodeBuf *opcode_buf) {
	generate_expression(executable, current_block, expression->u.up_cast.operand, opcode_buf);
	generate_code(opcode_buf, expression->line_number, DVM_UP_CAST,
	              expression->u.up_cast.interface_index);
}


static int add_type_specifier(TypeSpecifier *src, DVM_Executable *executable) {
	executable->type_specifier = MEM_realloc(executable->type_specifier,
	                                         sizeof(DVM_TypeSpecifier) * (executable->type_specifier_count + 1));
	copy_type_specifier_no_alloc(src, &executable->type_specifier[executable->type_specifier_count]);
	return executable->type_specifier_count++;
}

static void generate_array_creation_expression(DVM_Executable *executable, Block *current_block,
                                               Expression *expression, OpcodeBuf *opcode_buf) {
	int index = add_type_specifier(expression->type, executable);
	DBG_assert(expression->type->derive->tag == ARRAY_DERIVE, ("expression->type->derive->tag..%d", expression->type->derive->tag));

	TypeSpecifier type;
	type.basic_type = expression->type->basic_type;
	type.derive = expression->type->derive;

	int dim_count = 0;
	for (ArrayDimension *pos = expression->u.array_creation.dimension; pos; pos = pos->next) {
		if (pos->expression == NULL) {
			continue;
		}
		generate_expression(executable, current_block, pos->expression, opcode_buf);
		dim_count++;
	}

	generate_code(opcode_buf, expression->line_number, DVM_NEW_ARRAY, dim_count, index);
}

static void generate_expression(DVM_Executable *executable,
				Block *current_block, Expression *expression,
				OpcodeBuf *opcode_buf) {
	switch (expression->kind) {
		case BOOLEAN_EXPRESSION:
			generate_boolean_expression(executable, expression, opcode_buf);
			break;
		case INT_EXPRESSION:
			generate_int_expression(executable, expression, opcode_buf);
			break;
		case DOUBLE_EXPRESSION:
			generate_double_expression(executable, expression, opcode_buf);
			break;
		case STRING_EXPRESSION:
			generate_string_expression(executable, expression, opcode_buf);
			break;
		case IDENTIFIER_EXPRESSION:
			generate_identifier_expression(executable, current_block,
			                               expression, opcode_buf);
			break;
		case COMMA_EXPRESSION:
			generate_expression(executable, current_block,
			                    expression->u.comma.left, opcode_buf);
			generate_expression(executable, current_block,
			                    expression->u.comma.right, opcode_buf);
			break;
		case ASSIGN_EXPRESSION:
			generate_assign_expression(executable, current_block,
			                           expression, opcode_buf, DVM_FALSE);
			break;
		case ADD_EXPRESSION:
			generate_binary_expression(executable, current_block,
			                           expression, DVM_ADD_INT, opcode_buf);
			break;
		case SUB_EXPRESSION:
			generate_binary_expression(executable, current_block,
			                           expression, DVM_SUB_INT, opcode_buf);
			break;
		case MUL_EXPRESSION:
			generate_binary_expression(executable, current_block,
			                           expression, DVM_MUL_INT, opcode_buf);
			break;
		case DIV_EXPRESSION:
			generate_binary_expression(executable, current_block,
			                           expression, DVM_DIV_INT, opcode_buf);
			break;
		case MOD_EXPRESSION:
			generate_binary_expression(executable, current_block,
			                           expression, DVM_MOD_INT, opcode_buf);
			break;
		case EQ_EXPRESSION:
			generate_binary_expression(executable, current_block,
			                           expression, DVM_EQ_INT, opcode_buf);
			break;
		case NE_EXPRESSION:
			generate_binary_expression(executable, current_block,
			                           expression, DVM_NE_INT, opcode_buf);
			break;
		case GT_EXPRESSION:
			generate_binary_expression(executable, current_block,
			                           expression, DVM_GT_INT, opcode_buf);
			break;
		case GE_EXPRESSION:
			generate_binary_expression(executable, current_block,
			                           expression, DVM_GE_INT, opcode_buf);
			break;
		case LT_EXPRESSION:
			generate_binary_expression(executable, current_block,
			                           expression, DVM_LT_INT, opcode_buf);
			break;
		case LE_EXPRESSION:
			generate_binary_expression(executable, current_block,
			                           expression, DVM_LE_INT, opcode_buf);
			break;
		case LOGICAL_AND_EXPRESSION:
			generate_logical_add_expression(executable, current_block,
			                                expression, opcode_buf);
			break;
		case LOGICAL_OR_EXPRESSION:
			generate_logical_or_expression(executable, current_block,
			                               expression, opcode_buf);
			break;
		case MINUS_EXPRESSION:
			generate_expression(executable, current_block,
			                    expression->u.minus_expression, opcode_buf);
			generate_code(opcode_buf, expression->line_number,
			              DVM_MINUS_INT
			              + get_opcode_type_offset(expression->type));
			break;
		case LOGICAL_NOT_EXPRESSION:
			generate_expression(executable, current_block,
			                    expression->u.logical_not, opcode_buf);
			generate_code(opcode_buf, expression->line_number, DVM_LOGICAL_NOT);
			break;
		case FUNCTION_CALL_EXPRESSION:
			generate_function_call_expression(executable, current_block,
			                                  expression, opcode_buf);
			break;
		case MEMBER_EXPRESSION:
			generate_member_expression(executable, current_block, expression, opcode_buf);
			break;
		case NULL_EXPRESSION:
			generate_null_expression(executable, current_block, expression, opcode_buf);
			break;
		case THIS_EXPRESSION:
			generate_this_expression(executable, current_block, expression, opcode_buf);
			break;
		case SUPER_EXPRESSION:
			generate_super_expression(executable, current_block, expression, opcode_buf);
			break;
		case NEW_EXPRESSION:
			generate_new_expression(executable, current_block, expression, opcode_buf);
			break;
		case ARRAY_LITERAL_EXPRESSION:
			generate_array_literal_expression(executable, current_block, expression, opcode_buf);
			break;
		case INDEX_EXPRESSION:
			generate_index_expression(executable, current_block, expression, opcode_buf);
			break;
		case INCREMENT_EXPRESSION:
		case DECREMENT_EXPRESSION:
			generate_inc_dec_expression(executable, current_block,
			                            expression, expression->kind, opcode_buf, DVM_FALSE);
			break;
		case INSTANCEOF_EXPRESSION:
			generate_instanceof_expression(executable, current_block, expression, opcode_buf);
			break;
		case DOWN_CAST_EXPRESSION:
			generate_down_cast_expression(executable, current_block, expression, opcode_buf);
			break;
		case CAST_EXPRESSION:
			generate_cast_expression(executable, current_block,
			                         expression, opcode_buf);
			break;
		case UP_CAST_EXPRESSION:
			generate_up_cast_expression(executable, current_block, expression, opcode_buf);
			break;
		case ARRAY_CREATION_EXPRESSION:
			generate_array_creation_expression(executable, current_block, expression, opcode_buf);
			break;
		case EXPRESSION_KIND_COUNT_PLUS_1:
		default:
			DBG_assert(0, ("expression->kind..%d\n", expression->kind));
	}
}

static void generate_expression_statement(DVM_Executable *executable,
				Block *current_block, Expression *expression, 
				OpcodeBuf *opcode_buf) {
	if (expression->kind == ASSIGN_EXPRESSION) {
		generate_assign_expression(executable, current_block, 
						expression, opcode_buf, DVM_TRUE);
	} else if (expression->kind == INCREMENT_EXPRESSION 
					||expression->kind == DECREMENT_EXPRESSION) {
		generate_inc_dec_expression(executable, current_block, 
						expression, expression->kind, opcode_buf, DVM_TRUE);
	} else {
		generate_expression(executable, current_block, expression, opcode_buf);
		generate_code(opcode_buf, expression->line_number, DVM_POP);
	}
}

static void generate_if_statement(DVM_Executable *executable, 
				Block *current_block, Statement *statement, 
				OpcodeBuf *opcode_buf) {
	int if_false_label;
	int end_label;
	IfStatement *if_s = &statement->u.if_s; 	
	if_false_label = get_label(opcode_buf);
	generate_code(opcode_buf, statement->line_number, 
					DVM_JUMP_IF_FALSE, if_false_label);
	generate_statement_list(executable, if_s->then_block,
					if_s->then_block->statement_list, opcode_buf);
	end_label = get_label(opcode_buf);
	generate_code(opcode_buf, statement->line_number, DVM_JUMP, end_label);
	set_label(opcode_buf, if_false_label);
	for (Elseif *elseif = if_s->elseif_list; elseif; elseif = elseif->next) {
		generate_expression(executable, current_block, elseif->condition, 
						opcode_buf);
		if_false_label = get_label(opcode_buf);
		generate_code(opcode_buf, statement->line_number,
						DVM_JUMP_IF_FALSE, if_false_label);
		generate_statement_list(executable, elseif->block, 
						elseif->block->statement_list, opcode_buf);
		generate_code(opcode_buf, statement->line_number, DVM_JUMP, end_label);
		set_label(opcode_buf, if_false_label);
	}

	if (if_s->else_block) {
		generate_statement_list(executable, if_s->else_block, 
						if_s->else_block->statement_list, opcode_buf);
	}

	set_label(opcode_buf, end_label);
}

static void generate_while_statement(DVM_Executable *executable, 
				Block *current_block, Statement *statement, 
				OpcodeBuf *opcode_buf) {
	int loop_label;
	WhileStatement *while_s = &statement->u.while_s;
	loop_label = get_label(opcode_buf);
	set_label(opcode_buf, loop_label);

	generate_expression(executable, current_block, while_s->condition, opcode_buf);

	while_s->block->parent.statement.break_label = get_label(opcode_buf);
	while_s->block->parent.statement.continue_label = get_label(opcode_buf);

	generate_code(opcode_buf, statement->line_number, 
					DVM_JUMP_IF_FALSE, while_s->block->parent.statement.break_label);
	generate_statement_list(executable, while_s->block, 
					while_s->block->statement_list, opcode_buf);
	set_label(opcode_buf, while_s->block->parent.statement.continue_label);
	generate_code(opcode_buf, statement->line_number, DVM_JUMP, loop_label);
	set_label(opcode_buf, while_s->block->parent.statement.break_label);
}

static void generate_for_statement(DVM_Executable *executable, 
				Block *current_block, Statement *statement, 
				OpcodeBuf *opcode_buf) {
	int loop_label;
	ForStatement *for_s = &statement->u.for_s;

	if (for_s->init) {
		generate_expression(executable, current_block, for_s->init, opcode_buf);
	}

	loop_label = get_label(opcode_buf);

	if (for_s->condition) {
		generate_expression(executable, current_block, for_s->condition, opcode_buf);
	}

	for_s->block->parent.statement.break_label = get_label(opcode_buf);
	for_s->block->parent.statement.continue_label = get_label(opcode_buf);

	if (for_s->condition) {
		generate_code(opcode_buf, statement->line_number, 
						DVM_JUMP_IF_FALSE, 
						for_s->block->parent.statement.break_label);
	}

	generate_statement_list(executable, for_s->block, 
					for_s->block->statement_list, opcode_buf);

	set_label(opcode_buf, for_s->block->parent.statement.continue_label);

	if (for_s->post) {
		generate_expression(executable, current_block, for_s->post, opcode_buf);
	}

	generate_code(opcode_buf, statement->line_number, DVM_JUMP, loop_label);

	set_label(opcode_buf, for_s->block->parent.statement.break_label);

}

static void generate_return_statement(DVM_Executable *executable, 
				Block *current_block, Statement *statement, 
				OpcodeBuf *opcode_buf) {
	DBG_assert(statement->u.return_s.return_value != NULL, 
					("return value is null."));
	generate_expression(executable, current_block, statement->u.return_s.return_value, opcode_buf);
	generate_code(opcode_buf, statement->line_number, DVM_RETURN);
}

static void generate_break_statement(DVM_Executable *executable, 
				Block *current_block, Statement *statement, 
				OpcodeBuf *opcode_buf) {
	BreakStatement *break_s = &statement->u.break_s;

	Block *block_p;

	for (block_p = current_block; block_p; block_p = block_p->outer_block) {
		if (block_p->type != WHILE_STATEMENT_BLOCK
			&& block_p->type != FOR_STATEMENT_BLOCK) {
			continue;
		}

		if (break_s->label == NULL) {
			break;
		}

		if (block_p->type == WHILE_STATEMENT_BLOCK) {
			if (block_p->parent.statement.statement->u.while_s.label == NULL) {
				continue;
			}

			if (strcmp(break_s->label, 
						block_p->parent.statement.statement->u.while_s.label) == 0) {
				break;
			}
		} else if (block_p->type == FOR_STATEMENT_BLOCK) {
			if (block_p->parent.statement.statement->u.for_s.label == NULL) {
            	continue;
            }
                                                                                                     
            if (strcmp(break_s->label, 
						block_p->parent.statement.statement->u.for_s.label) == 0) {
            	break;
            }
		}
	}

	if (block_p == NULL) {
		dkc_compile_error(statement->line_number, 
						LABEL_NOT_FOUND_ERR, 
						STRING_MESSAGE_ARGUMENT, "label", break_s->label, 
						MESSAGE_ARGUMENT_END);
	}

	generate_code(opcode_buf, statement->line_number, 
					DVM_JUMP, block_p->parent.statement.break_label);
}

static void generate_continue_statement(DVM_Executable *executable, 
				Block *current_block, Statement *statement, 
				OpcodeBuf *opcode_buf) {
	ContinueStatement *continue_s = &statement->u.continue_s;
	Block *block_p;

    for (block_p = current_block; block_p; block_p = block_p->outer_block) {
    	if (block_p->type != WHILE_STATEMENT_BLOCK
    		&& block_p->type != FOR_STATEMENT_BLOCK) {
    		continue;
    	}
                                                                                                      
    	if (continue_s->label == NULL) {
    		break;
    	}
                                                                                                      
    	if (block_p->type == WHILE_STATEMENT_BLOCK) {
    		if (block_p->parent.statement.statement->u.while_s.label == NULL) {
    			continue;
    		}
                                                                                                      
    		if (strcmp(continue_s->label, 
    					block_p->parent.statement.statement->u.while_s.label) == 0) {
    			break;
    		}
    	} else if (block_p->type == FOR_STATEMENT_BLOCK) {
    		if (block_p->parent.statement.statement->u.for_s.label == NULL) {
            	continue;
            }
                                                                                                     
            if (strcmp(continue_s->label, 
    					block_p->parent.statement.statement->u.for_s.label) == 0) {
            	break;
            }
    	}
    }
                                                                                                      
    if (block_p == NULL) {
    	dkc_compile_error(statement->line_number, 
    					LABEL_NOT_FOUND_ERR, 
    					STRING_MESSAGE_ARGUMENT, "label", continue_s->label, 
    					MESSAGE_ARGUMENT_END);
    }
                                                                                                      
    generate_code(opcode_buf, statement->line_number, 
    				DVM_JUMP, block_p->parent.statement.continue_label);
}

static void generate_initializer(DVM_Executable *executable, Block *current_block, 
				Statement *statement, OpcodeBuf *opcode_buf) {
	Declaration *declaration = statement->u.declaration_s;
	if (declaration->initializer == NULL) {
		return;
	}

	generate_expression(executable, current_block, 
					declaration->initializer, opcode_buf);
	generate_pop_to_identifier(declaration, statement->line_number, opcode_buf);
}

static void generate_statement_list(DVM_Executable *executable,
                                    Block *current_block,
                                    StatementList *statement_list,
                                    OpcodeBuf *opcode_buf) {
    for (StatementList *pos = statement_list; pos; pos = pos->next) {
        switch (pos->statement->type) {
            case EXPRESSION_STATEMENT:
				generate_expression_statement(executable,
								current_block, pos->statement->u.expression_s,
								opcode_buf);
				break;
            case IF_STATEMENT:
				generate_if_statement(executable,
								current_block, pos->statement, opcode_buf);
				break;
            case WHILE_STATEMENT:
				generate_while_statement(executable,
								current_block, pos->statement, opcode_buf);
                break;
            case FOR_STATEMENT:
				generate_for_statement(executable,
								current_block, pos->statement, opcode_buf);
                break;
            case FOREACH_STATEMENT:
                break;
            case RETURN_STATEMENT:
				generate_return_statement(executable, current_block,
								pos->statement, opcode_buf);
                break;
            case BREAK_STATEMENT:
				generate_break_statement(executable, current_block,
								pos->statement, opcode_buf);
                break;
            case CONTINUE_STATEMENT:
				generate_continue_statement(executable, current_block,
								pos->statement, opcode_buf);
                break;
            case TRY_STATEMENT:
				break;
            case THROW_STATEMENT:
				break;
            case DECLARATION_STATEMENT:
				generate_initializer(executable, current_block,
								pos->statement, opcode_buf);
                break;
            case STATEMENT_TYPE_COUNT_PLUS_1:
            default:
                DBG_assert(0, ("pos->statement->type..", pos->statement->type));
        }
    }
}

static void init_opcode_buf(OpcodeBuf *opcode_buf) {
    opcode_buf->size = 0;
    opcode_buf->alloc_size = 0;
    opcode_buf->code = 0;
    opcode_buf->label_table = NULL;
    opcode_buf->label_table_size = 0;
    opcode_buf->label_table_alloc_size = 0;
    opcode_buf->line_number = NULL;
    opcode_buf->line_number_size = 0;
}

static void fix_labels(OpcodeBuf *opcode_buf) {
	int label;
	int address;
	for (int i = 0; i < opcode_buf->size; i++) {
		if (opcode_buf->code[i] == DVM_JUMP 
			||opcode_buf->code[i] == DVM_JUMP_IF_TRUE
			|| opcode_buf->code[i] == DVM_JUMP_IF_FALSE) {
			label = (opcode_buf->code[i + 1] << 8) + (opcode_buf->code[i+2]);
			address = opcode_buf->label_table[label].label_address;
			opcode_buf->code[i+1] = (DVM_Byte)(address >> 8);
			opcode_buf->code[i+2] = (DVM_Byte)(address &0xff);
		}
		OpcodeInfo *info = &dvm_opcode_info[opcode_buf->code[i]];
		for (int j = 0; info->parameter[j] != '\0'; j++) {
			switch (info->parameter[j]) {
				case 'b':
					i++;
					break;
				case 's':
				case 'p':
					i += 2;
					break;
				default:
					DBG_assert(0, ("parameter..%s, j..%d", info->parameter, j));
			}
		}
	}
}

static DVM_Byte *fix_opcode_buf(OpcodeBuf *opcode_buf) {
	DVM_Byte *ret;
	fix_labels(opcode_buf);
	ret = MEM_realloc(opcode_buf->code, opcode_buf->size);
	MEM_free(opcode_buf->label_table);
			
	return ret;
}

static int calc_need_stack_size(DVM_Byte *code, int code_size) {
	OpcodeInfo *info;
	int stack_size = 0;
	for (int i = 0; i < code_size; i++) {
		info = &dvm_opcode_info[code[i]];
		if (info->stack_increment > 0) {
			stack_size += info->stack_increment;
		}

		for (int j = 0; info->parameter[j] != '\0'; j++) {
			switch (info->parameter[j]) {
				case 'b':
					i++;
					break;
				case 's':
				case 'p':
					i += 2;
					break;
				default:
					DBG_assert(0, ("parameter..%s, j..%d", info->parameter, j));
			}
		}
	}

	return stack_size;
}

static DVM_Class *search_class(DKC_Compiler *compiler, ClassDefinition *src) {
	char *src_package_name = dkc_package_name_to_string(src->package_name);
	for (int i = 0; i < compiler->dvm_class_count; i++) {
		if (dvm_compare_package_name(src_package_name, compiler->dvm_class[i].package_name)
		    && strcmp(src->name, compiler->dvm_class[i].name) == 0) {
			MEM_free(src_package_name);
			return &compiler->dvm_class[i];
		}
	}

	DBG_assert(0, ("function %s#%s not found.", src_package_name, src->name));
	return NULL;
}

static void set_class_identifier(ClassDefinition *class_definition, DVM_ClassIdentifier *class_identifier) {
	class_identifier->name = MEM_strdup(class_definition->name);
	class_identifier->package_name = dkc_package_name_to_string(class_definition->package_name);
}

static void add_method(DVM_Executable *executable, MemberDeclaration *member, DVM_Method *dest,
                       DVM_Boolean is_implemented) {
	dest->access_modifier = member->access_modifier;
	dest->is_abstract = member->u.method.is_abstract;
	dest->is_virtual = member->u.method.is_virtual;
	dest->is_override = member->u.method.is_override;
	dest->name = MEM_strdup(member->u.method.function_definition->name);
}

static void add_field(MemberDeclaration *member, DVM_Field *dest) {
	dest->access_modifier = member->access_modifier;
	dest->name = MEM_strdup(member->u.field.name);
	dest->type = dkc_copy_type_specifier(member->u.field.type);
}

static void add_class(DVM_Executable *executable, ClassDefinition *class_definition, DVM_Class *dest) {

	dest->is_abstract = class_definition->is_abstract;
	dest->access_modifier = class_definition->access_modifier;
	dest->class_or_interface = class_definition->class_or_interface;

	if (class_definition->super_class) {
		dest->super_class = MEM_malloc(sizeof(DVM_ClassIdentifier));
		set_class_identifier(class_definition->super_class, dest->super_class);
	} else {
		dest->super_class = NULL;
	}

	int interface_count = 0;
	for (ExtendsList *pos = class_definition->interface_list; pos; pos = pos->next) {
		interface_count++;
	}

	dest->interface_count = interface_count;
	dest->interface = MEM_malloc(sizeof(DVM_ClassIdentifier) * interface_count);

	interface_count = 0;
	for (ExtendsList *pos = class_definition->interface_list; pos; pos = pos->next) {
		set_class_identifier(pos->class_definition, &dest->interface[interface_count]);
		interface_count++;
	}

	int method_count, field_count = 0;
	for (MemberDeclaration *pos = class_definition->member; pos; pos = pos->next) {
		if (pos->kind == METHOD_MEMBER) {
			method_count++;
		} else {
			DBG_assert(pos->kind == FIELD_MEMBER, ("pos.kind..%d", pos->kind));
			field_count++;
		}
	}

	dest->field_count = field_count;
	dest->field = MEM_malloc(sizeof(DVM_Field) * field_count);
	dest->method_count = method_count;
	dest->method = MEM_malloc(sizeof(DVM_Method) * method_count);

	method_count = field_count = 0;

	for (MemberDeclaration *pos = class_definition->member; pos; pos = pos->next) {
		if (pos->kind == METHOD_MEMBER) {
			add_method(executable, pos, &dest->method[method_count], dest->is_implemented);
			method_count++;
		} else {
			DBG_assert(pos->kind == FIELD_MEMBER, ("pos.kind..%d", pos->kind));

			add_field(pos, &dest->field[field_count]);
			field_count++;
		}
	}
}

static void copy_opcode_buf(DVM_CodeBlock *code_block, OpcodeBuf *opcode_buf) {
    code_block->code_size = opcode_buf->size;
    code_block->code = fix_opcode_buf(opcode_buf);
    code_block->line_number_size = opcode_buf->line_number_size;
    code_block->line_number = opcode_buf->line_number;
    code_block->need_stack_size = calc_need_stack_size(code_block->code, code_block->code_size);
}

static void generate_field_initializer(DVM_Executable *executable, ClassDefinition *class_definition, DVM_Class *dvm_class) {
	OpcodeBuf opcode_buf;
    
    init_opcode_buf(&opcode_buf);

    for (ClassDefinition *class_definition_pos = class_definition; 
        class_definition_pos; 
        class_definition_pos = class_definition_pos->super_class) {
        for (MemberDeclaration *member_pos = class_definition_pos->member; 
            member_pos; 
            member_pos = member_pos->next) {
            if (member_pos->kind != FIELD_MEMBER) continue;
            if (member_pos->u.field.initializer) {
                generate_expression(executable, NULL, member_pos->u.field.initializer, &opcode_buf);
                generate_code(&opcode_buf, member_pos->u.field.initializer->line_number, DVM_DUPLICATE_OFFSET, 1);
                generate_code(&opcode_buf, member_pos->u.field.initializer->line_number, DVM_POP_FIELD_INT + get_opcode_type_offset(member_pos->u.field.type), member_pos->u.field.field_index);
            }
        }
    }
    
    copy_opcode_buf(&dvm_class->field_initializer, &opcode_buf);
}

static void add_classes(DKC_Compiler *compiler, DVM_Executable *executable) {
	DVM_Class *dvm_class;
	for (ClassDefinition *pos = compiler->class_definition_list; pos; pos = pos->next) {
		dvm_class = search_class(compiler, pos);
		dvm_class->is_implemented = DVM_TRUE;
        generate_field_initializer(executable, pos, dvm_class);
	}

	for (int i = 0; i < compiler->dvm_class_count; i++) {
		ClassDefinition *class_definition = dkc_search_class(compiler->dvm_class[i].name);
		add_class(executable, class_definition, &compiler->dvm_class[i]);
	}
}

static void add_function(DVM_Executable *executable, FunctionDefinition *src, DVM_Function *dest,
                         DVM_Boolean in_this_executable) {
	OpcodeBuf opcode_buf;

	dest->type = dkc_copy_type_specifier(src->type);
	dest->parameter = copy_parameter_list(src->parameter, &dest->parameter_count);

	if (src->block && in_this_executable) {
		init_opcode_buf(&opcode_buf);
		generate_statement_list(executable, src->block, src->block->statement_list, &opcode_buf);
		dest->is_implemented = DVM_TRUE;
        copy_opcode_buf(&dest->code_block, &opcode_buf);
		dest->local_variable = copy_local_variables(src, dest->parameter_count);
		dest->local_variable_count = src->local_variable_count - dest->parameter_count;
	} else {
		dest->is_implemented = DVM_FALSE;
		dest->local_variable = NULL;
		dest->local_variable_count = 0;
	}

	dest->is_method = src->class_definition ? DVM_TRUE : DVM_FALSE;
}

static int search_function(DKC_Compiler *compiler, FunctionDefinition *src) {
	char *src_package_name = dkc_package_name_to_string(src->package_name);
	char *function_name;
	if (src->class_definition) {
		function_name = dvm_create_method_function_name(src->class_definition->name, src->name);
	} else {
		function_name = src->name;
	}

	for (int i = 0; i < compiler->dvm_function_count; i++) {
		if (dvm_compare_package_name(src_package_name, compiler->dvm_function[i].package_name) &&
		    strcmp(function_name, compiler->dvm_function[i].name) == 0) {
			if (src->class_definition) {
				MEM_free(function_name);
			}
			return i;
		}
	}
	DBG_assert(0, ("search_function. function %s#%s not found.", src_package_name, src->name));
	return 0;
}

static void add_functions(DKC_Compiler *compiler, DVM_Executable *executable) {
    FunctionDefinition *function_def;
	int dest_index = 0;
	DVM_Boolean *in_this_executable = MEM_malloc(sizeof(DVM_Boolean) * compiler->dvm_function_count);

	for (int i = 0; i < compiler->dvm_function_count; i++) {
		in_this_executable[i] = DVM_FALSE;
	}

    for (function_def = compiler->function_list;
         function_def;
         function_def = function_def->next) {
        if (function_def->class_definition && function_def->block == NULL) {
	        continue;
        }
	    dest_index = search_function(compiler, function_def);
	    in_this_executable[dest_index] = DVM_TRUE;
	    add_function(executable, function_def, &compiler->dvm_function[dest_index], DVM_TRUE);
    }

	for (int i = 0; i < compiler->dvm_function_count; i++) {
		if (in_this_executable[i]) continue;
		function_def = dkc_search_function(compiler->dvm_function[i].name);
		add_function(executable, function_def, &compiler->dvm_function[i], DVM_FALSE);
	}

	MEM_free(in_this_executable);
}

static void add_top_level(DKC_Compiler *compiler, DVM_Executable *executable) {
    OpcodeBuf opcode_buf;
    init_opcode_buf(&opcode_buf);
    generate_statement_list(executable, NULL, compiler->statement_list, &opcode_buf);

    executable->code_size = opcode_buf.size;
    executable->code = fix_opcode_buf(&opcode_buf);
    executable->line_number_size = opcode_buf.line_number_size;
    executable->line_number = opcode_buf.line_number;
    executable->need_stack_size = calc_need_stack_size(executable->code, executable->code_size);
}

DVM_Executable *dkc_generate(DKC_Compiler *compiler) {
    DVM_Executable *executable = alloc_executable(compiler->package_name);
	executable->function_count = compiler->dvm_function_count;
	executable->function = compiler->dvm_function;
	executable->class_count = compiler->dvm_class_count;
	executable->class_definition = compiler->dvm_class;

	add_global_variable(compiler, executable);
	add_classes(compiler, executable);
    add_functions(compiler, executable);
    add_top_level(compiler, executable);

    return executable;
}
