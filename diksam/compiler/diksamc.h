//
// Created by sulvto on 18-6-7.
//

#ifndef DIKSAM_DIKSAMC_H
#define DIKSAM_DIKSAMC_H

typedef enum {
	BOOLEAN_EXPRESSION = 1,
	INT_EXPRESSION,
	DOUBLE_EXPRESSION,
	STRING_EXPRESSION,
	IDENTIFIER_EXPRESSON,
	COMMA_EXPRESSION,
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
	LOGICAL_OR_EXPRESSION,
	LOGICAL_AND_EXPRESSION,
	MINUS_EXPRESSION,
	LOGICAL_NOT_EXPRESSION,
	FUNCTION_CALL_EXPRESSION,
	INCREMENT_EXPRESSION,
	DECREMENT_EXPERSSION,
	CAST_EXPRESSION,
	EXPRESSION_KIND_COUNT_PLUS_1
} ExpressionKind;

typedef struct ArgumentList_tag {
	Expression *expression;
	struct ArgumentList_tag *next;
} ArgumentList;

typedef struct TypeSpecifier_tag TypeSpecifier;

typedef struct ParameterList_tag {
	char				*name;
	TypeSpecifier		*type;
	int					line_number;
	struct ParameterList_tag *next;
} ParameterList;

typedef enum {
	FUNCTION_DERIVE
} DeriveTag;

typedef struct {
	ParameterList *parameter_list;
} FunctionDerive;

typedef struct TypeDerive_tag {
	DeriveTag	tag;
	union {
		FunctionDerive	function_d;
	} u;
	struct TypeDerive_tag *next;
} TypeDerive;

struct TypeSpecifier_tag {
	DVM_BasicType	basic_type;
	TypeDerive		*derive;
};

typedef struct {
	char			*name;
	TypeSpecifier 	*type;
	Expression 		*initializer;
	int				variable_index;
	DVM_Boolean		is_local;
} Declaration;

typedef struct DeclarationList_tag {
	Declaration *declaration;
	struct DeclarationList_tag *next;
} DeclarationList;


typedef struct FunctionDefinition_tag FunctionDefinition;

typedef struct {
	char 		*name;
	DVM_Boolean is_function;
	union {
		FunctionDefinition function;
		Declaration			*declaration;
	} u;
} IdentifierExpression;

typedef struct {
	Expression *left;
	Expression *right;
} BinaryExpression;

typedef enum {
	NORMAL_ASSIGN = 1,
	ADD_ASSIGN,
	SUB_ASSIGN,
	MUL_ASSIGN,
	DIV_ASSIGN,
	MOD_ASSIGN
} AssignmentOperator;

typedef struct {
	AssignmentOperator operator;
	Expression *left;
	Expression *right;
} AssignExpression;

typedef struct {
	Expression 		*function;
	ArgumentList	argument;
} FunctionCallExpression;

typedef struct {
	Expression *operand;
} IncrementOrDecrement;

typedef enum {
	INT_TO_DOUBLE_CAST,
	DOUBLE_TO_INT_CAST,
	BOOLEAN_TO_STRING_CAST,
	INT_TO_STRING_CAST,
	DOUBLE_TO_STRING_CAST
} CastType;

typedef struct {
	CastType 	type;
	Expression 	*operand;
} CastExpression;

struct Expression_tag {
	TypeSpecifier *type;
	ExpressionKind kind;
	int line_number;
	union {
		DVM_Boolean				boolean_value;
		int 					int_value;
		double					double_value;
		DVM_Char				*string_value;
		IdentifierExpression	identifier;
		CommaExpression			comma;
		AssignExpression		assign_expression;
		BinaryExpression		binary_expression;
		Expression				*minus_expression;
		Expression				*logical_not;
		FunctionCallExpression	function_call_expression;
		IncrementOrDecrement	inc_dec;
		CastExpression			cast;
	} u;
};


