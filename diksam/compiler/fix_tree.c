//
// Created by sulvto on 18-6-15.
//
#include <string.h>
#include <DVM_code.h>
#include <stdbool.h>
#include "DBG.h"
#include "MEM.h"
#include "diksamc.h"

static Expression *fix_expression(Block *current_block, Expression *expression, Expression *parent);

static void fix_type_specifier(TypeSpecifier *type);

static void fix_statement_list(Block *current_block, StatementList *list, FunctionDefinition *function_definition) ;

static int add_class(ClassDefinition *src) ;

static void fix_return_statement(Block *current_block, Statement *statement, FunctionDefinition *function_definition) ;

static TypeSpecifier *create_function_derive_type(FunctionDefinition *function_definition) ;


static Expression *chain_string(Expression *pTag);

static int reserve_function_index(DKC_Compiler *compiler, FunctionDefinition *src) {
	if (src->class_definition && src->block == NULL) {
		return ABSTRACT_METHOD_INDEX;
	}

	char *src_package_name = dkc_package_name_to_string(src->package_name);
	for (int i = 0; i < compiler->dvm_function_count; i++) {
		if (dvm_compare_package_name(src_package_name, compiler->dvm_function[i].package_name)
		    && 0 == strcmp(src->name, compiler->dvm_function[i].name)) {
			MEM_free(src_package_name);
			return i;
		}
	}

	compiler->dvm_function = MEM_realloc(compiler->dvm_function,
	                                     sizeof(DVM_Function) * (compiler->dvm_function_count + 1));
	DVM_Function *dest = &compiler->dvm_function[compiler->dvm_function_count];
	compiler->dvm_function_count++;
	dest->package_name = src_package_name;
	if (src->class_definition) {
		dest->name = dvm_create_method_function_name(src->class_definition->name, src->name);
	} else {
		dest->name = MEM_strdup(src->name);
	}

	return compiler->dvm_function_count - 1;
}

static void add_return_function(FunctionDefinition *function_definition) {
	StatementList *last;
	StatementList **last_p;
	if (function_definition->block->statement_list == NULL) {
		last_p = &function_definition->block->statement_list;
	} else {
		for (last = function_definition->block->statement_list; last->next; last = last->next) ;
		if (last->statement->type == RETURN_STATEMENT) {
			return;
		}
		last_p = &last->next;
	}

	Statement *ret = dkc_create_return_statement(NULL);
	ret->line_number = function_definition->end_line_number;
	if (ret->u.return_s.return_value) {
		ret->u.return_s.return_value->line_number = function_definition->end_line_number;
	}
	fix_return_statement(function_definition->block, ret, function_definition);
	*last_p = dkc_create_statement_list(ret);
}

static void add_local_variable(FunctionDefinition *function_definition, Declaration *declaration, DVM_Boolean is_parameter) {
	function_definition->local_variable = MEM_realloc(function_definition->local_variable, sizeof(Declaration *) *
	                                                                                       (function_definition->local_variable_count +
	                                                                                        1));
	function_definition->local_variable[function_definition->local_variable_count] = declaration;
	if (function_definition->class_definition && !is_parameter) {
		declaration->variable_index = function_definition->local_variable_count + 1;
	} else {
		declaration->variable_index = function_definition->local_variable_count;
	}

	function_definition->local_variable_count++;
}

static void add_declaration(Block *current_block, Declaration *declaration, FunctionDefinition *function_definition, int line_number, DVM_Boolean is_parameter) {
	if (dkc_search_declaration(declaration->name, current_block)) {
		dkc_compile_error(line_number, VARIABLE_MULTIPLE_DEFINE_ERR,
		                  STRING_MESSAGE_ARGUMENT, "name", declaration->name,
		                  MESSAGE_ARGUMENT_END);
	}

	if (current_block) {
		current_block->declaration_list = dkc_chain_declaration(current_block->declaration_list, declaration);
	}

	if (function_definition) {
		declaration->is_local = DVM_TRUE;
		add_local_variable(function_definition, declaration, is_parameter);
	} else {
		DKC_Compiler *compiler = dkc_get_current_compiler();
		declaration->is_local = DVM_FALSE;
		compiler->declaration_list = dkc_chain_declaration(compiler->declaration_list, declaration);
	}
}

static void add_parameter_as_declaration(FunctionDefinition *function_definition) {
	Declaration *declaration;
	for (ParameterList *parameter = function_definition->parameter; parameter; parameter = parameter->next) {
		if (dkc_search_declaration(parameter->name, function_definition->block)) {
			dkc_compile_error(parameter->line_number, PARAMETER_MULTIPLE_DEFINE_ERR,
			                  STRING_MESSAGE_ARGUMENT, "name", parameter->name,
			                  MESSAGE_ARGUMENT_END);
		}
		fix_type_specifier(parameter->type);
		declaration = dkc_alloc_declaration(parameter->type, parameter->name);
		if (function_definition == NULL || function_definition->block) {
			add_declaration(function_definition->block, declaration, function_definition, parameter->line_number,
			                DVM_TRUE);
		}
	}
}

static void fix_function(FunctionDefinition *function_definition) {
	add_parameter_as_declaration(function_definition);
	fix_type_specifier(function_definition->type);
	if (function_definition->block) {
		fix_statement_list(function_definition->block, function_definition->block->statement_list, function_definition);
		add_return_function(function_definition);
	}
}

static Expression *fix_identifier_expression(Block *current_block, Expression *expression) {
	DKC_Compiler *compiler = dkc_get_current_compiler();

	Declaration *declaration = dkc_search_declaration(expression->u.identifier.name, current_block);

	if (declaration) {
		expression->type = declaration->type;
		expression->u.identifier.kind = VARIABLE_IDENTIFIER;
		expression->u.identifier.u.declaration = declaration;
		return expression;
	}
	FunctionDefinition *function_definition = dkc_search_function(expression->u.identifier.name);
	if (function_definition == NULL) {
		dkc_compile_error(expression->line_number, IDENTIFIER_NOT_FOUND_ERR, STRING_MESSAGE_ARGUMENT, "name",
		                  expression->u.identifier.name, MESSAGE_ARGUMENT_END);
	}

	expression->type = create_function_derive_type(function_definition);
	expression->u.identifier.kind = FUNCTION_IDENTIFIER;
	expression->u.identifier.u.function.function_definition = function_definition;
	expression->u.identifier.u.function.function_index = reserve_function_index(compiler, function_definition);

	fix_type_specifier(expression->type);

	return expression;
}

static TypeSpecifier *create_function_derive_type(FunctionDefinition *function_definition) {
	TypeSpecifier *ret = dkc_alloc_type_specifier(function_definition->type->basic_type);
	*ret = *function_definition->type;
	ret->derive = dkc_alloc_type_derive(FUNCTION_DERIVE);
	ret->derive->u.function_derive.parameter_list = function_definition->parameter;
	ret->derive->next = function_definition->type->derive;

	return ret;
}

static ClassDefinition *search_class_and_add(int line_number, char *name, int *class_index_p) {
	ClassDefinition *class_definition = dkc_search_class(name);
	if (class_definition == NULL) {
		dkc_compile_error(line_number, CLASS_NOT_FOUND_ERR,
		                  STRING_MESSAGE_ARGUMENT, "name", name,
		                  MESSAGE_ARGUMENT_END);
	}
	*class_index_p = add_class(class_definition);

	return class_definition;
}

static int add_class(ClassDefinition *src) {
	char *src_package_name = dkc_package_name_to_string(src->package_name);
	DKC_Compiler *compiler = dkc_get_current_compiler();

	for (int i = 0; i < compiler->dvm_class_count; i++) {
		if (dvm_compare_package_name(src_package_name, compiler->dvm_class[i].package_name) &&
		    !strcmp(src->name, compiler->dvm_class[i].name)) {
			MEM_free(src_package_name);
			return i;
		}
	}

	compiler->dvm_class = MEM_realloc(compiler->dvm_class, sizeof(DVM_Class) * (compiler->dvm_class_count));

	DVM_Class *dest = &compiler->dvm_class[compiler->dvm_class_count];
	int result = compiler->dvm_class_count;
	compiler->dvm_class_count++;

	dest->package_name = src_package_name;
	dest->name = MEM_strdup(src->name);
	dest->is_implemented = DVM_FALSE;

	for (ExtendsList *pos = src->extends; pos; pos = pos->next) {
		int dummy;
		search_class_and_add(src->line_number, pos->identifier, &dummy);
	}

	return result;
}

static Expression *fix_comma_expression(Block *current_block, Expression *expression) {
	expression->u.comma.left = fix_expression(current_block, expression->u.comma.left, expression);
	expression->u.comma.right = fix_expression(current_block, expression->u.comma.right, expression);
	expression->type = expression->u.comma.right->type;
	return expression;
}

static void fix_parameter_list(ParameterList *parameter_list) {
	for (ParameterList *parameter = parameter_list; parameter; parameter = parameter->next) {
		fix_type_specifier(parameter->type);
	}
}

