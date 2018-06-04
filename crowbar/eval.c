//
// Created by sulvto on 18-6-2.
//
#include "crowbar.h"

static void push_value(CRB_Interpreter *interpreter, CRB_Value *value) {
    DEB_assert(interpreter->stack.stack_pointer <= interpreter->stack.stack_alloc_size,
               ("stack_pointer..%d, stack_alloc_size..%d\n",
                       interpreter->stack.stack_pointer,
                       interpreter->stack.stack_alloc_size));
    if (interpreter->stack.stack_pointer == interpreter->stack.stack_alloc_size) {
        interpreter->stack.stack_alloc_size += STACK_ALLOC_SIZE;
        interpreter->stack = MEM_malloc(interpreter->stack.stack,
                                        sizeof(CRB_Value) * interpreter->stack.stack_alloc_size);
    }

    interpreter->stack[interpreter->stack.stack_pointer] = *value;
    interpreter->stack.stack_pointer++;
}

static CRB_Value pop_value(CRB_Interpreter *interpreter) {
    return interpreter->stack.stack[--interpreter->stack.stack_pointer];
}

static CRB_Value *peek_value(CRB_Interpreter *interpreter, int index) {
    return &interpreter->stack.stack[interpreter->stack.stack_pointer - index - 1];
}

static void eval_boolean_expression(CRB_Interpreter *interpreter, CRB_Boolean boolean_value) {
    CRB_Value value;
    value.type = CRB_BOOLEAN_VALUE;
    value.u.boolean_value = boolean_value;
    push_value(interpreter, &value);
}

static void eval_int_expression(CRB_Interpreter *interpreter, int int_value) {
    CRB_Value value;
    value.type = CRB_INT_VALUE;
    value.u.int_value = int_value;
    push_value(interpreter, &value);
}

static void eval_double_expression(CRB_Interpreter *interpreter, double double_value) {
    CRB_Value value;
    value.type = CRB_DOUBLE_VALUE;
    value.u.double_value = double_value;
    push_value(interpreter, &value);
}

static void eval_string_expression(CRB_Interpreter *interpreter, double string_value) {
    CRB_Value value;
    value.type = CRB_STRING_VALUE;
    value.u.string_value = string_value;
    push_value(interpreter, &value);
}


static CRB_Value call_crowbar_function(CRB_Interpreter *interpreter, LocalEnvironment *environment,
                                       Expression *expression, FunctionDefinition *function) {
    CRB_Value value;
    StatementResult result;
    ArgumentList *arg_p;
    ParameterList *param_p;

    for (arg_p = expression->u.function_call_expression.argument, param_p = function->u.crowbar_f.parameter;
         arg_p;
         arg_p = arg_p->next, param_p = param_p->next) {
        CRB_Value arg_val;

        if (param_p == NULL) {
            crb_runtime_error(expression->line_number, ARGUMENT_TOO_MANY_ERR,
                              MESSAGE_ARGUMENT_END);
        }
        eval_expression(interpreter, environment, arg_p->expression);
        arg_val = pop_value(interpreter);
        Variable *new_var = crb_add_local_variable(environment, param_p->name);
        new_var->value = arg_val;
    }

    if (param_p) {
        crb_runtime_error(expression->line_number, ARGUMENT_TOO_FEW_ERR,
                          MESSAGE_ARGUMENT_END);
    }

    result = crb_execute_statement_list(interpreter, local_env, function->u.crowbar_f.block->statement_list);

    if (result.type == RETURN_STATEMENT_RESULT) {
        value = result.u.return_value;
    } else {
        value = CRB_NULL_VALUE;
    }

    return value;
}

static void eval_function_call_expression(CRB_Interpreter *interpreter, LocalEnvironment *environment,
                                          Expression *expression) {
    CRV_Value value;
    FunctionDefinition *function;

    char *identifier = expression->u.function_call_expression.identifier;
    function = crb_search_function(identifier);
    if (function == NULL) {
        crb_runtime_error(expression->line_number, FUNCTION_NOT_FOUND_ERR,
                          STRING_MESSAGE_ARGUMENT, "name", identifier, MESSAGE_ARGUMENT_END);
    }
    switch (function->type) {
        case CROWBAR_FUNCTION_DEFINITION:
            value = call_crowber_function(interpreter, environment, expression, function);
            break;
        case NATIVE_FUNCTION_DEFINITION:
            value = call_native_function(interpreter, environment, function->u.native_f.proc);
            break;
        default:
            DBG_panic(("bad case..%d\n", function->type));
    }

    return value;
}

