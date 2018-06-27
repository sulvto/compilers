//
// Created by sulvto on 18-6-9.
//

#include "MEM.h"
#include "DBG.h"
#include "diksamc.h"
#include "DVM_dev.h"

static void add_function_to_compiler(FunctionDefinition *function_definition) {
	DKC_Compiler *compiler = dkc_get_current_compiler();
	if (compiler->function_list) {
		FunctionDefinition *pos;
		for (pos = compiler->function_list; pos->next; pos = pos->next);
		pos->next = function_definition;
	} else {
		compiler->function_list = function_definition;
	}
}

/**
 * create and add to compiler
 */
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

	if (dkc_search_function(identifier) 
		|| dkc_search_declaration(identifier, NULL)) {
		dkc_compile_error(dkc_get_current_compiler()->current_line_number,
						FUNCTION_MULTIPLE_DEFINE_ERR,
						STRING_MESSAGE_ARGUMENT, "name", identifier,
						MESSAGE_ARGUMENT_END);
	} else {
		dkc_create_function_definition(type, identifier,
						parameter_list, block);
	}
}

PackageName *dkc_create_package_name(char *identifier) {
	PackageName *package_name = MEM_malloc(sizeof(PackageName));
	package_name->name = identifier;
	package_name->next = NULL;

	return package_name;
}

PackageName *dkc_chain_package_name(PackageName *list, char *identifier) {
	PackageName *pos;
	for (pos = list; pos->next; pos = pos->next);
	pos->next = dkc_create_package_name(identifier);

	return list;
}

RequireList *dkc_create_require_list(PackageName *package_name) {
	DKC_Compiler *compiler = dkc_get_current_compiler();

	char *current_package_name = dkc_package_name_to_string(compiler->package_name);
	char *req_package_name = dkc_package_name_to_string(package_name);
	if (dvm_compare_string(req_package_name, current_package_name)
			&& compiler->source_suffix == DKH_SOURCE) {
		dkc_compile_error(compiler->current_line_number, REQUIRE_ITSELF_ERR, MESSAGE_ARGUMENT_END);
	}

	MEM_free(current_package_name);
	MEM_free(req_package_name);

	RequireList *require_list = dkc_malloc(sizeof(RequireList));
	require_list->line_number = dkc_get_current_compiler()->current_line_number;
	require_list->package_name = package_name;
	require_list->next = NULL;

	return require_list;
}


RequireList *dkc_chain_require_list(RequireList *list, RequireList *add) {
	RequireList *pos;
	char buf[LINE_BUF_SIZE];

	for (pos = list; pos->next; pos = pos->next) {
		if (dkc_compare_package_name(pos->package_name, add->package_name)) {
			char *package_name = dkc_package_name_to_string(add->package_name);
			dvm_strncpy(buf, package_name, LINE_BUF_SIZE);
			MEM_free(package_name);
			dkc_compile_error(dkc_get_current_compiler()->current_line_number, REQUIRE_DUPLICATE_ERR,
			                  STRING_MESSAGE_ARGUMENT, "package_name", buf,
			                  MESSAGE_ARGUMENT_END);
		}
	}

	pos->next = add;

	return list;
}

RenameList *dkc_create_rename_list(PackageName *package_name, char *identifier) {
	PackageName *tail;
	PackageName *pre_tail;
	for (tail = package_name; tail->next; tail = tail->next) {
		pre_tail = tail;
	}

	if (pre_tail == NULL) {
		dkc_compile_error(dkc_get_current_compiler()->current_line_number,
		                  RENAME_HAS_NO_PACKAGED_NAME_ERR,
		                  MESSAGE_ARGUMENT_END);
	}

	pre_tail->next = NULL;

	RenameList *rename_list = dkc_malloc(sizeof(RenameList));
	rename_list->line_number = dkc_get_current_compiler()->current_line_number;
	rename_list->package_name = package_name;
	rename_list->original_name = tail->name;
	rename_list->renamed_name = identifier;
	rename_list->next = NULL;

	return rename_list;
}

RenameList *dkc_chain_rename_list(RenameList *list, RenameList *add) {
	RenameList *pos;
	for (pos = list; pos->next; pos = pos->next);
	pos->next = add;

	return list;
}