static void fix_type_specifier(TypeSpecifier *type) {
	DKC_Compiler *compiler = dkc_get_current_compiler();

	for (TypeDerive *pos = type->derive; pos; pos = pos->next) {
		if (pos->tag == FUNCTION_DERIVE) {
			fix_parameter_list(pos->u.function_derive.parameter_list);
		}
	}

	if (type->basic_type ==DVM_CLASS_TYPE && type->class_ref.class_definition == NULL) {
		ClassDefinition *class_definition = dkc_search_class(type->class_ref.identifier);
		if (class_definition) {
			if (!dkc_compare_package_name(class_definition->package_name, compiler->package_name) &&
			    class_definition->access_modifier != DVM_PUBLIC_ACCESS) {
				dkc_compile_error(type->line_number, PACKAGE_CLASS_ACCESS_ERR,
				                  STRING_MESSAGE_ARGUMENT, "class_name",
				                  class_definition->name,
				                  MESSAGE_ARGUMENT_END);
			}
			type->class_ref.class_definition = class_definition;
			type->class_ref.class_index = add_class(class_definition);
			return;
		}
		dkc_compile_error(type->line_number, TYPE_NAME_NOT_FOUND_ERR,
		                  STRING_MESSAGE_ARGUMENT, "name",
		                  type->class_ref.identifier,
		                  MESSAGE_ARGUMENT_END);
	}
}

static Expression *create_up_cast(Expression *src, ClassDefinition *dest_interface, int interface_index) {


	TypeSpecifier *type = dkc_alloc_type_specifier(DVM_CLASS_TYPE);
	type->class_ref.identifier = dest_interface->name;
	type->class_ref.class_definition = dest_interface;
	type->class_ref.class_index = interface_index;

	Expression *cast_expression = dkc_alloc_expression(UP_CAST_EXPRESSION);
	cast_expression->type = type;
	cast_expression->u.up_cast.interface_definition = dest_interface;
	cast_expression->u.up_cast.operand = src;
	cast_expression->u.up_cast.interface_index = interface_index;

	return cast_expression;
}

static DVM_Boolean is_super_interface(ClassDefinition *child, ClassDefinition *parent, int *interface_index_out) {
	int interface_index = 0;
	for (ExtendsList *pos = child->interface_list; pos; pos = pos->next) {
		if (pos->class_definition == parent) {
			*interface_index_out = interface_index;
			return DVM_TRUE;
		}
		interface_index++;
	}
	return DVM_FALSE;
}

static DVM_Boolean is_super_class(ClassDefinition *child, ClassDefinition *parent, DVM_Boolean *is_interface, int *interface_index) {
	for (ClassDefinition *pos = child->super_class; pos; pos = pos->super_class) {
		if (pos == parent) {
			*is_interface = DVM_FALSE;
			return DVM_TRUE;
		}
	}

	*is_interface = DVM_TRUE;

	return is_super_interface(child, parent, interface_index);
}

static Expression *alloc_cast_expression(CastType cast_type, Expression *operand) {
	Expression *cast_expression = dkc_alloc_expression(CAST_EXPRESSION);
	cast_expression->line_number = operand->line_number;
	cast_expression->u.cast.type = cast_type;
	cast_expression->u.cast.operand = operand;

	if (cast_type == INT_TO_DOUBLE_CAST) {
		cast_expression->type = dkc_alloc_type_specifier(DVM_DOUBLE_TYPE);
	} else if (cast_type == DOUBLE_TO_INT_CAST) {
		cast_expression->type = dkc_alloc_type_specifier(DVM_INT_TYPE);
	} else if (cast_type == BOOLEAN_TO_STRING_CAST
	           || cast_type == INT_TO_STRING_CAST
	           || cast_type == DOUBLE_TO_STRING_CAST) {
		cast_expression->type = dkc_alloc_type_specifier(DVM_STRING_TYPE);
	} else {
		DBG_panic(("cast_type..%d\n", cast_type));
	}

	return cast_expression;
}

static Expression *create_to_string_cast(Expression *src) {
	Expression *cast = NULL;

	if (dkc_is_boolean(src->type)) {
		cast = alloc_cast_expression(BOOLEAN_TO_STRING_CAST, src);
	} else if (dkc_is_int(src->type)) {
		cast = alloc_cast_expression(INT_TO_STRING_CAST, src);
	} else if (dkc_is_double(src->type)) {
		cast = alloc_cast_expression(DOUBLE_TO_STRING_CAST, src);
	}

	return cast;
}

static void cast_mismatch_error(int line_number, TypeSpecifier *src, TypeSpecifier *dest) {
	char *tmp = dkc_get_type_name(src);
	char *src_name = dkc_strdup(tmp);
	MEM_free(tmp);

	tmp = dkc_get_type_name(dest);
	char *dest_name = dkc_strdup(tmp);
	MEM_free(tmp);


	dkc_compile_error(line_number, CAST_MISMATCH_ERR,
	                  STRING_MESSAGE_ARGUMENT, "src", src_name,
	                  STRING_MESSAGE_ARGUMENT, "dest", dest_name,
	                  MESSAGE_ARGUMENT_END);
}

static Expression *create_assign_cast(Expression *src, TypeSpecifier *dest) {
	Expression *cast_expression;
	if (dkc_compare_type(src->type, dest)) {
		return src;
	}

	if (dkc_is_object(dest) && src->type->basic_type == DVM_NULL_TYPE) {
		DBG_assert(src->type->derive == NULL, ("derive != NULL"));
		return src;
	}

	if (dkc_is_class_object(src->type) && dkc_is_class_object(dest)) {
		DVM_Boolean is_interface;
		int interface_index;
		if (is_super_class(src->type->class_ref.class_definition, dest->class_ref.class_definition, &is_interface,
		                   &interface_index)) {
			if (is_interface) {
				cast_expression = create_up_cast(src, dest->class_ref.class_definition, interface_index);
				return cast_expression;
			}
			return src;
		} else {
			cast_mismatch_error(src->line_number, src->type, dest);
		}
	}

	if (dkc_is_int(src->type) && dkc_is_double(dest)) {
		cast_expression = alloc_cast_expression(INT_TO_DOUBLE_CAST, src);
		return cast_expression;
	} else if (dkc_is_double(src->type) && dkc_is_int(dest)) {
		cast_expression = alloc_cast_expression(DOUBLE_TO_INT_CAST, src);
		return cast_expression;
	} else if (dkc_is_string(dest)) {
		cast_expression = create_to_string_cast(src);

		if (cast_expression) {
			return cast_expression;
		}
	}

	cast_mismatch_error(src->line_number, src->type, dest);

	return NULL;
}

static Expression *fix_assign_expression(Block *current_block, Expression *expression) {
	if (expression->u.assign_expression.left->kind != IDENTIFIER_EXPRESSION
	    && expression->u.assign_expression.left->kind != INDEX_EXPRESSION
	    && expression->u.assign_expression.left->kind != MEMBER_EXPRESSION) {
		dkc_compile_error(expression->u.assign_expression.left->line_number, NOT_LVALUE_ERR, MESSAGE_ARGUMENT_END);
	}

	expression->u.assign_expression.left = fix_expression(current_block, expression->u.assign_expression.left,
	                                                      expression);
	Expression *left = expression->u.assign_expression.left;
	Expression *operand = fix_expression(current_block, expression->u.assign_expression.operand, expression);
	expression->u.assign_expression.operand = create_assign_cast(operand, expression->u.assign_expression.left->type);
	expression->type = left->type;

	return expression;
}

static Expression *eval_math_expression_int(Expression *expression, int left, int right) {
	if (expression->kind == ADD_EXPRESSION) {
		expression->u.int_value = left + right;
	} else if (expression->kind == SUB_EXPRESSION) {
		expression->u.int_value = left - right;
	} else if (expression->kind == MUL_EXPRESSION) {
		expression->u.int_value = left * right;
	} else if (expression->kind == DIV_EXPRESSION) {
		if (right == 0) {
			dkc_compile_error(expression->line_number, DIVISION_BY_ZERO_IN_COMPILE_ERR, MESSAGE_ARGUMENT_END);
		}
		expression->u.int_value = left / right;
	} else if (expression->kind == MOD_EXPRESSION) {
		expression->u.int_value = left % right;
	} else {
		DBG_assert(0, ("expression->kind..%d\n", expression->kind));
	}

	expression->kind = INT_EXPRESSION;
	expression->type = dkc_alloc_type_specifier(DVM_INT_TYPE);

	return expression;
}

static Expression *eval_math_expression_double(Expression *expression, double left, double right) {
	if (expression->kind == ADD_EXPRESSION) {
		expression->u.double_value = left + right;
	} else if (expression->kind == SUB_EXPRESSION) {
		expression->u.double_value = left - right;
	} else if (expression->kind == MUL_EXPRESSION) {
		expression->u.double_value = left * right;
	} else if (expression->kind == DIV_EXPRESSION) {
		expression->u.double_value = left / right;
	} else if (expression->kind == MOD_EXPRESSION) {
		expression->u.double_value = fmod(left , right);
	} else {
		DBG_assert(0, ("expression->kind..%d\n", expression->kind));
	}

	expression->kind = DOUBLE_EXPRESSION;
	expression->type = dkc_alloc_type_specifier(DVM_DOUBLE_TYPE);

	return expression;
}

static Expression *chain_string(Expression *expression) {
	DVM_Char *left_str = expression->u.binary_expression.left->u.string_value;
	DVM_Char *right_str = dkc_expression_to_string(expression->u.binary_expression.right);
	if (!right_str) {
		return expression;
	}
	int len = dvm_wcslen(left_str) + dvm_wcslen(right_str);
	DVM_Char *new_str = MEM_malloc(sizeof(DVM_Char) * len + 1);
	dvm_wcscpy(new_str, left_str);
	dvm_wcscpy(new_str, right_str);
	MEM_free(left_str);
	MEM_free(right_str);

	expression->kind = STRING_EXPRESSION;
	expression->type = dkc_alloc_type_specifier(DVM_STRING_TYPE);
	expression->u.string_value = new_str;

	return expression;
}

