//
// Created by sulvto on 18-6-9.
//

#include "MEM.h"
#include "diksamc.h"

static void add_function_to_compiler(FunctionDefinition *function_definition) {
	DKC_Compiler *compiler = dkc_get_current_compiler();
	if (compiler->function_list) {
		FunctionDefinition *pos;
		for (pos = compiler->function_list; pos->next; pos = pos->next) ;
		pos->next = function_definition;
	} else {
		compiler->function_list = function_definition;
	}
}

FunctionDefinition *dkc_create_function_definition(TypeSpecifier *type,
				char *identifier, ParameterList *parameter_list, 
				Block *block) {
	FunctionDefinition *function_def;
	DKC_Compiler *compiler = dkc_get_current_compiler();
	function_def = dkc_malloc(sizeof(FunctionDefinition));
	function_def->type = type;
	function_def->package_name = compiler->package_name;
	function_def->name = identifier;
	function_def->parameter = parameter_list;
	function_def->block = block;
	function_def->local_variable_count = 0;
	function_def->local_variable = NULL;
	function_def->class_definition = NULL;
	function_def->end_line_number = compiler->current_line_number;
	function_def->next = NULL;

	if (block) {
		block->type = FUNCTION_BLOCK;
		block->parent.function.function = function_def;
	}

	add_function_to_compiler(function_def);

	return function_def;
}

void dkc_function_define(TypeSpecifier *type,
				char *identifier, ParameterList *parameter_list, 
				Block *block) {
	FunctionDefinition *function_def;
	FunctionDefinition *pos;
	DKC_Compiler *compiler;

	if (dkc_search_function(identifier) 
		|| dkc_search_declaration(identifier, NULL)) {
		dkc_compiler_error(dkc_get_current_compiler()->current_line_number,
						FUNCTION_MULTIPLE_DEFINE_ERR,
						STRING_MESSAGE_ARGUMENT, "name", identifier,
						MESSAGE_ARGUMENT_END);
	} else {
		function_def = dkc_create_function_definition(type, identifier,
						parameter_list, block);
		if (block) {
			block->type = FUNCTION_BLOCK;
			block->parent.function.function = function_def;
		}

		compiler = dkc_get_current_compiler();
		if (compiler->function_list) {
			for (pos = compiler->function_list; pos->next; pos = pos->next);
			pos->next = function_def;
		} else {
			compiler->function_list = function_def;
		}
	}
}

ParameterList *dkc_create_parameter(TypeSpecifier *type, char *identifier) {
	ParameterList *parameter = dkc_malloc(sizeof(ParameterList));
	parameter->name = identifier;
	parameter->type = type;
	parameter->line_number = dkc_get_current_compiler()->current_line_number;
	parameter->next = NULL;

	return parameter;
}

ParameterList *dkc_chain_parameter_list(ParameterList *parameter_list,
                                        TypeSpecifier *type, char *identifier) {
	ParameterList *pos;
	for (pos = parameter_list; pos->next; pos = pos->next);
	pos->next = dkc_create_parameter(type, identifier);

	return parameter_list;
}

ArgumentList *dkc_create_argument_list(Expression *expression) {
	ArgumentList *argument = dkc_malloc(sizeof(ArgumentList));
	argument->expression = expression;
	argument->next = NULL;
	return argument;
}

ArgumentList *dkc_chain_argument_list(ArgumentList *argument_list, 
				Expression *expression) {
	ArgumentList *pos;
	for (pos = argument_list; pos->next; pos = pos->next) ;
	pos->next = dkc_create_argument_list(expression);
	return argument_list;
}

StatementList *dkc_create_statement_list(Statement *statement) {
	StatementList *list = dkc_malloc(sizeof(StatementList));
	list->statement = statement;
	list->next = NULL;
	return list;
}

StatementList *dkc_chain_statement_list(StatementList *list, Statement *statement)  {

	if (list == NULL) {
		return dkc_create_statement_list(statement);
	}

	StatementList *pos;
	for (pos = list; pos->next; pos = pos->next) ;
	pos->next = dkc_create_statement_list(statement);

	return list;
}

ExpressionList *dkc_create_expression_list(Expression *expression) {
	ExpressionList *list = dkc_malloc(sizeof(ExpressionList));
	list->expression = expression;
	list->next = NULL;
	return list;
}

ExpressionList *dkc_chain_expression_list(ExpressionList *expression_list, Expression *expression) {
	ExpressionList *pos;
	for (pos = expression_list; pos->next; pos = pos->next );
	pos->next= dkc_create_expression_list(expression);

	return expression_list;
}