typedef struct Statement_tag Statement;

typedef enum {
	UNDEFINED_BLOCK = 1,
	FUNCTION_BLOCK,
	WHILE_STATEMENT_BLOCK,
	FOR_STATEMENT_BLOCK
} BlockType;

typedef struct {
	Statement	*statement;
	int 		continue_label;
	int 		block_label;
} StatementBlockInfo;

typedef struct {
	FunctionDefinition 	*function;
	int 				end_label;
} FunctionBlockInfo;

typedef struct Block_tag {
	BlockType type;
	struct Block_tag *outer_block;
	StatementList *statement_list;
	DeclarationList *declaration_list;
	union {
		StatementBlockInfo statement;
		FunctionBlockInfo function;
	} parent;
} Block;

typedef struct Elseif_tag {
	Expression 			*condition;
	Block 				*block;
	struct Elseif_tag 	*next;
} Elseif;

typedef struct {
	Expression 	*condition;
	Block 		*then_block;
	Elseif		*elseif_list;
	Block 		*else_block;
} IfStatement;


typedef struct {
	char 		*label;
	Expression 	*condition;
	Block 		*block;
} WhileStatement;

typedef struct {
	char 		*label;
	Expression 	*init;
	Expression 	*condition;
	Expression 	*post;
	Block 		*block;
} ForStatement;

typedef struct {
	char 		*label;
	char 		*variable;
	Expression 	*collection;
	Block 		*block;
} ForeachStatement;

typedef struct {
	Expression *return_value;
} ReturnStatement;

typedef struct {
	char *label;
} BreakStatement;

typedef struct {
	char *label;
} ContinueStatement;

typedef struct {
	Block 	*try_block;
	Block 	catch_block;
	char 	*exception;
	Block 	*finally_block;
} TryStatement;

typedef struct {
	Expression *exception;
} ThrowStatement;

typedef enum {
    EXPRESSION_STATEMENT = 1,
    IF_STATEMENT,
    SWITCH_STATEMENT,
    WHILE_STATEMENT,
    DO_WHILE_STATEMENT,
    FOR_STATEMENT,
    FOREACH_STATEMENT,
    RETURN_STATEMENT,
    BREAK_STATEMENT,
    CONTINUE_STATEMENT,
    TRY_STATEMENT,
    THROW_STATEMENT,
    DECLARATION_STATEMENT,
    STATEMENT_TYPE_COUNT_PLUS_1,
} StatementType;

struct Statement_tag {
    StatementType   type;
    int             line_number;
    union {
        Expression 		*expression_s;
		IfStatement		if_s;
		WhileStatement	while_s;
		ForStatement 	for_s;
		ForeachStatement	foreach_s;
		BreakStatement	break_s;
		ContinueStatement	continue_s;
		ReturnStatement	return_s;
		TryStatement	try_s;
		ThrowStatement	throw_s;
		Declaration 	*declaration_s;
    } u;
};

typedef struct StatementList_tag {
    Statement                   *statement;
    struct StatementList_tag    *next;
} StatementList;

struct FunctionDefinition_tag {
	TypeSpecifier 	*type;
	char			*name;
	ParameterList	*parameter;
	Block			*block;
	int 			local_variable_count;
	Declaration		**local_variable;
	int 			index;
	struct FunctionDefinition_tag *next;
}

typedef enum {
	EUC_ENCODING = 1,
	SHIFT_JIS_ENCODING,
	UTF_8_ENCODING
} Encoding;

struct DKC_Compiler_tag {
	MEM_Storage 		compile_storage;
	int					function_count;
	FunctionDefinition 	*function_list;
	DeclarationList		*declaration_list;
	StatementList		&statement_list;
	int 				current_line_number;
	Block 				*current_block;
	DKC_InputMode		input_mode;
	Encoding			*source_encoding;
};

#endif //DIKSAM_DIKSAMC_H