static Expression *eval_math_expression(Block *current_block, Expression *expression) {
	if (expression->u.binary_expression.left->kind == INT_EXPRESSION &&
	    expression->u.binary_expression.right->kind == INT_EXPRESSION) {
		expression = eval_math_expression_int(expression, expression->u.binary_expression.left->u.int_value,
		                                      expression->u.binary_expression.right->u.int_value);
	} else if (expression->u.binary_expression.left->kind == DOUBLE_EXPRESSION &&
	           expression->u.binary_expression.right->kind == DOUBLE_EXPRESSION) {
		expression = eval_math_expression_double(expression, expression->u.binary_expression.left->u.double_value,
		                                         expression->u.binary_expression.right->u.double_value);
	} else if (expression->u.binary_expression.left->kind == DOUBLE_EXPRESSION &&
	           expression->u.binary_expression.right->kind == INT_EXPRESSION) {
		expression = eval_math_expression_double(expression, expression->u.binary_expression.left->u.double_value,
		                                         expression->u.binary_expression.right->u.int_value);
	} else if (expression->u.binary_expression.left->kind == INT_EXPRESSION &&
	           expression->u.binary_expression.right->kind == DOUBLE_EXPRESSION) {
		expression = eval_math_expression_double(expression, expression->u.binary_expression.left->u.int_value,
		                                         expression->u.binary_expression.right->u.double_value);
	} else if (expression->u.binary_expression.left->kind == STRING_EXPRESSION
	           && expression->kind == ADD_EXPRESSION) {
		expression = chain_string(expression);
	}

	return expression;
}

static Expression *cast_binary_expression(Expression *expression) {
	Expression *cast_expression = NULL;
	if (dkc_is_int(expression->u.binary_expression.left->type)
			&& dkc_is_double(expression->u.binary_expression.right->type)) {
		cast_expression = alloc_cast_expression(INT_TO_DOUBLE_CAST,
		                                        expression->u.binary_expression.left);
		expression->u.binary_expression.left = cast_expression;
	} else if(dkc_is_double(expression->u.binary_expression.left->type)
	          && dkc_is_int(expression->u.binary_expression.right->type)) {
		cast_expression = alloc_cast_expression(DOUBLE_TO_INT_CAST,
		                                        expression->u.binary_expression.left);
		expression->u.binary_expression.left = cast_expression;
	}
	if (cast_expression) {
		return expression;
	}

	if (dkc_is_string(expression->u.binary_expression.left->type)&& expression->kind == ADD_EXPRESSION) {
		cast_expression = create_to_string_cast(expression->u.binary_expression.right);

		if (cast_expression) {
			expression->u.binary_expression.right = cast_expression;
		}
	}

	return expression;
}

static Expression *fix_math_binary_expression(Block *current_block, Expression *expression) {
	expression->u.binary_expression.left = fix_expression(current_block, expression->u.binary_expression.left,
	                                                      expression);
	expression->u.binary_expression.right = fix_expression(current_block, expression->u.binary_expression.right,
	                                                       expression);
	expression = eval_math_expression(current_block, expression);
	if (expression->kind == INT_EXPRESSION || expression->kind == DOUBLE_EXPRESSION ||
	    expression->kind == STRING_EXPRESSION) {
		return expression;
	}
	expression = cast_binary_expression(expression);

	if (dkc_is_int(expression->u.binary_expression.left->type) &&
	    dkc_is_int(expression->u.binary_expression.right->type)) {
		expression->type = dkc_alloc_type_specifier(DVM_INT_TYPE);
	} else if (dkc_is_double(expression->u.binary_expression.left->type) &&
	           dkc_is_double(expression->u.binary_expression.right->type)) {
		expression->type = dkc_alloc_type_specifier(DVM_DOUBLE_TYPE);
	} else if (expression->kind == ADD_EXPRESSION && ((dkc_is_string(expression->u.binary_expression.left->type) &&
	                                                   dkc_is_string(expression->u.binary_expression.right->type)) ||
	                                                  (dkc_is_string(expression->u.binary_expression.left->type) &&
	                                                   expression->u.binary_expression.right->kind ==
	                                                   NULL_EXPRESSION))) {
		expression->type = dkc_alloc_type_specifier(DVM_STRING_TYPE);
	} else {
		dkc_compile_error(expression->line_number, MATH_TYPE_MISMATCH_ERR, MESSAGE_ARGUMENT_END);
	}

	return expression;
}

static Expression *eval_compare_expression_boolean(Expression *expression, DVM_Boolean left, DVM_Boolean right) {
	if (expression->kind == EQ_EXPRESSION) {
		expression->u.boolean_value = (left == right);
	} else if (expression->kind == NE_EXPRESSION) {
		expression->u.boolean_value = (left != right);
	} else {
		DBG_assert(0, ("expression->kind..%d\n", expression->kind));
	}
	expression->kind = BOOLEAN_EXPRESSION;
	expression->type = dkc_alloc_type_specifier(DVM_BOOLEAN_TYPE);

	return expression;
}

static Expression *eval_compare_expression_int(Expression *expression, int left, int right) {
	if (expression->kind == EQ_EXPRESSION) {
		expression->u.boolean_value = (left == right);
	} else if (expression->kind == NE_EXPRESSION) {
		expression->u.boolean_value = (left != right);
	} else if (expression->kind == GT_EXPRESSION) {
		expression->u.boolean_value = (left > right);
	} else if (expression->kind == GE_EXPRESSION) {
		expression->u.boolean_value = (left >= right);
	} else if (expression->kind == LT_EXPRESSION) {
		expression->u.boolean_value = (left < right);
	} else if (expression->kind == LE_EXPRESSION) {
		expression->u.boolean_value = (left <= right);
	} else {
		DBG_assert(0, ("expression->kind..%d\n", expression->kind));
	}

	expression->type = dkc_alloc_type_specifier(DVM_BOOLEAN_TYPE);
	expression->kind = BOOLEAN_EXPRESSION;
	return expression;
}

static Expression *eval_compare_expression_double(Expression *expression, int left, int right) {
	if (expression->kind == EQ_EXPRESSION) {
		expression->u.boolean_value = (left == right);
	} else if (expression->kind == NE_EXPRESSION) {
		expression->u.boolean_value = (left != right);
	} else if (expression->kind == GT_EXPRESSION) {
		expression->u.boolean_value = (left > right);
	} else if (expression->kind == GE_EXPRESSION) {
		expression->u.boolean_value = (left >= right);
	} else if (expression->kind == LT_EXPRESSION) {
		expression->u.boolean_value = (left < right);
	} else if (expression->kind == LE_EXPRESSION) {
		expression->u.boolean_value = (left <= right);
	} else {
		DBG_assert(0, ("expression->kind..%d\n", expression->kind));
	}

	expression->type = dkc_alloc_type_specifier(DVM_BOOLEAN_TYPE);
	expression->kind = BOOLEAN_EXPRESSION;
	return expression;
}

static Expression *eval_compare_expression_string(Expression *expression, DVM_Char *left, DVM_Char *right) {
	int cmp = dvm_wcscmp(left, right);
	if (expression->kind == EQ_EXPRESSION) {
		expression->u.boolean_value = (cmp == 0);
	} else if (expression->kind == NE_EXPRESSION) {
		expression->u.boolean_value = (cmp != 0);
	} else if (expression->kind == GT_EXPRESSION) {
		expression->u.boolean_value = (cmp > 0);
	} else if (expression->kind == GE_EXPRESSION) {
		expression->u.boolean_value = (cmp >= 0);
	} else if (expression->kind == LT_EXPRESSION) {
		expression->u.boolean_value = (cmp < 0);
	} else if (expression->kind == LE_EXPRESSION) {
		expression->u.boolean_value = (cmp <= 0);
	} else {
		DBG_assert(0, ("expression->kind..%d\n", expression->kind));
	}

	MEM_free(left);
	MEM_free(right);

	expression->kind = BOOLEAN_EXPRESSION;
	expression->type = dkc_alloc_type_specifier(DVM_BOOLEAN_TYPE);

	return expression;
}

static Expression *eval_compare_expression(Expression *expression) {
	if (expression->u.binary_expression.left->kind == BOOLEAN_EXPRESSION &&
	    expression->u.binary_expression.right->kind == BOOLEAN_EXPRESSION) {
		expression = eval_compare_expression_boolean(expression, expression->u.binary_expression.left->u.boolean_value,
		                                             expression->u.binary_expression.right->u.boolean_value);
	} else if (expression->u.binary_expression.left->kind == INT_EXPRESSION &&
	           expression->u.binary_expression.right->kind == INT_EXPRESSION) {
		expression = eval_compare_expression_int(expression, expression->u.binary_expression.left->u.int_value,
		                                         expression->u.binary_expression.right->u.int_value);
	} else if (expression->u.binary_expression.left->kind == DOUBLE_EXPRESSION &&
	           expression->u.binary_expression.right->kind == DOUBLE_EXPRESSION) {
		expression = eval_compare_expression_double(expression, expression->u.binary_expression.left->u.double_value,
		                                            expression->u.binary_expression.right->u.double_value);
	} else if (expression->u.binary_expression.left->kind == DOUBLE_EXPRESSION &&
	           expression->u.binary_expression.right->kind == INT_EXPRESSION) {
		expression = eval_compare_expression_double(expression, expression->u.binary_expression.left->u.double_value,
		                                            expression->u.binary_expression.right->u.int_value);
	} else if (expression->u.binary_expression.left->kind == INT_EXPRESSION &&
	           expression->u.binary_expression.right->kind == DOUBLE_EXPRESSION) {
		expression = eval_compare_expression_double(expression, expression->u.binary_expression.left->u.int_value,
		                                            expression->u.binary_expression.right->u.double_value);
	} else if (expression->u.binary_expression.left->kind == STRING_EXPRESSION &&
	           expression->u.binary_expression.right->kind == STRING_EXPRESSION) {
		expression = eval_compare_expression_string(expression, expression->u.binary_expression.left->u.string_value,
		                                            expression->u.binary_expression.right->u.string_value);
	} else if (expression->u.binary_expression.left->kind == NULL_EXPRESSION &&
	           expression->u.binary_expression.right->kind == NULL_EXPRESSION) {
		expression->kind = BOOLEAN_EXPRESSION;
		expression->type = dkc_alloc_type_specifier(DVM_BOOLEAN_TYPE);
		expression->u.boolean_value = DVM_TRUE;
	}
	return expression;
}