Expression *dkc_alloc_expression(ExpressionKind kind) {
	Expression *expr = dkc_malloc(sizeof(Expression));	
	expr->type = NULL;
	expr->kind = kind;
	expr->line_number = dkc_get_current_compiler()->current_line_number;

	return expr;
}

Expression *dkc_create_comma_expression(Expression *left, Expression *right) {
	Expression *expr = dkc_alloc_expression(COMMA_EXPRESSION);
	expr->u.comma.left = left;
	expr->u.comma.right = right;

	return expr;	
}

Expression *dkc_create_assign_expression(Expression *left,
				AssignmentOperator operator, Expression *operand) {
	Expression *expr = dkc_alloc_expression(ASSIGN_EXPRESSION);
	expr->u.assign_expression.left = left;
	expr->u.assign_expression.operator = operator;
	expr->u.assign_expression.operand = operand;

	return expr;
}

Expression *dkc_create_binary_expression(ExpressionKind kind,
				Expression *left, Expression *right) {
	Expression *expr = dkc_alloc_expression(kind);
	expr->u.binary_expression.left = left;
	expr->u.binary_expression.right = right;

	return expr;
}

Expression *dkc_create_minus_expression(Expression *operand) {
	Expression *expr = dkc_alloc_expression(MINUS_EXPRESSION);
	expr->u.minus_expression = operand;

	return expr;
}

Expression *dkc_create_logical_not_expression(Expression *operand) {
	Expression *expr = dkc_alloc_expression(LOGICAL_NOT_EXPRESSION);
	expr->u.logical_not = operand;

	return expr;
}

Expression *dkc_create_function_call_expression(Expression *function, ArgumentList *argument_list) {
	Expression *expr = dkc_alloc_expression(FUNCTION_CALL_EXPRESSION);
	expr->u.function_call_expression.function = function;
	expr->u.function_call_expression.argument = argument_list;

	return expr;
}

Expression *dkc_create_incdec_expression(Expression *operand, ExpressionKind inc_or_dec) {
	Expression *expr = dkc_alloc_expression(inc_or_dec);
	expr->u.inc_dec.operand = operand;

	return expr;
}

Expression *dkc_create_index_expression(Expression *array, Expression *index) {
	Expression *expr = dkc_alloc_expression(INDEX_EXPRESSION);
	expr->u.index_expression.array = array;
	expr->u.index_expression.index = index;

	return expr;
}

Expression *dkc_create_member_expression(Expression *expression, char *member_name) {
	Expression *expr = dkc_alloc_expression(MEMBER_EXPRESSION);
	expr->u.member_expression.expression = expression;
	expr->u.member_expression.member_name = member_name;

	return expr;
}

Expression *dkc_create_identifier_expression(char *identifier) {
	Expression *expr = dkc_alloc_expression(IDENTIFIER_EXPRESSION);
	expr->u.identifier.name = identifier;

	return expr;
}

Expression *dkc_create_boolean_expression(DVM_Boolean boolean) {
	Expression *expr = dkc_alloc_expression(BOOLEAN_EXPRESSION);
	expr->u.boolean_value = boolean;

	return expr;
}

Expression *dkc_create_null_expression() {
	Expression *expr = dkc_alloc_expression(NULL_EXPRESSION);

	return expr;
}

Expression *dkc_create_array_literal_expression(ExpressionList *expression_list) {
	Expression *expr = dkc_alloc_expression(ARRAY_LITERAL_EXPRESSION);
	expr->u.array_literal = expression_list;

	return expr;
}

TypeSpecifier *dkc_create_type_specifier(DVM_BasicType basic_type) {
	TypeSpecifier *type_specifier = dkc_malloc(sizeof(TypeSpecifier));
	type_specifier->basic_type = basic_type;
	type_specifier->derive = NULL;

	return type_specifier;
}

ArrayDimension *dkc_chain_array_dimension(ArrayDimension *list, ArrayDimension *dim) {
	ArrayDimension *pos;
	for (pos = list; pos->next; pos = pos->next) ;
	pos->next = dim;

	return list;
}

Expression *dkc_create_array_creation(DVM_BasicType basic_type,
									  ArrayDimension *dim_expr_list,
									  ArrayDimension *dim_list) {
	Expression *expr = dkc_alloc_expression(ARRAY_CREATION_EXPRESSION);
	expr->u.array_creation.type = dkc_create_type_specifier(basic_type);
	expr->u.array_creation.dimension = dkc_chain_array_dimension(dim_expr_list, dim_list);

	return expr;
}