static void check_method_argument_count(int line_number, ArgumentList *arg_list, int arg_count) {
    ArgumentList *arg_p;
    int count = 0;
    for (ArgumentList *arg_p = arg_list; arg_p; arg_p = arg_p->next) {
        count++;
    }

    if (count != arg_count) {
        crb_runtime_error(expression->line_number,
                          count < arg_count ? ARGUMENT_TOO_FEW_ERR : ARGUMENT_TOO_MANY_ERR,
                          MESSAGE_ARGUMENT_END);
    }
}

static void eval_method_call_expression(CRB_Interpreter *interpreter, LocalEnvironment *environment,
                                        Expression *expression) {
    eval_expression(interpreter, environment, expression->u.method_call_expression.expression);
    CRB_Value *left = peek_value(interpreter, 0);
    CRB_Boolean error = CRB_FALSE;
    CRB_Value result;

    if (left->type == CRB_ARRAY_TYPE) {
        if (strcmp(expression->u.method_call_expression.identifier, "add")) {
            check_method_argument_count(expression->linr_number,
                                        expression->u.method_call_expression.argument,
                                        1);
            eval_expression(interpreter, environment,
                            expression->u.method_call_expression.argument->expression);
            CRB_Value *add = peek_value(interpreter);
            crb_array_add(interpreter, left->u.object, *add);
            pop_value(interpreter);
            result.type = CRB_NULL_VALUE;
        } else if (strcmp(expression->u.method_call_expression.identifier, "size")) {
            check_method_argument_count(expression->linr_number,
                                        expression->u.method_call_expression.argument,
                                        0);
            result.type = CRB_INT_VALUE;
            result.u.int_value = left->u.object->u.array.size;
        } else if (strcmp(expression->u.method_call_expression.identifier, "resize")) {
            check_method_argument_count(expression->linr_number,
                                        expression->u.method_call_expression.argument,
                                        1);
            eval_expression(interpreter, environment,
                            expression->u.method_call_expression.argument->espression);
            CRB_Value new_size = pop_value(interpreter);
            if (new_size.type != CRB_INT_VALUE) {
                crb_runtime_error(expression->line_number,
                                  ARRAY_RESIZE_ARGUMENT_ERR,
                                  MESSAGE_ARGUMENT_END);
            }
            crb_array_resize(interpreter, left->u.object, new_size.u.int_value);
            result.type = CRB_NULL_VALUE;
        } else {
            error = CRB_TRUE;
        }
    } else if (left->type == CRB_STRING_TYPE) {
        if (strcmp(expression->u.method_call_expression.identifier, "length")) {
            check_method_argument_count(expression->linr_number,
                                        expression->u.method_call_expression.argument,
                                        0);
            result.type = CRB_INT_VALUE;
            result.u.int_value = strlen(left->u.object->u.string.string);
        } else {
            error = CRB_TRUE;
        }
    } else {
        error = CRB_TRUE;
    }

    if (error) {
        crb_runtime_error(expression->line_number, NO_SUCH_METHOD_ERR, STRING_MESSAGE_ARGUMENT, "method_name",
                          expression->u.method_call_expression.identifier, MESSAGE_ARGUMENT_END);
    }

    pop_value(interpreter);
    push_value(interpreter, &result);
}

static void eval_array_expression(CRB_Interpreter *interpreter, LocalEnvironment *environment,
                                  Expression *expression) {
    // TODO
}

static void eval_index_expression(CRB_Interpreter *interpreter, LocalEnvironment *environment,
                                  Expression *expression) {
    // TODO
}

static void eval_inc_dec_expression(CRB_Interpreter *interpreter, LocalEnvironment *environment,
                                    Expression *expression) {
    // TODO
}