static Expression *fix_compare_expression(Block *current_block, Expression *expression) {
	expression->u.binary_expression.left = fix_expression(current_block, expression->u.binary_expression.left,
	                                                      expression);
	expression->u.binary_expression.right = fix_expression(current_block, expression->u.binary_expression.right,
	                                                       expression);
	expression = eval_compare_expression(expression);
	if (expression->kind == BOOLEAN_EXPRESSION) {
		return expression;
	}

	expression = cast_binary_expression(expression);

	if (!(dkc_compare_type(expression->u.binary_expression.left->type, expression->u.binary_expression.right->type)) ||
	    (dkc_is_object(expression->u.binary_expression.left->type) &&
	     expression->u.binary_expression.right->kind == NULL_EXPRESSION) ||
	    (dkc_is_object(expression->u.binary_expression.right->type) &&
	     expression->u.binary_expression.left->kind == NULL_EXPRESSION)) {
		dkc_compile_error(expression->line_number, COMPARE_TYPE_MISMATCH_ERR, MESSAGE_ARGUMENT_END);
	}

	expression->type = dkc_alloc_type_specifier(DVM_BOOLEAN_TYPE);

	return expression;
}

static Expression *fix_logical_and_or_expression(Block *current_block, Expression *expression) {
	expression->u.binary_expression.left = fix_expression(current_block, expression->u.binary_expression.left,
	                                                      expression);
	expression->u.binary_expression.right = fix_expression(current_block, expression->u.binary_expression.right,
	                                                       expression);
	if (dkc_is_boolean(expression->u.binary_expression.left->type) &&
	    dkc_is_boolean(expression->u.binary_expression.right->type)) {
		expression->type = dkc_alloc_type_specifier(DVM_BOOLEAN_TYPE);
	} else {
		dkc_compile_error(expression->line_number, LOGICAL_TYPE_MISMATCH_ERR, MESSAGE_ARGUMENT_END);
	}

	return expression;
}

static void check_argument(Block *current_block, int line_number, ParameterList *parameter_list, ArgumentList *argument, TypeSpecifier *array_base) {
	ParameterList*parameter;
	TypeSpecifier *temp_type;

	for (parameter = parameter_list; parameter && argument; parameter = parameter->next, argument = argument->next) {
		argument->expression = fix_expression(current_block, argument->expression, NULL);
		if (parameter->type->basic_type == DVM_BASE_TYPE) {
			DBG_assert(array_base != NULL, ("array_base == NULL\n"));
			temp_type = array_base;
		} else {
			temp_type = parameter->type;
		}
		argument->expression = create_assign_cast(argument->expression, temp_type);
	}

	if (parameter || argument) {
		dkc_compile_error(line_number, ARGUMENT_COUNT_MISMATCH_ERR, MESSAGE_ARGUMENT_END);
	}
}

static Expression *fix_function_call_expression(Block *current_block, Expression *expression) {
	DKC_Compiler *compiler = dkc_get_current_compiler();
	TypeSpecifier *array_base_p = NULL;
	TypeSpecifier array_base;
	Expression *function_expression = fix_expression(current_block, expression->u.function_call_expression.function,
	                                                 expression);
	FunctionDefinition *function_definition = NULL;

	expression->u.function_call_expression.function = function_expression;

	if (function_expression->kind == IDENTIFIER_EXPRESSION) {
		function_definition = dkc_search_function(function_expression->u.identifier.name);
	} else if (function_expression->kind == MEMBER_EXPRESSION) {
		if (dkc_is_array(function_expression->u.member_expression.expression->type)) {
			function_definition = &compiler->array_method[function_expression->u.member_expression.method_index];
			array_base = *function_expression->u.member_expression.expression->type;
			array_base.derive = function_expression->u.member_expression.expression->type->derive->next;
			array_base_p = &array_base;
		} else if (dkc_is_string(function_expression->u.member_expression.expression->type)) {
			function_definition = &compiler->string_method[function_expression->u.member_expression.method_index];
		} else {
			if (function_expression->u.member_expression.declaration->kind == FIELD_MEMBER) {
				dkc_compile_error(expression->line_number, FIELD_CAN_NOT_CALL_ERR,
				                  STRING_MESSAGE_ARGUMENT, "member_name",
				                  function_expression->u.member_expression.declaration->u.field.name,
				                  MESSAGE_ARGUMENT_END);
			}
			if (function_expression->u.member_expression.declaration->u.method.is_constructor) {
				Expression *object = function_expression->u.member_expression.expression;
				if (object->kind != SUPER_EXPRESSION && object->kind != THIS_EXPRESSION) {
					dkc_compile_error(expression->line_number, CONSTRUCTOR_CALLED_ERR,
					                  STRING_MESSAGE_ARGUMENT, "member_name",
					                  function_expression->u.member_expression.declaration->u.field.name,
					                  MESSAGE_ARGUMENT_END);
				}
			}
			function_definition = function_expression->u.member_expression.declaration->u.method.function_definition;
		}
	}

	if (function_definition == NULL) {
		dkc_compile_error(expression->line_number, FUNCTION_NOT_FOUND_ERR,
		                  STRING_MESSAGE_ARGUMENT, "name",
		                  function_expression->u.identifier.name,
		                  MESSAGE_ARGUMENT_END);
	}

	TypeSpecifier *function_type = function_definition->type;

	ParameterList *function_param = function_definition->parameter;
	check_argument(current_block, expression->line_number, function_param,
	               expression->u.function_call_expression.argument, array_base_p);
	expression->type = dkc_alloc_type_specifier(function_type->basic_type);
	*expression->type = *function_type;
	expression->type->derive = function_type->derive;
	if (function_type->basic_type == DVM_CLASS_TYPE) {
		expression->type->class_ref.identifier = function_type->class_ref.identifier;
		fix_type_specifier(expression->type);
	}

	return expression;
}

static void check_member_accessibility(int line_number, ClassDefinition *target_class, MemberDeclaration *member,
                                       char *member_name) {
	DKC_Compiler *compiler = dkc_get_current_compiler();
	if (compiler->current_class_definition == NULL ||
	    compiler->current_class_definition != target_class) {
		if (member->access_modifier == DVM_PRIVATE_ACCESS) {
			dkc_compile_error(line_number, PRIVATE_MEMBER_ACCESS_ERR, STRING_MESSAGE_ARGUMENT, "member_name",
			                  member_name, MESSAGE_ARGUMENT_END);
		}
	}

	if (!dkc_compare_package_name(compiler->package_name, target_class->package_name)
	    && member->access_modifier != DVM_PUBLIC_ACCESS) {
		dkc_compile_error(line_number, PACKAGE_MEMBER_ACCESS_ERR, STRING_MESSAGE_ARGUMENT, "member_name",
		                  member_name, MESSAGE_ARGUMENT_END);
	}
}

static DVM_Boolean is_interface_method(ClassDefinition *class_definition, MemberDeclaration *member,
                                ClassDefinition **target_interface, int *interface_index_out) {
	int interface_index;
	ExtendsList *extends_pos;
	for (extends_pos = class_definition->interface_list, interface_index = 0; extends_pos; extends_pos = extends_pos->next, interface_index++) {
		for (MemberDeclaration *member_pos = extends_pos->class_definition->member; member_pos; member_pos = member_pos->next) {
			if (member_pos->kind != METHOD_MEMBER) {
				continue;
			}

			if (!strcmp(member->u.method.function_definition->name, member_pos->u.method.function_definition->name)) {
				*target_interface = extends_pos->class_definition;
				*interface_index_out = interface_index;
				return DVM_TRUE;
			}
		}
	}

	return DVM_FALSE;
}

static Expression *fix_class_member_expression(Expression *expression, Expression *object, char *member_name) {
	fix_type_specifier(object->type);
	MemberDeclaration *member = dkc_search_member(object->type->class_ref.class_definition, member_name);

	if (member == NULL) {
		dkc_compile_error(expression->line_number, MEMBER_NOT_FOUND_ERR,
		                  STRING_MESSAGE_ARGUMENT, "class_name", object->type->class_ref.class_definition->name,
		                  STRING_MESSAGE_ARGUMENT, "member_name", member_name,
		                  MESSAGE_ARGUMENT_END);
	}

	check_member_accessibility(object->line_number, object->type->class_ref.class_definition, member, member_name);

	expression->u.member_expression.declaration = member;

	ClassDefinition *target_interface;
	int interface_index;

	if (member->kind == METHOD_MEMBER) {
		expression->type = create_function_derive_type(member->u.method.function_definition);
		if (object->type->class_ref.class_definition->class_or_interface == DVM_CLASS_DEFINITION &&
		    is_interface_method(object->type->class_ref.class_definition, member, &target_interface,
		                        &interface_index)) {
			expression->u.member_expression.expression = create_up_cast(object, target_interface, interface_index);
		}
	} else if (member->kind == FIELD_MEMBER) {
		if (object->kind == SUPER_EXPRESSION) {
			dkc_compile_error(expression->line_number, FIELD_OF_SUPER_REFERENCED_ERR, MESSAGE_ARGUMENT_END);
		}
		expression->type = member->u.field.type;
	}

	return expression;
}