static RequireList *add_default_package(RequireList *require_list) {
	RequireList *pos;
	DVM_Boolean default_package_required = DVM_FALSE;
	for (pos = require_list; pos; pos = pos->next) {
		char *temp_name = dkc_package_name_to_string(pos->package_name);
		if (strcmp(temp_name, DVM_DIKSAM_DEFAULT_PACKAGE) == 0) {
			default_package_required = DVM_TRUE;
		}
		MEM_free(temp_name);
	}

	if (!default_package_required) {
		PackageName *package_name = dkc_create_package_name(DVM_DIKSAM_DEFAULT_PACKAGE_P1);
		package_name = dkc_chain_package_name(package_name, DVM_DIKSAM_DEFAULT_PACKAGE_P2);
		RequireList *require_temp = require_list;
		require_list = dkc_create_require_list(package_name);
		require_list->next = require_temp;
	}

	return require_list;
}

void dkc_set_require_and_rename_list(RequireList *require_list, RenameList *rename_list) {
	DKC_Compiler *compiler = dkc_get_current_compiler();
	char *current_package_name = dkc_package_name_to_string(compiler->package_name);
	if (!dvm_compare_string(current_package_name, DVM_DIKSAM_DEFAULT_PACKAGE)) {
		require_list = add_default_package(require_list);
	}
	MEM_free(current_package_name);
	compiler->require_list = require_list;
	compiler->rename_list = rename_list;
}

ParameterList *dkc_create_parameter(TypeSpecifier *type, char *identifier) {
	ParameterList *parameter = dkc_malloc(sizeof(ParameterList));
	parameter->name = identifier;
	parameter->type = type;
	parameter->line_number = dkc_get_current_compiler()->current_line_number;
	parameter->next = NULL;

	return parameter;
}

