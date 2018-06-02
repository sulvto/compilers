//
// Created by sulvto on 18-5-31.
//

#ifndef CROWBAR_CROWBAR_H
#define CROWBAR_CROWBAR_H

#include "CRB_dev.h"

typedef struct ParameterList_tag {
    char                        *name;
    struct ParameterList_tag    *next;
} ParameterList;

typedef struct {
    StatementList   *statement_list;
} Block;

typedef enum {
    CROWBAR_FUNCTION_DEFINITION = 1,
    NATIVE_FUNCTION_DEFINITION
} FunctionDefinitionType;

typedef struct FunctionDefinition_tag {
    char                    *name;
    FunctionDefinitionType  type;
    union {
        struct {
            ParameterList   *parameter;
            Block           *block;
        } crowbar_f;

        struct {
            CRB_NativeFunctionProc  *proc;
        } native_f;
    } u;

    struct FunctionDefinition_tag   *next;
} FunctionDefinition;

typedef enum {
    NORMAL_STATEMENT_RESULT = 1,
    RETURN_STATEMENT_RESULT,
    BREAK_STATEMENT_RESULT,
    CONTINUE_STATEMENT_RESULT,
    STATEMENT_RESULT_TYPE_COUNT_PLUS_1
} StatementResultType;

typedef struct {
    StatementResultType type;
    union {
        CRB_Value   return_value;
    } u;
} StatementResult;

typedef struct Variable_tag {
    char    *name;
    CRB_Value   value;
    struct Variable_tag *next;
} Variable;

typedef struct GlobalVariableRef_tag {
    Variable                *variable;
    GlobalVariableRef_tag   *next;
} GlobalVariableRef;

typedef struct {
    Variable            *variable;
    GlobalVariableRef   *global_variable;
} LocalEnvironment;

typedef struct CRB_String_tag {
    int ref_count;
    char *string;
    CRB_Boolean is_literal;
} CRB_String;

typedef struct {
    int stack_alloc_size;
    int stack_pointer;
    CRB_Value *stack;
} Stack;

struct CRB_Interpreter_tag {
    MEM_Storage         interpreter_storage;
    MEM_Storage         execute_storage;
    Variable            *variable;
    FunctionDefinition  *function_list;
    StatementList       *statement_list;
    int                 current_line_number;
};


typedef enum {
    BOOLEAN_EXPRESSION  = 1,
    INT_EXPRESSION,
    DOUBLE_EXPRESSION,
    STRING_EXPRESSION,
    IDENTIFIER_EXPRESSION,
    ASSIGN_EXPRESSION,
    ADD_EXPRESSION,
    SUB_EXPRESSION,
    MUL_EXPRESSION,
    DIV_EXPRESSION,
    MOD_EXPRESSION,
    EQ_EXPRESSION,
    NE_EXPRESSION,
    GT_EXPRESSION,
    GE_EXPRESSION,
    LT_EXPRESSION,
    LE_EXPRESSION,
    LOGICAL_AND_EXPRESSION,
    LOGICAL_OR_EXPRESSION,
    MINUS_EXPRESSION,
    FUNCTION_CALL_EXPRESSION,
    NULL_EXPRESSION,
    EXPRESSION_TYPE_COUNT_PLUS_1
} ExpressionType;

typedef struct {
    Expression  *left;
    Expression  *right;
} BinaryExpression;

struct Expression_tag {
    ExpressionType type;
    int line_number;
    union {
        CRB_Boolean boolean_value;
        int         int_value;
        double      double_value;
        char        *string_value;
        char        *identifier;
        AssignExpression    assign_expression;
        BinaryExpression    binary_expression;
        Expression          *minus_expression;
        FunctionCallExpression  function_call_expression;
    } u;
} Expression;


typedef enum {
    EXPRESSION_STATEMENT = 1,
    GLOBAL_STATEMENT,
    IF_STATEMENT,
    WHILE_STATEMENT,
    FOR_STATEMENT,
    RETURN_STATEMENT,
    BREAK_STATEMENT,
    CONTINUE_STATEMENT,
    STATEMENT_TYPE_COUNT_PLUS_1,
} StatementType;

typedef struct {
    Expression  *condition;
    Block       *block;
} WhileStatement;

struct Statement_tag {
    StatementType   type;
    int line_number;
    union {
        Expression      *expression_s;
        GlobalStatement global_s;
        IfStatement     if_s;
        WhileStatement  while_s;
        ForStatement    for_s;
        ReturnStatement return_s;
    } u;
};

typedef enum {
    PARSE_ERR = 1,
    CHARACTER_INVALID_ERR,
    FUNCTION_MULTIPLE_DEFINE_ERR,
    COMPILE_ERROR_COUNT_PLUS_1
} CompileError;

typedef enum {
    VARIABLE_NOT_FOUND_ERR = 1,
    FUNCTION_NOT_FOUND_ERR,
    ARGUMENT_TOO_MANY_ERR,
    RETURN_ERROR_COUNT_PLUS_1
} RuntimeError;

struct CRB_Array_tag {
    int     size;
    int alloc_size;
    CRB_Value *array;
};

typedef enum {
    ARRAY_OBJECT = 1,
    STRING_OBJECT,
    OBJECT_TYPE_COUNT_PLUS_1
} ObjectType;

struct CRB_Object_tag {
    ObjectType type;
    unsigned int marked:1;
    union {
        CRB_Array   array;
        CRB_String string;
    } u;
    struct CRB_Object_tag *prev;
    struct CRB_Object_tag *next;
};

#endif //CROWBAR_CROWBAR_H