static Expression *fix_array_method_expression(Expression *expression, Expression *object, char *member_name) {
	DKC_Compiler *compiler = dkc_get_current_compiler();
	FunctionDefinition *function_definition;
	int i;
	for (i = 0; i < compiler->array_method_count; i++) {
		if (!strcmp(compiler->array_method[i].name, member_name)) {
			break;
		}
	}
	if (i == compiler->array_method_count) {
		dkc_compile_error(expression->line_number, ARRAY_METHOD_NOT_FOUND_ERR,
		                  STRING_MESSAGE_ARGUMENT, "name", member_name,
		                  MESSAGE_ARGUMENT_END);
	}

	function_definition = &compiler->array_method[i];
	expression->u.member_expression.method_index = i;
	expression->type = create_function_derive_type(function_definition);

	return expression;
}

static Expression *fix_string_method_expression(Expression *expression, Expression *object, char *member_name) {
	DKC_Compiler *compiler = dkc_get_current_compiler();
	FunctionDefinition *function_definition;
	int i;
	for (i = 0; i < compiler->string_method_count; i++) {
		if (!strcmp(compiler->string_method[i].name, member_name)) {
			break;
		}
	}
	if (i == compiler->array_method_count) {
		dkc_compile_error(expression->line_number, STRING_METHOD_NOT_FOUND_ERR,
		                  STRING_MESSAGE_ARGUMENT, "name", member_name,
		                  MESSAGE_ARGUMENT_END);
	}

	function_definition = &compiler->string_method[i];
	expression->u.member_expression.method_index = i;
	expression->type = create_function_derive_type(function_definition);

	return expression;
}

static Expression *fix_member_expression(Block *current_block, Expression *expression) {
	Expression *object = expression->u.member_expression.expression = fix_expression(current_block,
	                                                                                 expression->u.member_expression.expression,
	                                                                                 expression);
	if (dkc_is_class_object(object->type)) {
		return fix_class_member_expression(expression, object, expression->u.member_expression.member_name);
	} else if (dkc_is_array(object->type)) {
		return fix_array_method_expression(expression, object, expression->u.member_expression.member_name);
	} else if (dkc_is_string(object->type)) {
		return fix_string_method_expression(expression, object, expression->u.member_expression.member_name);
	} else {
		dkc_compile_error(expression->line_number, MEMBER_EXPRESSION_TYPE_ERR, MESSAGE_ARGUMENT_END);
	}

	return NULL;
}

static Expression *fix_this_expression(Expression *expression) {
	TypeSpecifier *type;
	ClassDefinition *class_definition = dkc_get_current_compiler()->current_class_definition;
	if (class_definition == NULL) {
		dkc_compile_error(expression->line_number, THIS_OUT_OF_CLASS_ERR, MESSAGE_ARGUMENT_END);
	}

	type = dkc_alloc_type_specifier(DVM_CLASS_TYPE);
	type->class_ref.identifier = class_definition->name;
	type->class_ref.class_definition = class_definition;
	expression->type = type;

	return expression;
}

static Expression *fix_super_expression(Expression *expression, Expression *parent) {
	TypeSpecifier *type;
	ClassDefinition *class_definition = dkc_get_current_compiler()->current_class_definition;

	if (class_definition == NULL) {
		dkc_compile_error(expression->line_number, SUPER_OUT_OF_CLASS_ERR, MESSAGE_ARGUMENT_END);
	}

	if (class_definition->super_class == NULL) {
		dkc_compile_error(expression->line_number, HASNT_SUPER_CLASS_ERR, MESSAGE_ARGUMENT_END);
	}

	if (parent == NULL || parent->kind != MEMBER_EXPRESSION) {
		dkc_compile_error(expression->line_number, SUPER_NOT_IN_MEMBER_EXPRESSION_ERR, MESSAGE_ARGUMENT_END);
	}

	type = dkc_alloc_type_specifier(DVM_CLASS_TYPE);
	type->class_ref.identifier = class_definition->super_class->name;
	type->class_ref.class_definition = class_definition->super_class;

	expression->type = type;

	return expression;
}

static Expression *fix_array_literal_expression(Block *current_block, Expression *expression) {
	TypeSpecifier *elem_type;
	ExpressionList *literal = expression->u.array_literal;
	if (literal == NULL) {
		dkc_compile_error(expression->line_number, ARRAY_LITERAL_EMPTY_ERR, MESSAGE_ARGUMENT_END);
	}

	literal->expression = fix_expression(current_block, literal->expression, expression);

	elem_type = literal->expression->type;

	for (ExpressionList *pos = literal->next; pos; pos = pos->next) {
		pos->expression = fix_expression(current_block, pos->expression, expression);
		pos->expression = create_assign_cast(pos->expression, elem_type);
	}

	expression->type = dkc_alloc_type_specifier(elem_type->basic_type);
	*expression->type = *elem_type;
	expression->type->derive = dkc_alloc_type_derive(ARRAY_DERIVE);
	expression->type->derive->next = elem_type->derive;

	return expression;
}

static Expression *fix_index_expression(Block *current_block, Expression *expression) {
	IndexExpression *index_expression = &expression->u.index_expression;
	index_expression->array = fix_expression(current_block, index_expression->array, expression);
	index_expression->index = fix_expression(current_block, index_expression->index, expression);
	if (index_expression->array->type->derive != NULL && index_expression->array->type->derive->tag == ARRAY_DERIVE) {
		expression->type = dkc_alloc_type_specifier2(index_expression->array->type);
		expression->type->derive = index_expression->array->type->derive->next;
	} else {
		dkc_compile_error(expression->line_number, INDEX_LEFT_OPERAND_NOT_ARRAY_ERR, MESSAGE_ARGUMENT_END);
	}

	if (!dkc_is_int(index_expression->index->type)) {
		dkc_compile_error(expression->line_number, INDEX_NOT_INT_ERR, MESSAGE_ARGUMENT_END);
	}

	return expression;
}

static Expression *fix_inc_dec_expression(Block *current_block, Expression *expression) {
	expression->u.inc_dec.operand = fix_expression(current_block, expression->u.inc_dec.operand, expression);

	if (!dkc_is_int(expression->u.inc_dec.operand->type)) {
		dkc_compile_error(expression->line_number, INC_DEC_TYPE_MISMATCH_ERR, MESSAGE_ARGUMENT_END);
	}

	expression->type = expression->u.inc_dec.operand->type;

	return expression;
}

static Expression *fix_instanceof_expression(Block *current_block, Expression *expression) {
	DVM_Boolean is_interface_dummy;
	int interface_index_dummy;
	expression->u.instanceof_expression.operand = fix_expression(current_block,
	                                                             expression->u.instanceof_expression.operand,
	                                                             expression);
	fix_type_specifier(expression->u.instanceof_expression.type);
	Expression *operand = expression->u.instanceof_expression.operand;
	TypeSpecifier *target = expression->u.instanceof_expression.type;

	if (!dkc_is_object(operand->type)) {
		dkc_compile_error(expression->line_number, INSTANCEOF_OPERAND_NOT_REFERENCE_ERR, MESSAGE_ARGUMENT_END);
	}
	if (!dkc_is_object(target)) {
		dkc_compile_error(expression->line_number, INSTANCEOF_TYPE_NOT_REFERENCE_ERR, MESSAGE_ARGUMENT_END);
	}

	if (!dkc_is_class_object(operand->type) || !dkc_is_class_object(target)) {
		dkc_compile_error(expression->line_number, INSTANCEOF_FOR_NOT_CLASS_ERR, MESSAGE_ARGUMENT_END);
	}

	if (dkc_compare_type(operand->type, target)) {
		dkc_compile_error(expression->line_number, INSTANCEOF_MUST_RETURN_TRUE_ERR, MESSAGE_ARGUMENT_END);
	}

	if (is_super_class(operand->type->class_ref.class_definition, target->class_ref.class_definition,&is_interface_dummy, &interface_index_dummy)) {
		dkc_compile_error(expression->line_number, INSTANCEOF_MUST_RETURN_TRUE_ERR, MESSAGE_ARGUMENT_END);
	}

	if (target->class_ref.class_definition->class_or_interface == DVM_CLASS_DEFINITION &&
	    !is_super_class(target->class_ref.class_definition, operand->type->class_ref.class_definition,
	                    &is_interface_dummy, &interface_index_dummy)) {

		dkc_compile_error(expression->line_number, INSTANCEOF_MUST_RETURN_FALSE_ERR, MESSAGE_ARGUMENT_END);
	}

	expression->type = dkc_alloc_type_specifier(DVM_BOOLEAN_TYPE);

	return expression;
}

