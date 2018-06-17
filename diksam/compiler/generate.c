//
// Created by sulvto on 18-6-6.
//

#include "MEM.h"
#include "diksamc.h"

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

static DVM_Executable *alloc_executable() {
	DVM_Executable *executable = MEM_malloc(sizeof(DVM_Executable));
	executable->constant_pool_count = 0;
	executable->constant_pool = NULL;
	executable->global_variable_count = 0;
	executable->global_variable = NULL;
	executable->function_count = 0;
	executable->function = NULL;
	executable->code_size = 0;
	executable->code = NULL;

	return executable;
}

static DVM_TypeSpecifier *copy_type_specifier(TypeSpecifier *src);

static DVM_LocalVariable *copy_local_varialbes(FunctionDefinition *function_def, 
				int parameter_count) {
	int local_variable_count = function_def->local_variable_count - parameter_count;

	DVM_LocalVariable *desc = MEM_malloc(sizeof(DVM_LocalVariable) * local_variable_count);

	for (int i = 0; i < local_variable_count; i++) {
		desc[i].name = 
				MEM_strdup(function_def->local_variable[i + parameter_count]->name);
		desc[i].type = 
				copy_type_specifier(function_derive->local_variable[i+parameter_count]->type);
	}

	return dest;
}

static DVM_LocalVariable *copy_paramter_list(ParameterList *src, 
				int *parameter_count_p)  {
	int parameter_count = 0;

	for (ParameterList *parameter = src; parameter; parameter = parameter->next) {
		parameter_count++;
	}

	*parameter_count_p = parameter_count;

	DVM_LocalVariable *desc = 
			MEM_malloc(sizeof(DVM_LocalVariable) * parameter_count);

	for (ParameterList *parameter = src; parameter; parameter = parameter->next) {
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

	for (derive = sec->derive; derive; derive = derive->next) {
		derive_count++;
	}

	desc->derive_count = derive_count;
	dest->derive = MEM_malloc(sizeof(DVM_TypeDerive) * derive_count);
	for (i = 0, derive = src->derive; derive; derive = derive->next, i++) {
		switch (derive->tag) {
			case FUNCTION_DERIVE:
				desc->derive[i].tag = DVM_FUNCTION_DERIVE;
				desc->derive[i].u.function_derive.parameter = copy_paramter_list(derive->u.function_derive.parameter_list, &parameter_count);
				desc->derive[i].u.function_derive.parameter_count = parameter_count;
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
		executable->global_variable[i].name = MEM_strdup(declaration_list[i]->declaration->name);
		executable->global_variable[i].type = copy_type_specifier(declaration->declaration->type);
	}
}

static void copy_function(FunctionDefinition *src, DVM_Function *dest) {
	dest->type = copy_type_specifier(src->type);
	dest->name = MEM_strdup(src->name);
	dest->parameter = copy_paramter_list(src->parameter,
                                         &dest->parameter_count);

    if (src->block) {
        dest->local_variable = 
				copy_local_varialbes(src, dest->parameter_count);
        dest->local_variable_count = 
				src->local_variable_count - dest->parameter_count;
    } else {
        dest->local_variable = NULL;
        dest->local_variable_count = 0;
    }
}

static void generate_code(OpcodeBuf *opcode_buf, 
				int line_number, DVM_Opcode code, ...) {
	va_list ap;
	char *parameter;
	int parameter_count;
	int start_pc;

	va_start(op, code);

	parameter = dvm_opcode_info[(int) code].parameter;
	parameter_count = strlen(parameter);
	if (opcode_buf->alloc_size < opcode_buf->size + 1 + (parameter_count * 2)) {
		opcode_buf->code = 
				MEM_realloc(opcode_buf->code, 
								opcode_buf->alloc_size, 
								OPCODE_ALLOC_SIZE);
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
static int add_constant_pool(DVM_Executable executable, 
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
		constant_pool.u.c_double = expression>u.double_value;
		int constant_pool_index = add_constant_pool(executable, &constant_pool);

		generate_code(opcode_buf, expression->line_number, DVM_PUSH_DOUBLE, constant_pool_index);
	}
}

static void generate_string_expression(DVM_Executable *executable, Expression *executable, OpcodeBuf *opcode_buf) {
	DVM_ConstantPool = constant_pool;
	constant_pool.tag = DVM_CONSTANT_STRING;
	constant_pool.u.c_string = expression->u.string_value;
	int constant_pool_index = add_constant_pool(executable, &constant_pool);

	generate_code(executable, expression->line_number, DVM_PUSH_STRING, constant_pool_index);
}

static void generate_identifier_expression(DVM_Executable *executable, Block current_block, Expression *executable, OpcodeBuf *opcode_buf) {
	if (expression->u.identifier.is_function) {
		generate_code(opcode_buf, expression->line_number, DVM_PUSH_FUNCTION, expression->u.identifier.u.function->index);
		
	} else if (expression->u.identifier.u.declaration->is_local) {
		generate_code(opcode_buf, expression->line_number, DVM_PUSH_STACK_INT + get_opcode_type_offset(expression->u.identifier.u.declaration->type->basic_type), expression->u.identifier.u.declaration->variable_index);
	} else {
		generate_code(opcode_buf, expression->line_number, DVM_PUSH_STATIC_INT + get_opcode_type_offset(expression->u.identifier.u.declaration->type->basic_type), expression->u.identifier.u.declaration->variable_index);
	}
}

static void generate_pop_to_identifier(Declaration *declaration, int line_number, OpcodeBuf *opcode_buf) {
	if (dec->is_local) {
		generate_code(opcode_buf, 
					line_number, 
					DVM_POP_STACK_INT + get_opcode_type_offset(declaration->type->basic_type), 
					declaration->variable_index);
	} else {
		generate_code(opcode_buf, 
					line_number, 
					DVM_POP_STATIC_INT + get_opcode_type_offset(declaration->type->basic_type), 
					declaration->variable_index);
	}
}

static void generate_assign_expression(DVM_Executable *executable, 
				Block *current_block, Expression *expression, 
				OpcodeBuf *opcode_buf, DVM_Boolean is_toplevel) {
	if (expression->u.assign_expression.operator != NORMAL_ASSIGN) {
		generate_identifier_expression(executable, 
						block, expression->u.assign_expression.left, 
						opcode_buf);
	}
	generate_expression(executable, block, 
					expression->u.assign_expression.operand, opcode_buf);
	switch (expression->u.assign_expression.operator) {
		case NORMAL_ASSIGN:
			break;
		case ADD_ASSIGN:
			generate_code(opcode_buf, 
						expression->line_number, 
						DVM_ADD_INT + get_opcode_type_offset(expression->type->basic_type));
			break;
		case SUB_ASSIGN:
			generate_code(opcode_buf, 
						expression->line_number, 
						DVM_SUB_INT + get_opcode_type_offset(expression->type->basic_type));
			break;
		case MUL_ASSIGN:
			generate_code(opcode_buf, 
						expression->line_number, 
						DVM_MUL_INT + get_opcode_type_offset(expression->type->basic_type));
			break;
		case DIV_ASSIGN:
			generate_code(opcode_buf, 
						expression->line_number, 
						DVM_DIV_INT + get_opcode_type_offset(expression->type->basic_type));
			break;
		case MOD_ASSIGN:
			generate_code(opcode_buf, 
						expression->line_number, 
						DVM_MOD_INT + get_opcode_type_offset(expression->type->basic_type));
			break;
		default:
			DBG_assert(0, ("operator..%d\n", expression->u.assign_expression.operator));
	}

	if (!is_toplevel) {
		generate_code(opcode_buf, expression->line_number, DVM_DUPLICATE);
	}

	generate_pop_to_identifier(expression->u.assign_expression.left->u.identifier.u.declaration, expression->line_number, opcode_buf);
}

static void generate_binary_expression(DVM_Executable *executable, 
				Block *current_block, DVM_Opcode code, 
				OpcodeBuf *opcode_buf) {
	DBG_assert(expression->u.binary_expression.left->type->basic_type == expression->u.binary_expression.right->type->basic_type, ("left..%d, right..%d", expression->u.binary_expression.left->type->basic_type, expression->u.binary_expression.right->type->basic_type));

	generate_expression(executable, current_block, 
					expression->u.binary_expression.left, opcode_buf);
	generate_expression(executable, current_block, 
					expression->u.binary_expression.right, opcode_buf);
	generate_code(opcode_buf, expression->line_number, code + get_opcode_type_offset(expression->u.binary_expression.left->type->basic_type));
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
				Block *current_block, Expression *executable, 
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
				Block *current_block, Expression *executable, 
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
				Block current_block, Expression *expression, 
				ExpressionKind kind, OpcodeBuf *opcode_buf, 
				DVM_Boolean is_toplevel) {
	generate_expression(executable, current_block, 
					expression.u.inc_or_dec.operand, opcode_buf);

	if (kind == INCREMENT_EXPRESSION) {
		generate_code(opcode_buf, expression->line_number, DVM_INCREMENT);
	} else {
		generate_code(opcode_buf, expression->line_number, DVM_DECREMENT);	
	}

	if (!is_toplevel) {
		generate_code(opcode_buf, expression->line_number, DVM_DUPLICATE);
	}

	generate_pop_to_identifier(expression->u.inc_dec.operand->u.identifier.u.declaration, expression->line_number, opcode_buf);
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
		case IDENTIFIER_EXPRESSON:
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
							+  get_opcode_type_offset(expression->type->basic_type));
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
		case INCREMENT_EXPRESSION:
		case DECREMENT_EXPERSSION:
			generate_inc_dec_expression(executable, current_block, 
							expression, expression->kind, opcode_buf, DVM_FALSE);
			break;
		case CAST_EXPRESSION:
			generate_cast_expression(executable, current_block, 
							expression, opcode_buf);
		case EXPRESSION_KIND_COUNT_PLUS_1:
		default:
			DBG_assert(0, ("expression->kind..%d\n", expression->kind));
}

static void generate_expression_statement(DVM_Executable *executable, 
				Block *current_block, Expression *expression, 
				OpcodeBuf *opcode_buf) {
	if (expression->kind == ASSIGN_EXPRESSION) {
		generate_assign_expression(executable, current_block, 
						expression, opcode_buf, DVM_TURE);
	} else if (expression->kind == INCREMENT_EXPRESSION 
					||expression->kind = DECREMENT_EXPERSSION) {
		generate_inc_dec_expression(executable, current_block, 
						expression, expression->kind, opcode_buf, DVM_TURE);
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
	generate_statement_list(executable, statement->block, 
					if_s->then_block->statement_list, opcode_buf);
	end_label = get_label(opcode_buf);
	generate_code(opcode_buf, statement->line_number, DVM_JUMP, end_label);
	set_label(opcode_buf, if_false_label);
	for (Elseif *elseif = if_s->elseif_list; elseif; elseif = elseif->next) {
		generate_expression(executable, current_block, elseif->condition, 
						opcode_buf);
		if_false_label = get_label(opcode_buf);
		generate_code(executable, statement->line_number, 
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

	generate_expression(executable, block, while_s->condition, opcode_buf);

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
	ForStatement for_s = &statement->u.for_s;

	if (for_s->init) {
		generate_expression(executable, current_block, for_s->init, opcode_buf);
	}

	loop_label = get_label(opcode_buf);

	if (for_s->condition) {
		generate_expression(executable, current_block. for_s->condition, opcode_buf);
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

	set_label(opcode_buf. for_s->block->parent.statement.break_label);

}

static void generate_return_statement(DVM_Executable *executable, 
				Block *current_block, Statement *statement, 
				OpcodeBuf *opcode_buf) {
	DBG_assert(statement->u.return_s.return_value != NULL, 
					("return value is null."));
	generate_expression(executable, statement->i.return_s.return_value, opcode_buf);
	generate_code(opcode_buf, statement->line_number, DVM_RETURN);
}

static void generate_break_statement(DVM_Executable *executable, 
				Block *current_block, Statement *statement, 
				OpcodeBuf *opcode_buf) {
	BreakStatement *break_s = &statement->u.break_s;
	for (Block *block_p = current_block; block_p; block_p = block_p->outer_block) {
		if (block_p->ytpe != WHILE_STATEMENT_BLOCK 
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
		} else if (block_p->type = FOR_STATEMENT_BLOCK) {
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
		dkc_compiler_error(statement->line_number, 
						LABEL_NOT_FOUND_ERR, 
						STRING_MESSAGE_ARGUMENT, "label", break_s->label, 
						MESSAGE_ARGUMENT_END);
	}

	generate_code(opcode_buf, statement->line_number, 
					DVM_JUMP, block_p->parent.statement.block_label);
}

static void generate_continue_statement(DVM_Executable *executable, 
				Block *current_block, Statement *statement, 
				OpcodeBuf *opcode_buf) {
	ContinueStatement *continue_s = &statement->u.continue_s;
    for (Block *block_p = current_block; block_p; block_p = block_p->outer_block) {
    	if (block_p->ytpe != WHILE_STATEMENT_BLOCK 
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
                                                                                                      
    		if (strcmp(continue_s->label, 
    					block_p->parent.statement.statement->u.while_s.label) == 0) {
    			break;
    		}
    	} else if (block_p->type = FOR_STATEMENT_BLOCK) {
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
    	dkc_compiler_error(statement->line_number, 
    					LABEL_NOT_FOUND_ERR, 
    					STRING_MESSAGE_ARGUMENT, "label", continue_s->label, 
    					MESSAGE_ARGUMENT_END);
    }
                                                                                                      
    generate_code(opcode_buf, statement->line_number, 
    				DVM_JUMP, continue_s->parent.statement.continue_label);
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
            case while_statement:
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

static void add_functions(DKC_Compiler *compiler, DVM_Executable *executable) {
    FunctionDefinition *function_def;
    int function_count = 0;

    for (FunctionDefinition *function_def = compiler->function_list;
         function_def;
         function_def = function_def.next) {
        function_count++;
    }

    executable->function_count = function_count;
    executable->function = MEM_malloc(sizeof(DVM_Function) * function_count);

    int i;
    OpcodeBuf opcode_buf;

    for (FunctionDefinition *function_def = compiler->function_list, i = 0;
         function_def;
         function_def = function_def->next, i++) {
        copy_function(function_def, &executable->function[i]);

        if (function_def->block) {
            init_opcode_buf(&opcode_buf);
            generate_statement_list(executable,
                                    function_def->block,
                                    function_def->block->statement_list,
                                    &opcode_buf);
            executable->function[i].is_implemented = DVM_TURE;
            executable->function[i].code_size = opcode_buf.size;
            executable->function[i].code = fix_opcode_buf(&opcode_buf);
            executable->function[i].line_number_size = opcode_buf.line_number_size;
            executable->function[i].line_number = opcode_buf.line_number;
            executable->function[i].need_stack_size = calc_need_stack_size(executable->function[i].code,
                                                                           executable->function[i].coed_size);
        } else {
            executable->function[i].is_implemented = DVM_FALSE;
        }
    }
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
    DVM_Executable *executable = alloc_executable();

    add_global_variable(compiler, executable);
    add_functions(compiler, executable);
    add_top_level(compiler, executable);

    return executable;
}