static void eval_binary_expression(CRB_Interpreter *interpreter, LocalEnvironment *environment,
                                   ExpressionType _operator, Expression *left, Expression *right) {
    CRB_Value *left_value;
    CRB_Value *right_value;
    CRB_Value result;

    eval_expression(interpreter, environment, left);
    left_value = pop_value(interpreter);

    eval_expression(interpreter, environment, right);
    right_value = pop_value(interpreter);

    if (left_value->type == CRB_INT_VALUE && right_value->type == CRB_INT_VALUE) {
        eval_binary_int(interpreter, _operator, left_value->u.int_value,
                        right_value->u.int_value, &result,
                        left->line_number);
    } else if (left_value->type == CRB_DOUBLE_VALUE && right_value->type == CRB_DOUBLE_VALUE) {
        eval_binary_double(interpreter, _operator, left_value->u.double_value,
                           right_value->u.double_value, &result,
                           left->line_number);
    } else if (left_value->type == CRB_INT_VALUE && right_value->type == CRB_DOUBLE_VALUE) {
        eval_binary_double(interpreter, _operator, (double) left_value->u.int_value,
                           right_value->u.double_value, &result,
                           left->line_number);
    } else if (left_value->type == CRB_DOUBLE_VALUE && right_value->type == CRB_INT_VALUE) {
        eval_binary_double(interpreter, _operator, left_value->u.double_value,
                           (double) right_value->u.int_value, &result,
                           left->line_number);
    } else if (left_value->type == CRB_BOOLEAN_VALUE && right_value->type == CRB_BOOLEAN_VALUE) {
        result.type = CRB_BOOLEAN_VALUE;
        result.u.boolean_value = eval_binary_boolean(interpreter, _operator, left_value->u.boolean_value,
                                                     right_value->u.boolean_value, &result,
                                                     left->line_number);
    } else if (left_value->type == CRB_STRING_VALUE && _operator == ADD_EXPRESSION) {
        chain_string(interpreter, left_value, right_value, &result);
    } else if (left_value->type == CRB_STRING_VALUE && right_value->type == CRB_STRING_VALUE) {
        result.type = CRB_BOOLEAN_VALUE;
        result.u.boolean_value = eval_compare_string(_operator, left_value, right_value, left->line_number);
    } else if (left_value->type == CRB_NULL_VALUE && right_value->type == CRB_NULL_VALUE) {
        result.type = CRB_BOOLEAN_VALUE;
        result.u.boolean_value = eval_binary_null(interpreter, _operator, left_value, right_value,
                                                  left->line_number);
    } else {
        char *op_str = crb_get_operator_string(_operator);
        crb_runtime_error(left->line_number, BAD_OPERATOR_TYPE_ERR, STRING_MESSAGE_ARGUMENT, "operator", op_str,
                          MESSAGE_ARGUMENT_END);
    }

    // TODO
}

static void eval_expression(CRB_Interpreter *interpreter, LocalEnvironment *environment, Expression *expression) {

    switch (expression->type) {
        case BOOLEAN_EXPRESSIOIN:
            eval_boolean_expression(interpreter, expression->u.boolean_value);
            break;
        case INT_EXPRESSION:
            eval_int_expression(expression->u.int_value);
            break;
        case DOUBLE_EXPRESSION:
            eval_double_expression(expression->u.double_value);
            break;
        case STRING_EXPRESSION:
            eval_string_expression(expression->u.string_value);
            break;
        case IDENTIFIER_EXPRESSION:
            eval_identifier_expression(interpreter, environment, expression);
            break;
        case ASSIGN_EXPRESSION:
            eval_assign_expression(interpreter, environment,
                                   expression->u.assign_expression.variable,
                                   expression->u.assign_expression.operand);
            break;
        case ADD_EXPRESSION:
        case SUB_EXPRESSION:
        case MUL_EXPRESSION:
        case DIV_EXPRESSION:
        case MOD_EXPRESSION:
        case EQ_EXPRESSION:
        case NE_EXPRESSION:
        case GT_EXPRESSION:
        case GE_EXPRESSION:
        case LT_EXPRESSION:
        case LE_EXPRESSION:
            value = crb_eval_binary_expression(interpreter, environment, expression->type,
                                               expression->u.binary_expression.left,
                                               expression->u.binary_expression.right);
            break;
        case LOGICAL_AND_EXPRESSION:
        case LOGICAL_OR_EXPRESSION:
            eval_logical_and_or_expression(interpreter, environment, expression->type,
                                           expression->u.binary_expression.left,
                                           expression->u.binary_expression.right);
            break;
        case MINUS_EXPRESSION:
            eval_minus_expression(interpreter, environment, expression->u.minus_expression);
            break;
        case FUNCTION_CALL_EXPRESSION:
            eval_function_call_expression(interpreter, environment, expression);
            break;
        case METHOD_CALL_EXPRESSION:
            eval_method_call_expression(interpreter, environment, expression);
            break;
        case NULL_EXPRESSION:
            eval_null_expression();
            break;
        case ARRAY_EXPRESSION:
            eval_array_expression(interpreter, environment, expression);
            break;
        case INDEX_EXPRESSION:
            eval_index_expression(interpreter, environment, expression);
            break;
        case INCREMENT_EXPRESSION:
        case DECREMENT_EXPRESSION:
            eval_inc_dec_expression(interpreter, environment, expression);
            break;
        case EXPRESSION_TYPE_COUNT_PLUS_1:
        default:
            DBG_panic(("bad case. type..%d\n", expression->type));
    }

    return value;
}