static Expression *fix_new_expression(Block *current_block, Expression *expression) {
	MemberDeclaration *member;
	TypeSpecifier *type;
	expression->u.new_expression.class_definition = search_class_and_add(expression->line_number,
	                                                                     expression->u.new_expression.class_name,
	                                                                     &expression->u.new_expression.class_index);
	if (expression->u.new_expression.class_definition->is_abstract) {
		dkc_compile_error(expression->line_number, NEW_ABSTRACT_CLASS_ERR,
		                  STRING_MESSAGE_ARGUMENT, "name", expression->u.new_expression.class_name,
		                  MESSAGE_ARGUMENT_END);
	}
	if (!expression->u.new_expression.method_name) {
		expression->u.new_expression.method_name = DEFAULT_CONSTRUCTOR_NAME;
	}
	member = dkc_search_member(expression->u.new_expression.class_definition, expression->u.new_expression.method_name);
	if (member == NULL) {
		dkc_compile_error(expression->line_number, MEMBER_NOT_FOUND_ERR,
		                  STRING_MESSAGE_ARGUMENT, "class_name", expression->u.new_expression.class_name,
		                  STRING_MESSAGE_ARGUMENT, "member_name", expression->u.new_expression.method_name,
		                  MESSAGE_ARGUMENT_END);
	}

	if (member->kind != METHOD_MEMBER) {
		dkc_compile_error(expression->line_number, CONSTRUCTOR_IS_FIELD_ERR,
		                  STRING_MESSAGE_ARGUMENT, "member_name", expression->u.new_expression.method_name,
		                  MESSAGE_ARGUMENT_END);
	}
	if (!member->u.method.is_constructor) {
		dkc_compile_error(expression->line_number, NOT_CONSTRUCTOR_ERR,
		                  STRING_MESSAGE_ARGUMENT, "member_name", expression->u.new_expression.method_name,
		                  MESSAGE_ARGUMENT_END);
	}
	check_member_accessibility(expression->line_number, expression->u.new_expression.class_definition, member,
	                           expression->u.new_expression.method_name);

	DBG_assert(member->u.method.function_definition->type->derive==NULL&&member->u.method.function_definition->type->basic_type==DVM_VOID_TYPE, ("constructor is not void.\n"));

	check_argument(current_block, expression->line_number, member->u.method.function_definition->parameter,
	               expression->u.new_expression.argument, NULL);

	expression->u.new_expression.method_declaration = member;
	type = dkc_alloc_type_specifier(DVM_CLASS_TYPE);
	type->class_ref.identifier = expression->u.new_expression.class_definition->name;
	type->class_ref.class_definition = expression->u.new_expression.class_definition;
	expression->type = type;

	return expression;
}

static Expression *fix_down_cast_expression(Block *current_block, Expression *expression) {
	DVM_Boolean is_interface_dummy;
	int interface_index_dummy;
	TypeSpecifier *org_type;
	TypeSpecifier *target_type;

	expression->u.down_cast.operand = fix_expression(current_block, expression->u.down_cast.operand, expression);
	fix_type_specifier(expression->u.down_cast.type);
	org_type = expression->u.down_cast.operand->type;
	target_type = expression->u.down_cast.type;
	if (!dkc_is_class_object(org_type)) {
		dkc_compile_error(expression->line_number, DOWN_CAST_OPERAND_IS_NOT_CLASS_ERR, MESSAGE_ARGUMENT_END);
	}
	if (!dkc_is_class_object(target_type)) {
		dkc_compile_error(expression->line_number, DOWN_CAST_TARGET_IS_NOT_CLASS_ERR, MESSAGE_ARGUMENT_END);
	}

	if (dkc_compare_type(org_type, target_type)) {
		dkc_compile_error(expression->line_number, DOWN_CAST_DO_NOTHING_ERR, MESSAGE_ARGUMENT_END);
	}

	if (target_type->class_ref.class_definition->class_or_interface == DVM_CLASS_DEFINITION &&
	    !is_super_class(target_type->class_ref.class_definition, org_type->class_ref.class_definition,
	                    &is_interface_dummy, &interface_index_dummy)) {
		dkc_compile_error(expression->line_number, DOWN_CAST_TO_BAD_CLASS_ERR, MESSAGE_ARGUMENT_END);
	}

	expression->type = target_type;

	return expression;
}

static Expression *fix_array_creation_expression(Block *current_block, Expression *expression) {
	TypeDerive *derive = NULL;
	TypeDerive *tmp_derive;
	fix_type_specifier(expression->u.array_creation.type);

	for (ArrayDimension *pos = expression->u.array_creation.dimension; pos; pos = pos->next) {
		if (pos->expression) {
			pos->expression = fix_expression(current_block, pos->expression, expression);
			if (!dkc_is_int(pos->expression->type)) {
				dkc_compile_error(expression->line_number, ARRAY_SIZE_NOT_INT_ERR, MESSAGE_ARGUMENT_END);
			}
		}
		tmp_derive = dkc_alloc_type_derive(ARRAY_DERIVE);
		tmp_derive->next = derive;
		derive = tmp_derive;
	}

	expression->type = dkc_alloc_type_specifier2(expression->u.array_creation.type);
	expression->type->derive = derive;

	return expression;
}

static Expression *fix_expression(Block *current_block, Expression *expression, Expression *parent) {
	if (expression == NULL) {
		return NULL;
	}
	switch (expression->kind) {
		case BOOLEAN_EXPRESSION:
			expression->type = dkc_alloc_type_specifier(DVM_BOOLEAN_TYPE);
			break;
		case INT_EXPRESSION:
			expression->type = dkc_alloc_type_specifier(DVM_INT_TYPE);
			break;
		case DOUBLE_EXPRESSION:
			expression->type = dkc_alloc_type_specifier(DVM_DOUBLE_TYPE);
			break;
		case STRING_EXPRESSION:
			expression->type = dkc_alloc_type_specifier(DVM_STRING_TYPE);
			break;
		case IDENTIFIER_EXPRESSION:
			expression = fix_identifier_expression(current_block, expression);
			break;
		case COMMA_EXPRESSION:
			expression = fix_comma_expression(current_block, expression);
			break;
		case ASSIGN_EXPRESSION:
			expression = fix_assign_expression(current_block, expression);
			break;
		case ADD_EXPRESSION:
		case SUB_EXPRESSION:
		case MUL_EXPRESSION:
		case DIV_EXPRESSION:
		case MOD_EXPRESSION:
			expression = fix_math_binary_expression(current_block, expression);
			break;
		case EQ_EXPRESSION:
		case NE_EXPRESSION:
		case GT_EXPRESSION:
		case GE_EXPRESSION:
		case LT_EXPRESSION:
		case LE_EXPRESSION:
			expression = fix_compare_expression(current_block, expression);
			break;
		case LOGICAL_AND_EXPRESSION:
		case LOGICAL_OR_EXPRESSION:
			expression = fix_logical_and_or_expression(current_block, expression);
			break;
		case FUNCTION_CALL_EXPRESSION:
			expression = fix_function_call_expression(current_block, expression);
			break;
		case MEMBER_EXPRESSION:
			expression = fix_member_expression(current_block, expression);
			break;
		case NULL_EXPRESSION:
			expression->type = dkc_alloc_type_specifier(DVM_NULL_TYPE);
			break;
		case THIS_EXPRESSION:
			expression = fix_this_expression(expression);
			break;
		case SUPER_EXPRESSION:
			expression = fix_super_expression(expression, parent);
			break;
		case ARRAY_LITERAL_EXPRESSION:
			expression = fix_array_literal_expression(current_block, expression);
			break;
		case INDEX_EXPRESSION:
			expression = fix_index_expression(current_block, expression);
			break;
		case INCREMENT_EXPRESSION:
		case DECREMENT_EXPRESSION:
			expression = fix_inc_dec_expression(current_block, expression);
			break;
		case INSTANCEOF_EXPRESSION:
			expression = fix_instanceof_expression(current_block, expression);
			break;
		case DOWN_CAST_EXPRESSION:
			expression = fix_down_cast_expression(current_block, expression);
			break;
		case CAST_EXPRESSION:
			break;
		case UP_CAST_EXPRESSION:
			break;
		case NEW_EXPRESSION:
			expression = fix_new_expression(current_block, expression);
			break;
		case ARRAY_CREATION_EXPRESSION:
			expression = fix_array_creation_expression(current_block, expression);
			break;
		case EXPRESSION_KIND_COUNT_PLUS_1:
			break;
		default:
			DBG_assert(0, ("bad case. kind..%d\n", expression->kind));
	}

	fix_type_specifier(expression->type);

	return expression;
}

static void fix_if_statement(Block *current_block, IfStatement *if_statement, FunctionDefinition *function_definition) {
	if_statement->condition = fix_expression(current_block, if_statement->condition, NULL);
	if (!dkc_is_boolean(if_statement->condition->type)) {
		dkc_compile_error(if_statement->condition->line_number, IF_CONDITION_NOT_BOOLEAN_ERR, MESSAGE_ARGUMENT_END);
	}

	fix_statement_list(if_statement->then_block, if_statement->then_block->statement_list, function_definition);

	for (Elseif *pos = if_statement->elseif_list; pos; pos = pos->next) {
		pos->condition = fix_expression(current_block, pos->condition, NULL);
		if (pos->block) {
			fix_statement_list(pos->block, pos->block->statement_list, function_definition);
		}
	}

	if (if_statement->else_block) {
		fix_statement_list(if_statement->else_block, if_statement->else_block->statement_list, function_definition);
	}
}

static void fix_while_statement(Block *current_block, WhileStatement *while_s, FunctionDefinition *function_definition) {
	while_s->condition = fix_expression(current_block, while_s->condition, NULL);
	if (!dkc_is_boolean(while_s->condition->type)) {
		dkc_compile_error(while_s->condition->line_number, WHILE_CONDITION_NOT_BOOLEAN_ERR, MESSAGE_ARGUMENT_END);
	}
	fix_statement_list(while_s->block, while_s->block->statement_list, function_definition);
}

