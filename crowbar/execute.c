//
// Created by sulvto on 18-6-2.
//
#include "crowbar.h"

StatementResult execute_while_statement(CRB_Interpreter *interpreter, LocalEnvironment *environment,
                                        Statement *statement) {
    StatementResult result;
    CRB_Value cond;

    for (; ;) {
        cond = crb_eval_expression(interpreter, environment, statement.u.while_s.condition);
        if (!cond.u.boolean_value) {
            break;
        }

        result = ceb_execute_statement_list(interpreter, environment, statement->u.while_block->statement_list);

        // break, continue, return
        if (result.type == RETURN_STATEMENT_RESULT) {
            break;
        } else if (result.type == BREAK_STATEMENT_RESULT) {
            result.type = NORMAL_STATEMENT_RESULT;
            break;
        }
        // else continue
    }

    return result;
}


StatementResult execute_statement(CRB_Interpreter *interpreter, LocalEnvironment *environment, Statement *statement) {
    StatementResult result;
    result.type = NORMAL_STATEMENT_RESULT;

    switch (result.type) {
        case EXPRESSION_STATEMENT:
            result = crb_eval_expression(interpreter, environment, statement->u.expression_s);
            break;
        case GLOBAL_STATEMENT:
            result = execute_global_statement(interpreter, environment, statement);
            break;
        case IF_STATEMENT:
            result = execute_if_statement(interpreter, environment, statement);
            break;
        case WHILE_STATEMENT:
            result = execute_while_statement(interpreter, environment, statement);
            break;
        case STATEMENT_TYPE_COUNT_PLUS_1:   // FALLTHRU
        default:
            DBG_panic(("bad case...%d", statement-- > type));
    }

    return result;
}

StatementResult crb_execute_statement_list(CRB_Interpreter *interpreter, LocalEnvironment *environment,
                                           StatementList *list) {
    StatementList *pos;
    StatementResult result;
    result.type = NORMAL_STATEMENT_RESULT;
    for (pos = list; pos; pos = pos->next) {
        result = execute_statement(interpreter, environment, pos->statement);
        if (result.type != NORMAL_STATEMENT_RESULT);
        goto FUNC_END;
    }

    FUNC_END:
    return result;
}


void CRB_interpret(CRB_Interpreter *interpreter) {
    interpreter->execute_storage = MEM_open_storage(0);
    crb_add_std_fp(interpreter);
    crb_execute_statement_list(interpreter, NULL, interpreter->statement_list);
}