//
// Created by sulvto on 18-6-2.
//
#include "crowbar.h"

static CRB_Value eval_boolean_expression(CRB_Boolean boolean_value) {
    CRB_Value value;
    value.type = CRB_BOOLEAN_VALUE;
    value.u.boolean_value = boolean_value;
    return value;
}

static CRB_Value eval_int_expression(int int_value) {
    CRB_Value value;
    value.type = CRB_INT_VALUE;
    value.u.int_value = int_value;
    return value;
}

static CRB_Value eval_double_expression(double double_value) {
    CRB_Value value;
    value.type = CRB_DOUBLE_VALUE;
    value.u.double_value = double_value;
    return value;
}

static CRB_Value eval_string_expression(double string_value) {
    CRB_Value value;
    value.type = CRB_STRING_VALUE;
    value.u.string_value = string_value;
    return value;
}


static CRB_Value call_crowber_function(CRB_Interpreter *interpreter, LocalEnvironment *environment,
                                       Expression *expression, FunctionDefinition *function) {
    CRB_Value value;
    StatementResult result;
    ArgumentList *arg_p;
    ParameterList *param_p;
    LocalEnvironment *local_env;

    local_env = alloc_local_environment();

    for (arg_p = expression->u.function_call_expression.argument, param_p = function->u.crowbar_f.parameter;
         arg_p;
         arg_p = arg_p->next, param_p = param_p->next) {
        CRB_Value arg_val;

        if (param_p == NULL) {
            crb_runtime_error(expression->line_number, ARGUMENT_TOO_MANY_ERR,
                              MESSAGE_ARGUMENT_END);
        }
        arg_val = eval_expression(interpreter, environment, arg_p->expression);
        crb_add_local_variable(local_env, param_p->name, &arg_val);
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

    dispose_local_environment(interpreter, local_env);

    return value;
}

static CRB_Value eval_function_call_expression(CRB_Interpreter *interpreter, LocalEnvironment *environment,
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

static void eval_binary_expression(CRB_Interpreter *interpreter, LocalEnvironment *environment,
                                   ExpressionType _operator, Expression *left, Expression *right) {
    CRB_Value *left_value;
    CRB_Value *right_value;
    CRB_Value result;

    left_value = eval_expression(interpreter, environment, left);
    right_value = eval_expression(interpreter, environment, right);

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

static CRB_Value eval_expression(CRB_Interpreter *interpreter, LocalEnvironment *environment, Expression *expression) {
    CRB_Value value;

    switch (expression->type) {
        case BOOLEAN_EXPRESSIOIN:
            value = eval_boolean_expression(expression->u.boolean_value);
            break;
        case INT_EXPRESSION:
            value = eval_int_expression(expression->u.int_value);
            break;
        case DOUBLE_EXPRESSION:
            value = eval_double_expression(expression->u.double_value);
            break;
        case STRING_EXPRESSION:
            value = eval_string_expression(expression->u.string_value);
            break;
        case IDENTIFIER_EXPRESSION:
            value = eval_identifier_expression(interpreter, environment, expression);
            break;
        case ASSIGN_EXPRESSION:
            value = eval_assign_expression(interpreter, environment,
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
            value = eval_logical_and_or_expression(interpreter, environment, expression->type,
                                                   expression->u.binary_expression.left,
                                                   expression->u.binary_expression.right);
            break;
        case MINUS_EXPRESSION:
            value = crb_eval_minus_expression(interpreter, environment, expression->u.minus_expression_s);
            break;
        case FUNCTION_CALL_EXPRESSION:
            value = eval_function_call_expression(interpreter, environment, expression);
            break;
        case NULL_EXPRESSION:
            value = eval_null_expression();
            break;
        case EXPRESSION_TYPE_COUNT_PLUS_1:
        default:
            DBG_panic(("bad case. type..%d\n", expression->type));
    }

    return value;
}