Statement *alloc_statement(StatementType type) {
	Statement *statement = dkc_malloc(sizeof(Statement));
	statement->type = type;
	statement->line_number = dkc_get_current_compiler()->current_line_number;

	return statement;
}

Statement *dkc_create_expression_statement(Expression *expression) {
	Statement *statement = alloc_statement(EXPRESSION_STATEMENT);
	statement->u.expression_s = expression;

	return statement;
}

Statement *dkc_create_if_statement(Expression *condition, 
				Block *then_block, Elseif *elseif_list,
				Block *else_block) {
	Statement *statement = alloc_statement(IF_STATEMENT);
	statement->u.if_s.condition = condition;
	statement->u.if_s.then_block = then_block;
	statement->u.if_s.elseif_list = elseif_list;
	statement->u.if_s.else_block = else_block;

	return statement;
}

Elseif *dkc_create_elseif(Expression *condition, Block *block) {
	Elseif *elseif = dkc_malloc(sizeof(Elseif));
	elseif->condition = condition;
	elseif->block = block;
	elseif->next = NULL;
	return elseif;
}

Elseif *dkc_chain_elseif_list(Elseif *elseif_list, Elseif *elseif) {
	Elseif *pos;
	for (pos = elseif_list; pos->next; pos = pos->next) ;
	pos->next = elseif;

	return elseif_list;
}

Statement *dkc_create_while_statement(char *label, Expression *condition, 
				Block *block) {
	Statement *statement = alloc_statement(WHILE_STATEMENT);
	statement->u.while_s.label = label;
	statement->u.while_s.condition = condition;
	statement->u.while_s.block = block;
	block->type = WHILE_STATEMENT_BLOCK;
	block->parent.statement.statement = statement;

	return statement;
}

Statement *dkc_create_for_statement(char *label, Expression *init, 
				Expression *condition, Expression *post, 
				Block *block) {
	Statement *statement = alloc_statement(FOR_STATEMENT);
	statement->u.for_s.label = label;
	statement->u.for_s.init = init;
	statement->u.for_s.condition = condition;
	statement->u.for_s.post = post;
	statement->u.for_s.block = block;

	block->type = FOR_STATEMENT_BLOCK;
	block->parent.statement.statement = statement;

	return statement;
}

Statement *dkc_create_foreach_statement(char *label, char *variable, 
				Expression *collection, Block *block) {
	Statement *statement = alloc_statement(FOREACH_STATEMENT);
	statement->u.foreach_s.label = label;
	statement->u.foreach_s.variable = variable;
	statement->u.foreach_s.collection = collection;
	statement->u.foreach_s.block = block;

	return statement;
}

Statement *dkc_create_return_statement(Expression *expression) {
	Statement *statement = alloc_statement(RETURN_STATEMENT);
	statement->u.return_s.return_value = expression;

	return statement;
}

Statement *dkc_create_break_statement(char *label) {
	Statement *statement = alloc_statement(BREAK_STATEMENT);
	statement->u.break_s.label = label;

	return statement;
}

Statement *dkc_create_continue_statement(char *label) {
	Statement *statement = alloc_statement(CONTINUE_STATEMENT);
	statement->u.continue_s.label = label;

	return statement;
}

Statement *dkc_create_try_statement(Block *try_block, CatchClause *catch_clause, Block *finally_block) {
	Statement *statement = alloc_statement(TRY_STATEMENT);
	statement->u.try_s.try_block = try_block;
	statement->u.try_s.catch_clause = catch_clause;
	statement->u.try_s.finally_block = finally_block;

	return statement;
}

Statement *dkc_create_throw_statement(Expression *expression) {
	Statement *statement = alloc_statement(THROW_STATEMENT);
	statement->u.throw_s.exception = expression;

	return statement;
}

Declaration *dkc_alloc_declaration(TypeSpecifier *type, char *identifier) {
	Declaration *declaration = dkc_malloc(sizeof(Declaration));
	declaration->name = identifier;
	declaration->type = type;
	declaration->variable_index = -1;

	return declaration;
}

Statement *dkc_create_declaration_statement(TypeSpecifier *type, char *identifier, Expression *initializer) {
	Statement *statement = alloc_statement(DECLARATION_STATEMENT);
	Declaration *declaration = dkc_alloc_declaration(type, identifier);
	declaration->initializer = initializer;

	statement->u.declaration_s = declaration;

	return statement;
} 