static void fix_for_statement(Block *current_block, ForStatement *for_s, FunctionDefinition *function_definition) {
	for_s->init = fix_expression(current_block, for_s->init, NULL);
	for_s->condition = fix_expression(current_block, for_s->condition, NULL);
	if (for_s->condition != NULL && !dkc_is_boolean(for_s->condition->type)) {
		dkc_compile_error(for_s->condition->line_number, FOR_CONDITION_NOT_BOOLEAN_ERR, MESSAGE_ARGUMENT_END);
	}
	for_s->post = fix_expression(current_block, for_s->post, NULL);
	fix_statement_list(for_s->block, for_s->block->statement_list, function_definition);
}

static void fix_statement(Block *current_block, Statement *statement, FunctionDefinition *function_definition) {
	switch (statement->type) {
		case EXPRESSION_STATEMENT:
			statement->u.expression_s = fix_expression(current_block, statement->u.expression_s, NULL);
			break;
		case IF_STATEMENT:
			fix_if_statement(current_block, &statement->u.if_s, function_definition);
			break;
		case WHILE_STATEMENT:
			fix_while_statement(current_block, &statement->u.while_s, function_definition);
			break;
		case FOR_STATEMENT:
			fix_for_statement(current_block, &statement->u.for_s, function_definition);
			break;
		case DO_WHILE_STATEMENT:
			// TODO
//			fix_do_while_statement(current_block, statement->u.)
			break;
		case FOREACH_STATEMENT:
			// TODO
//			statement->u.foreach_s.collection = fix_do_while_statement(current_block, statement->u.foreach_s.collection,
//			                                                           NULL);
//			fix_statement_list(statement->u.foreach_s.block, statement->u.foreach_s.block->statement_list,
//			                   function_definition);
			break;
		case RETURN_STATEMENT:
			fix_return_statement(current_block, statement, function_definition);
			break;
		case BREAK_STATEMENT:
			break;
		case CONTINUE_STATEMENT:
			break;
		case DECLARATION_STATEMENT:
			add_declaration(current_block, statement->u.declaration_s, function_definition, statement->line_number,
			                DVM_FALSE);
			fix_type_specifier(statement->u.declaration_s->type);
			if (statement->u.declaration_s->initializer) {
				statement->u.declaration_s->initializer = fix_expression(current_block,
				                                                         statement->u.declaration_s->initializer, NULL);
				statement->u.declaration_s->initializer = create_assign_cast(statement->u.declaration_s->initializer,
				                                                             statement->u.declaration_s->type);
			}
			break;
		case STATEMENT_TYPE_COUNT_PLUS_1:
		default:
			DBG_assert(0, ("bad case. type..%d\n", statement->type));
	}
}

static void fix_return_statement(Block *current_block, Statement *statement, FunctionDefinition *function_definition) {
	Expression *casted_expression;
	Expression *return_value = fix_expression(current_block, statement->u.return_s.return_value, NULL);

	if (function_definition->type->derive == NULL && function_definition->type->basic_type == DVM_VOID_TYPE && return_value != NULL) {
		dkc_compile_error(statement->line_number, RETURN_IN_VOID_FUNCTION_ERR, MESSAGE_ARGUMENT_END);
	}

	if (return_value == NULL) {
		if (function_definition->type->derive) {
			if (function_definition->type->derive->tag == ARRAY_DERIVE) {
				return_value = dkc_alloc_expression(NULL_EXPRESSION);
			} else {
				DBG_assert(0, (("function_definition->type->derive..%d\n"), function_definition->type->derive));
			}
		} else {
			switch (function_definition->type->basic_type) {
				case DVM_VOID_TYPE:
					return_value = dkc_alloc_expression(INT_EXPRESSION);
					return_value->u.int_value = 0;
					break;
				case DVM_BOOLEAN_TYPE:
					return_value = dkc_alloc_expression(BOOLEAN_EXPRESSION);
					return_value->u.boolean_value = DVM_FALSE;
					break;
				case DVM_INT_TYPE:
					return_value = dkc_alloc_expression(INT_EXPRESSION);
					return_value->u.int_value = 0;
					break;
				case DVM_DOUBLE_TYPE:
					return_value = dkc_alloc_expression(DOUBLE_EXPRESSION);
					return_value->u.double_value = 0;
					break;
				case DVM_STRING_TYPE:
				case DVM_CLASS_TYPE:
					return_value = dkc_alloc_expression(NULL_EXPRESSION);
					break;
				case DVM_NULL_TYPE:
				case DVM_BASE_TYPE:
				default:
					DBG_assert(0, ("basic_type..%d\n"));
					break;
			}
		}
		statement->u.return_s.return_value = return_value;
		return;
	}
	casted_expression = create_assign_cast(statement->u.return_s.return_value, function_definition->type);
	statement->u.return_s.return_value = casted_expression;
}

static void fix_statement_list(Block *current_block, StatementList *list, FunctionDefinition *function_definition) {
	for (StatementList *pos = list; pos; pos = pos->next) {
		fix_statement(current_block, pos->statement, function_definition);
	}
}

static MemberDeclaration *search_member_in_super(ClassDefinition *class_definition, char *member_name) {
	MemberDeclaration *member = NULL;
	if (class_definition->super_class) {
		member = dkc_search_member(class_definition->super_class, member_name);
	}

	if (member) {
		return member;
	}

	for (ExtendsList *extends_p = class_definition->interface_list; extends_p; extends_p = extends_p->next) {
		member = dkc_search_member(extends_p->class_definition, member_name);
		if (member) {
			return member;
		}
	}

	return NULL;
}

static void get_super_field_method_count(ClassDefinition *class_definition, int *field_index_out, int *method_index_out) {
	int field_index = -1;
	int method_index = -1;
	for (ClassDefinition *class_def_pos = class_definition->super_class; class_def_pos; class_def_pos = class_def_pos->super_class) {
		for (MemberDeclaration *member_pos = class_def_pos->member; member_pos; member_pos = member_pos->next) {
			if (member_pos->kind == METHOD_MEMBER) {
				if (member_pos->u.method.method_index > method_index) {
					method_index = member_pos->u.method.method_index;
				}
			} else {
				DBG_assert(member_pos->kind == FIELD_MEMBER, ("member_pos->kind..%d", member_pos->kind));
				if (member_pos->u.field.field_index > field_index) {
					field_index = member_pos->u.field.field_index;
				}
			}
		}
	}

	*field_index_out = field_index + 1;
	*method_index_out = method_index + 1;
}

static void fix_extends(ClassDefinition *class_definition) {
	if (class_definition->class_or_interface == DVM_INTERFACE_DEFINITION
			&& class_definition->extends != NULL) {
		dkc_compile_error(class_definition->line_number, INTERFACE_INHERIT_ERR, MESSAGE_ARGUMENT_END);
	}
	class_definition->interface_list = NULL;

	int dummy_class_index;
	ExtendsList *new_extends_list;
	ExtendsList *last_extends_list = NULL;
	for (ExtendsList *pos = class_definition->extends; pos; pos = pos->next) {
		ClassDefinition *super = search_class_and_add(class_definition->line_number, pos->identifier,
		                                              &dummy_class_index);
		pos->class_definition = super;

		new_extends_list = dkc_malloc(sizeof(ExtendsList));
		*new_extends_list = *pos;
		new_extends_list->next = NULL;

		if (super->class_or_interface == DVM_CLASS_DEFINITION) {
			if (class_definition->super_class) {
				dkc_compile_error(class_definition->line_number, MULTIPLE_INHERITANCE_ERR, STRING_MESSAGE_ARGUMENT,
				                  "name", super->name, MESSAGE_ARGUMENT_END);
			}
			if (!super->is_abstract) {
				dkc_compile_error(class_definition->line_number, INHERIT_CONCRETE_CLASS_ERR, STRING_MESSAGE_ARGUMENT,
				                  "name", super->name, MESSAGE_ARGUMENT_END);
			}
			class_definition->super_class = super;
		} else {
			DBG_assert(super->class_or_interface == DVM_INTERFACE_DEFINITION, ("super..%d", super->class_or_interface));
			if (class_definition->interface_list == NULL) {
				class_definition->interface_list = new_extends_list;
			} else {
				last_extends_list->next = new_extends_list;
			}
			last_extends_list = new_extends_list;
		}
	}
}

static void add_super_interfaces(ClassDefinition *class_definition) {
	ClassDefinition *super_pos;
	ExtendsList *tail = NULL;
	ExtendsList *if_pos;
	if (class_definition->interface_list) {
		for (tail = class_definition->interface_list; tail->next; tail = tail->next);
	}

	for (super_pos = class_definition->super_class; super_pos; super_pos = super_pos->super_class) {
		for (if_pos = super_pos->interface_list; if_pos; if_pos = if_pos->next) {
			ExtendsList *new_extends = dkc_malloc(sizeof(ExtendsList));
			*new_extends = *if_pos;
			new_extends->next = NULL;
			if (tail) {
				tail->next = new_extends;
			}else {
				class_definition->interface_list = new_extends;
			}
			tail = new_extends;
		}
	}
}