ParameterList *dkc_chain_parameter(ParameterList *parameter_list,
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

Expression *dkc_create_down_cast_expression(Expression *operand, TypeSpecifier *type) {
	Expression *expression = dkc_alloc_expression(DOWN_CAST_EXPRESSION);
	expression->u.down_cast.operand = operand;
	expression->u.down_cast.type = type;
	
	return expression;
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

Expression *dkc_create_instanceof_expression(Expression *operand, TypeSpecifier *type) {
	Expression *expression = dkc_alloc_expression(INSTANCEOF_EXPRESSION);
	expression->u.instanceof_expression.operand = operand;
	expression->u.instanceof_expression.type = type;
	
	return expression;
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

Expression *dkc_create_this_expression(void) {
	Expression *expr = dkc_alloc_expression(THIS_EXPRESSION);
	
	return expr;
}

Expression *dkc_create_super_expression(void) {
	Expression *expr = dkc_alloc_expression(SUPER_EXPRESSION);
	
	return expr;
}

Expression *dkc_create_new_expression(char *class_name, char *method_name,
                                      ArgumentList *argument) {
	Expression *expression = dkc_alloc_expression(NEW_EXPRESSION);
	expression->u.new_expression.class_name = class_name;
	expression->u.new_expression.class_definition = NULL;
	expression->u.new_expression.method_name = method_name;
	expression->u.new_expression.method_declaration = NULL;
	expression->u.new_expression.argument = argument;
	
	return expression;
}

Expression *dkc_create_array_literal_expression(ExpressionList *expression_list) {
	Expression *expr = dkc_alloc_expression(ARRAY_LITERAL_EXPRESSION);
	expr->u.array_literal = expression_list;

	return expr;
}

Expression *dkc_create_basic_array_creation(DVM_BasicType basic_type,
                                            ArrayDimension *dim_expr_list,
                                            ArrayDimension *dim_list) {
	TypeSpecifier *type = dkc_create_type_specifier(basic_type);
	Expression *expression = dkc_create_class_array_creation(type, dim_expr_list, dim_list);

	return expression;
}

Expression *dkc_create_class_array_creation(TypeSpecifier *type,
                                            ArrayDimension *dim_expr_list,
                                            ArrayDimension *dim_list) {
	Expression *expression = dkc_alloc_expression(ARRAY_CREATION_EXPRESSION);
	expression->u.array_creation.type = type;
	expression->u.array_creation.dimension = dkc_chain_array_dimension(dim_expr_list, dim_list);

	return expression;
}

TypeSpecifier *dkc_create_type_specifier(DVM_BasicType basic_type) {
	TypeSpecifier *type_specifier = dkc_malloc(sizeof(TypeSpecifier));
	type_specifier->basic_type = basic_type;
	type_specifier->derive = NULL;

	return type_specifier;
}

TypeSpecifier *dkc_create_class_type_specifier(char *identifier) {
	TypeSpecifier *type = dkc_alloc_type_specifier(DVM_CLASS_TYPE);
	type->class_ref.identifier = identifier;
	type->class_ref.class_definition = NULL;
	type->line_number = dkc_get_current_compiler()->current_line_number;

	return type;
}

TypeSpecifier *dkc_create_array_type_specifier(TypeSpecifier *base) {
	TypeDerive *new_derive = dkc_alloc_type_derive(ARRAY_DERIVE);
	if (base->derive == NULL) {
		base->derive = new_derive;
	} else {
		TypeDerive *pos;
		for (pos = base->derive; pos->next != NULL; pos = pos->next);
		pos->next = new_derive;
	}

	return base;
}

ArrayDimension *dkc_create_array_dimension(Expression *expression) {
	ArrayDimension *dimension = dkc_malloc(sizeof(ArrayDimension));
	dimension->expression = expression;
	dimension->next = NULL;

	return dimension;
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

Statement *dkc_alloc_statement(StatementType type) {
	Statement *statement = dkc_malloc(sizeof(Statement));
	statement->type = type;
	statement->line_number = dkc_get_current_compiler()->current_line_number;

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

Block *dkc_alloc_block(void) {
	Block *new_block = dkc_malloc(sizeof(Block));
	new_block->type = UNDEFINED_BLOCK;
	new_block->outer_block = NULL;
	new_block->statement_list = NULL;
	new_block->declaration_list = NULL;
	return new_block;
}

Block *dkc_open_block(void) {
	DKC_Compiler *compiler = dkc_get_current_compiler();

	Block *new_block = dkc_alloc_block();

	new_block->outer_block = compiler->current_block;
	compiler->current_block = new_block;

	return new_block;
}

Block *dkc_close_block(Block *block, StatementList *statement_list) {
	DKC_Compiler *compiler = dkc_get_current_compiler();
	DBG_assert(block == compiler->current_block, ("block mismatch.\n"));

	block->statement_list = statement_list;
	compiler->current_block = block->outer_block;

	return block;
}

static DVM_AccessModifier conv_access_modifier(ClassOrMemberModifierKind kind) {
	if (kind == PUBLIC_MODIFIER) {
		return DVM_PUBLIC_ACCESS;
	} else if (kind == PRIVATE_MODIFIER) {
		return DVM_PRIVATE_ACCESS;
	} else {
		DBG_assert(kind == NOT_SPECIFIED_MODIFIER, ("kind..%d\n", kind));
		return DVM_FILE_ACCESS;
	}
}

void dkc_start_class_definition(ClassOrMemberModifierList *modifier, DVM_ClassOrInterface class_or_interface,
                                char *identifier,
                                ExtendsList *extends) {
	DKC_Compiler *compiler = dkc_get_current_compiler();
	ClassDefinition *class_definition = dkc_malloc(sizeof(ClassDefinition));
	class_definition->is_abstract = (class_or_interface == DVM_INTERFACE_DEFINITION);
	class_definition->access_modifier = DVM_FILE_ACCESS;
	if (modifier) {
		if (modifier->is_abstract == ABSTRACT_MODIFIER) {
			class_definition->is_abstract = DVM_TRUE;
		}
		class_definition->access_modifier = conv_access_modifier(modifier->access_modifier);
	}

	class_definition->class_or_interface = class_or_interface;
	class_definition->package_name = compiler->package_name;
	class_definition->name = identifier;
	class_definition->extends = extends;
	class_definition->super_class = NULL;
	class_definition->interface_list = NULL;
	class_definition->member = NULL;
	class_definition->next = NULL;
	class_definition->line_number = compiler->current_line_number;

	DBG_assert(compiler->current_class_definition == NULL, ("current_class_definition is not NULL."));

	compiler->current_class_definition = class_definition;
}

void dkc_class_define(MemberDeclaration *member_list) {
	DKC_Compiler *compiler = dkc_get_current_compiler();
	ClassDefinition *class_definition = compiler->current_class_definition;

	DBG_assert(class_definition != NULL, ("current_class_definition is NULL."));

	if (compiler->class_definition_list == NULL) {
		compiler->class_definition_list = class_definition;
	} else {
		ClassDefinition *pos;
		for (pos = compiler->class_definition_list; pos->next; pos = pos->next);
		pos->next = class_definition;
	}
	class_definition->member = member_list;
	compiler->current_class_definition = NULL;
}

Declaration *dkc_alloc_declaration(TypeSpecifier *type, char *identifier) {
	Declaration *declaration = dkc_malloc(sizeof(Declaration));
	declaration->name = identifier;
	declaration->type = type;
	declaration->variable_index = -1;

	return declaration;
}

DeclarationList *dkc_chain_declaration(DeclarationList *list,
                                       Declaration *declaration) {
	DeclarationList *new_item = dkc_malloc(sizeof(DeclarationList));
	new_item->declaration = declaration;
	new_item->next = NULL;

	if (list == NULL) {
		return new_item;
	}

	DeclarationList *pos;

	for (pos = list; pos->next; pos = pos->next);

	pos->next = new_item;

	return list;
}

Statement *dkc_create_declaration_statement(TypeSpecifier *type, char *identifier, Expression *initializer) {
	Statement *statement = alloc_statement(DECLARATION_STATEMENT);
	Declaration *declaration = dkc_alloc_declaration(type, identifier);
	declaration->initializer = initializer;

	statement->u.declaration_s = declaration;

	return statement;
}

static MemberDeclaration *alloc_member_declaration(MemberKind kind, ClassOrMemberModifierList *modifier) {
	MemberDeclaration *member_declaration = dkc_malloc(sizeof(MemberDeclaration));
	member_declaration->kind = kind;
	if (modifier) {
		member_declaration->access_modifier = conv_access_modifier(modifier->access_modifier);
	} else {
		member_declaration->access_modifier = DVM_FILE_ACCESS;
	}
	member_declaration->line_number = dkc_get_current_compiler()->current_line_number;
	member_declaration->next = NULL;

	return member_declaration;
}

MemberDeclaration *dkc_create_field_member(ClassOrMemberModifierList *modifier, TypeSpecifier *type, char *name) {
	MemberDeclaration *member_declaration = alloc_member_declaration(FIELD_MEMBER, modifier);
	member_declaration->u.field.name = name;
	member_declaration->u.field.type = type;

	return member_declaration;
}

ClassOrMemberModifierList dkc_create_class_or_member_modifier(ClassOrMemberModifierKind modifier) {
	ClassOrMemberModifierList ret;

	ret.is_abstract = NOT_SPECIFIED_MODIFIER;
	ret.is_override = NOT_SPECIFIED_MODIFIER;
	ret.is_virtual = NOT_SPECIFIED_MODIFIER;
	ret.access_modifier = NOT_SPECIFIED_MODIFIER;

	switch (modifier) {
		case ABSTRACT_MODIFIER:
			ret.is_abstract = ABSTRACT_MODIFIER;
			break;
		case PUBLIC_MODIFIER:
			ret.access_modifier = PUBLIC_MODIFIER;
			break;
		case PRIVATE_MODIFIER:
			ret.access_modifier = PRIVATE_MODIFIER;
			break;
		case OVERRIDE_MODIFIER:
			ret.is_override = OVERRIDE_MODIFIER;
			break;
		case VIRTUAL_MODIFIER:
			ret.is_virtual = VIRTUAL_MODIFIER;
			break;
		case NOT_SPECIFIED_MODIFIER:
			DBG_assert(0, ("modifier..%d", modifier));
	}

	return ret;
}

ClassOrMemberModifierList dkc_chain_class_or_member_modifier(ClassOrMemberModifierList list,
                                                             ClassOrMemberModifierList add) {
	if (add.is_abstract != NOT_SPECIFIED_MODIFIER) {
		DBG_assert(add.is_abstract == ABSTRACT_MODIFIER, ("add.is_abstract..%d", add.is_abstract));
		if (list.is_abstract != NOT_SPECIFIED_MODIFIER) {
			dkc_compile_error(dkc_get_current_compiler()->current_line_number, ABSTRACT_MULTIPLE_SPECIFIED_ERR,
			                  MESSAGE_ARGUMENT_END);
		}
		list.is_abstract = ABSTRACT_MODIFIER;
	} else if (add.access_modifier != NOT_SPECIFIED_MODIFIER) {
		DBG_assert(add.access_modifier == PUBLIC_MODIFIER || add.access_modifier == PRIVATE_MODIFIER,
		           ("add.access_modifier..", add.access_modifier));
		if (list.access_modifier != NOT_SPECIFIED_MODIFIER) {
			dkc_compile_error(dkc_get_current_compiler()->current_line_number, ACCESS_MODIFIER_MULTIPLE_SPECIFIED_ERR,
			                  MESSAGE_ARGUMENT_END);
		}
		list.access_modifier = add.access_modifier;
	} else if (add.is_override != NOT_SPECIFIED_MODIFIER) {
		DBG_assert(add.is_override == OVERRIDE_MODIFIER, ("add.is_override..", add.is_override));
		if (list.is_override != NOT_SPECIFIED_MODIFIER) {
			dkc_compile_error(dkc_get_current_compiler()->current_line_number, OVERRIDE_MODIFIER_MULTIPLE_SPECIFIED_ERR,
			                  MESSAGE_ARGUMENT_END);
		}
		list.is_override = add.is_override;
	} else if (add.is_virtual != NOT_SPECIFIED_MODIFIER) {
		DBG_assert(add.is_virtual == VIRTUAL_MODIFIER, ("add.is_virtual..", add.access_modifier));
		if (list.is_virtual != NOT_SPECIFIED_MODIFIER) {
			dkc_compile_error(dkc_get_current_compiler()->current_line_number, VIRTUAL_MODIFIER_MULTIPLE_SPECIFIED_ERR,
			                  MESSAGE_ARGUMENT_END);
		}
		list.is_virtual = add.is_virtual;
	}

	return list;
}


MemberDeclaration *dkc_create_method_member(ClassOrMemberModifierList *modifier,
                                            FunctionDefinition *function_definition, DVM_Boolean is_constructor) {
	MemberDeclaration *ret = alloc_member_declaration(METHOD_MEMBER, modifier);
	ret->u.method.is_constructor = is_constructor;
	ret->u.method.is_abstract = DVM_FALSE;
	ret->u.method.is_virtual = DVM_FALSE;
	ret->u.method.is_override = DVM_FALSE;

	if (modifier) {
		if (modifier->is_abstract == ABSTRACT_MODIFIER) {
			ret->u.method.is_abstract = DVM_TRUE;
		}
		if (modifier->is_virtual == VIRTUAL_MODIFIER) {
			ret->u.method.is_virtual = DVM_TRUE;
		}
		if (modifier->is_override == OVERRIDE_MODIFIER) {
			ret->u.method.is_override = DVM_TRUE;
		}
	}

	DKC_Compiler *compiler = dkc_get_current_compiler();
	if (compiler->current_class_definition->class_or_interface ==DVM_INTERFACE_DEFINITION) {
		ret->u.method.is_abstract = DVM_TRUE;
		ret->u.method.is_virtual = DVM_TRUE;
	}

	ret->u.method.function_definition = function_definition;

	if (ret->u.method.is_abstract) {
		if (function_definition->block) {
			dkc_compile_error(compiler->current_line_number, ABSTRACT_METHOD_HAS_BODY_ERR, MESSAGE_ARGUMENT_END);
		}
	} else {
		if (function_definition->block == NULL) {
			dkc_compile_error(compiler->current_line_number, CONCRETE_METHOD_HAS_NO_BODY_ERR, MESSAGE_ARGUMENT_END);
		}
	}

	function_definition->class_definition = compiler->current_class_definition;

	return ret;
}

FunctionDefinition *dkc_method_function_define(TypeSpecifier *type, char *identifier, ParameterList *parameter_list,
                                               Block *block) {
	return dkc_create_function_definition(type, identifier, parameter_list, block);
}

FunctionDefinition *dkc_constructor_function_define(char *identifier, ParameterList *parameter_list, Block *block) {
	TypeSpecifier *type = dkc_create_type_specifier(DVM_VOID_TYPE);
	FunctionDefinition *function_definition = dkc_method_function_define(type, identifier, parameter_list, block);

	return function_definition;
}

MemberDeclaration *dkc_chain_member_declaration(MemberDeclaration *list, MemberDeclaration *add) {
	MemberDeclaration *pos;
	for (pos = list; pos->next; pos = pos->next) ;
	pos->next = add;

	return list;
}

ExtendsList *dkc_create_extends_list(char *identifier) {
	ExtendsList *list = dkc_malloc(sizeof(ExtendsList));
	list->identifier = identifier;
	list->class_definition = NULL;
	list->next = NULL;

	return list;
}

ExtendsList *dkc_chain_extends_list(ExtendsList *list, char *add) {
	ExtendsList *pos;
	for (pos = list; pos->next; pos = pos->next);

	pos->next = dkc_create_extends_list(add);

	return list;
}