static void add_default_constructor(ClassDefinition *class_definition) {
	MemberDeclaration *member_pos;
	MemberDeclaration *tail = NULL;
	TypeSpecifier *type;
	FunctionDefinition *function_definition;
	Block *block;
	Statement *statement;
	Expression *super_e;
	Expression *member_e;
	Expression *function_call_e;
	ClassOrMemberModifierList modifier;
	ClassOrMemberModifierList modifier2;
	ClassOrMemberModifierList modifier3;

	for (member_pos = class_definition->member; member_pos; member_pos = member_pos->next) {
		if (member_pos->kind == METHOD_MEMBER && member_pos->u.method.is_constructor) {
			return;
		}

		tail = member_pos;
	}

	type = dkc_alloc_type_specifier(DVM_VOID_TYPE);
	block = dkc_alloc_block();
	modifier = dkc_create_class_or_member_modifier(VIRTUAL_MODIFIER);
	if (class_definition->super_class) {
		statement = dkc_alloc_statement(EXPRESSION_STATEMENT);
		super_e = dkc_create_super_expression();
		member_e = dkc_create_member_expression(super_e, DEFAULT_CONSTRUCTOR_NAME);
		function_call_e = dkc_create_function_call_expression(member_e, NULL);
		statement->u.expression_s = function_call_e;
		block->statement_list = dkc_create_statement_list(statement);
		modifier = dkc_create_class_or_member_modifier(VIRTUAL_MODIFIER);
		modifier2 = dkc_create_class_or_member_modifier(OVERRIDE_MODIFIER);
		modifier = dkc_chain_class_or_member_modifier(modifier, modifier2);
		modifier3 = dkc_create_class_or_member_modifier(PUBLIC_MODIFIER);
		modifier = dkc_chain_class_or_member_modifier(modifier, modifier3);
	} else {
		block->statement_list = NULL;
	}
	function_definition = dkc_create_function_definition(type, DEFAULT_CONSTRUCTOR_NAME, NULL, block);

	if (tail) {
		tail->next = dkc_create_method_member(&modifier, function_definition, DVM_TRUE);
	} else {
		class_definition->member = dkc_create_method_member(&modifier, function_definition, DVM_TRUE);
	}
}

static DVM_Boolean check_type_compatibility(TypeSpecifier *super_type, TypeSpecifier *sub_type) {
	DVM_Boolean is_interface_dummy;
	int interface_index_dummy;

	if (!dkc_is_class_object(super_type)) {
		return dkc_compare_type(super_type, sub_type);
	}
	if (!dkc_is_class_object(sub_type)) {
		return DVM_FALSE;
	}
	if (super_type->class_ref.class_definition == sub_type->class_ref.class_definition
	    || is_super_class(sub_type->class_ref.class_definition, super_type->class_ref.class_definition,
	                   &is_interface_dummy, &interface_index_dummy)) {
		return DVM_TRUE;
	}

	return DVM_FALSE;
}

static void check_func_compati_sub(int line_number, char *name, TypeSpecifier *type1, ParameterList *parameter1,
                                   TypeSpecifier *type2, ParameterList *parameter2) {
	ParameterList *parameter1_pos;
	ParameterList *parameter2_pos;
	int parameter_index = 1;
	for (parameter1_pos = parameter1, parameter2_pos = parameter2;
	     parameter1_pos != NULL && parameter2_pos != NULL;
	     parameter1_pos = parameter1_pos->next, parameter2_pos = parameter2_pos->next) {

		if (!check_type_compatibility(parameter2_pos->type, parameter1_pos->type)) {
			dkc_compile_error(line_number, BAD_PARAMETER_TYPE_ERR,
			                  STRING_MESSAGE_ARGUMENT, "function_name", name,
			                  INT_MESSAGE_ARGUMENT, "index", parameter_index,
			                  STRING_MESSAGE_ARGUMENT, "parameter_name", parameter2_pos->name,
			                  MESSAGE_ARGUMENT_END);
		}
		parameter_index++;
	}

	if (parameter1_pos != NULL || parameter2_pos != NULL) {
		dkc_compile_error(line_number, BAD_PARAMETER_COUNT_ERR,
		                  STRING_MESSAGE_ARGUMENT, "name", name,
		                  MESSAGE_ARGUMENT_END);
	}

	if (!check_type_compatibility(type1, type2)) {
		dkc_compile_error(line_number, BAD_RETURN_TYPE_ERR,
		                  STRING_MESSAGE_ARGUMENT, "name", name,
		                  MESSAGE_ARGUMENT_END);
	}
}

static void check_function_compatibility(FunctionDefinition *fun_def1, FunctionDefinition *fun_def2) {
	check_func_compati_sub(fun_def2->end_line_number, fun_def2->name, fun_def1->type, fun_def1->parameter,
	                       fun_def2->type, fun_def2->parameter);
}

static void check_method_override(MemberDeclaration *super_method, MemberDeclaration *sub_method) {
	if ((super_method->access_modifier == DVM_PUBLIC_ACCESS && sub_method->access_modifier != DVM_PUBLIC_ACCESS) ||
	    (super_method->access_modifier == DVM_FILE_ACCESS && sub_method->access_modifier == DVM_PRIVATE_ACCESS)) {
		dkc_compile_error(sub_method->line_number, OVERRIDE_METHOD_ACCESSIBILITY_ERR, STRING_MESSAGE_ARGUMENT, "name",
		                  sub_method->u.method.function_definition->name, MESSAGE_ARGUMENT_END);
	}

	if (!sub_method->u.method.is_constructor) {
		check_function_compatibility(super_method->u.method.function_definition,
		                             sub_method->u.method.function_definition);
	}
}

static void fix_class_list(DKC_Compiler *compiler) {
	ClassDefinition *class_pos;
	for (class_pos = compiler->class_definition_list; class_pos; class_pos = class_pos->next) {
		add_class(class_pos);
		fix_extends(class_pos);
	}
	for (class_pos = compiler->class_definition_list; class_pos; class_pos = class_pos->next) {
		add_super_interfaces(class_pos);
	}
	for (class_pos = compiler->class_definition_list; class_pos; class_pos = class_pos->next) {
		if (class_pos->class_or_interface != DVM_CLASS_DEFINITION) {
			continue;
		}
		compiler->current_class_definition = class_pos;
		add_default_constructor(class_pos);
		compiler->current_class_definition = NULL;
	}


	int field_index;
	int method_index;
	char *abstract_method_name = NULL;
	MemberDeclaration *super_member;

	for (class_pos = compiler->class_definition_list; class_pos; class_pos = class_pos->next) {
		compiler->current_class_definition = class_pos;
		get_super_field_method_count(class_pos, &field_index, &method_index);
		for (MemberDeclaration *pos = class_pos->member; pos; pos = pos->next) {
			if (pos->kind == METHOD_MEMBER) {
				fix_function(pos->u.method.function_definition);
				super_member = search_member_in_super(class_pos, pos->u.method.function_definition->name);
				if (super_member) {
					if (super_member->kind != METHOD_MEMBER) {
						dkc_compile_error(pos->line_number, FIELD_OVERRIDED_ERR,
						                  STRING_MESSAGE_ARGUMENT, "name",
						                  super_member->u.field.name,
						                  MESSAGE_ARGUMENT_END);
					}
					if (!super_member->u.method.is_virtual) {
						dkc_compile_error(pos->line_number, NON_VIRTUAL_METHOD_OVERRIDED_ERR,
						                  STRING_MESSAGE_ARGUMENT, "name",
						                  super_member->u.method.function_definition->name,
						                  MESSAGE_ARGUMENT_END);
					}
					if (!super_member->u.method.is_override) {
						dkc_compile_error(pos->line_number, NEED_OVERRIDE_ERR,
						                  STRING_MESSAGE_ARGUMENT, "name",
						                  super_member->u.method.function_definition->name,
						                  MESSAGE_ARGUMENT_END);
					}
					check_method_override(super_member, pos);

					pos->u.method.method_index = super_member->u.method.method_index;
				} else {
					pos->u.method.method_index = method_index;
					method_index++;
				}
				if (pos->u.method.is_abstract) {
					abstract_method_name = pos->u.method.function_definition->name;
				}
			} else {
				DBG_assert(pos->kind == FIELD_MEMBER, ("member->kind..%d", pos->kind));
				fix_type_specifier(pos->u.field.type);
				super_member = search_member_in_super(class_pos, pos->u.field.name);
				if (super_member) {
					dkc_compile_error(pos->line_number, FIELD_NAME_DUPLICATE_ERR,
					                  STRING_MESSAGE_ARGUMENT, "name",
					                  pos->u.field.name,
					                  MESSAGE_ARGUMENT_END);
				} else {
					pos->u.field.field_index = field_index;
					field_index++;
				}
			}
		}
		if (abstract_method_name && !class_pos->is_abstract) {
			dkc_compile_error(class_pos->line_number, ABSTRACT_METHOD_IN_CONCRETE_CLASS_ERR,
			                  STRING_MESSAGE_ARGUMENT, "method_name",
			                  abstract_method_name,
			                  MESSAGE_ARGUMENT_END);
		}
		compiler->current_class_definition = NULL;
	}
}

void dkc_fix_tree(DKC_Compiler *compiler) {
    fix_class_list(compiler);
    int variable_count = 0;

    for (FunctionDefinition *pos = compiler->function_list; pos; pos = pos->next) {
        reserve_function_index(compiler, pos);
    }

    fix_statement_list(NULL, compiler->statement_list, NULL);

    for (FunctionDefinition *pos = compiler->function_list; pos; pos = pos->next) {
        if (pos->class_definition == NULL) {
            fix_function(pos);
        }
    }

    for (DeclarationList *decl = compiler->declaration_list; decl; decl = decl->next) {
        decl->declaration->variable_index = variable_count;
        variable_count++;
    }
}