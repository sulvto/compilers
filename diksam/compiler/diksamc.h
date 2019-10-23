//
// Created by sulvto on 18-6-7.
//

#ifndef DIKSAM_DIKSAMC_H
#define DIKSAM_DIKSAMC_H

#include "MEM.h"
#include "DKC.h"
#include "DVM_code.h"
#include "share.h"

typedef struct TypeSpecifier_tag TypeSpecifier;
typedef struct ClassDefinition_tag ClassDefinition;
typedef struct FunctionDefinition_tag FunctionDefinition;
typedef struct MemberDeclaration_tag MemberDeclaration;
typedef struct Expression_tag Expression;

#define DEFAULT_CONSTRUCTOR_NAME	("init")

#define smaller(a, b)   ((a) < (b) ? (a) : (b))

#define LINE_BUF_SIZE			(1024)
#define MESSAGE_ARGUMENT_MAX    (256)

#define ABSTRACT_METHOD_INDEX   (-1)


typedef struct {
	char *format;
} ErrorDefinition;

typedef enum {
	INT_MESSAGE_ARGUMENT = 1,
	DOUBLE_MESSAGE_ARGUMENT,
	STRING_MESSAGE_ARGUMENT,
	CHARACTER_MESSAGE_ARGUMENT,
	POINTER_MESSAGE_ARGUMENT,
	MESSAGE_ARGUMENT_END
} MessageArgumentType;

typedef enum {
	PARSE_ERR = 1,
	CHARACTER_INVALID_ERR,
	FUNCTION_MULTIPLE_DEFINE_ERR,
	BAD_MULTIBYTE_CHARACTER_ERR,
	LABEL_NOT_FOUND_ERR,
	PARAMETER_MULTIPLE_DEFINE_ERR,
	VARIABLE_MULTIPLE_DEFINE_ERR,
	PACKAGE_NAME_TOO_LONG_ERR,
	CAST_MISMATCH_ERR,
	MATH_TYPE_MISMATCH_ERR,
	COMPARE_TYPE_MISMATCH_ERR,
	LOGICAL_TYPE_MISMATCH_ERR,
	INC_DEC_TYPE_MISMATCH_ERR,
	FUNCTION_NOT_FOUND_ERR,
	ARGUMENT_COUNT_MISMATCH_ERR,
	IDENTIFIER_NOT_FOUND_ERR,
	FIELD_CAN_NOT_CALL_ERR,
	METHOD_IS_NOT_CALLED_ERR,
	REQUIRE_DUPLICATE_ERR,
	REQUIRE_FILE_NOT_FOUND_ERR,
	REQUIRE_ITSELF_ERR,
	ARRAY_LITERAL_EMPTY_ERR,
	ABSTRACT_MULTIPLE_SPECIFIED_ERR,
	ACCESS_MODIFIER_MULTIPLE_SPECIFIED_ERR,
	OVERRIDE_MODIFIER_MULTIPLE_SPECIFIED_ERR,
	VIRTUAL_MODIFIER_MULTIPLE_SPECIFIED_ERR,
	PRIVATE_MEMBER_ACCESS_ERR,
	PACKAGE_MEMBER_ACCESS_ERR,
	MEMBER_NOT_FOUND_ERR,
	MEMBER_EXPRESSION_TYPE_ERR,
	ABSTRACT_METHOD_HAS_BODY_ERR,
	CONCRETE_METHOD_HAS_NO_BODY_ERR,
	MULTIPLE_INHERITANCE_ERR,
	CLASS_NOT_FOUND_ERR,
	INHERIT_CONCRETE_CLASS_ERR,
	RENAME_HAS_NO_PACKAGED_NAME_ERR,
	CONSTRUCTOR_IS_FIELD_ERR,
	NOT_CONSTRUCTOR_ERR,
	NEW_ABSTRACT_CLASS_ERR,
	RETURN_IN_VOID_FUNCTION_ERR,
	NOT_LVALUE_ERR,
	DIVISION_BY_ZERO_IN_COMPILE_ERR,
	ARRAY_SIZE_NOT_INT_ERR,
	INDEX_NOT_INT_ERR,
	INDEX_LEFT_OPERAND_NOT_ARRAY_ERR,
	NON_VIRTUAL_METHOD_OVERRIDED_ERR,
	NEED_OVERRIDE_ERR,
	ABSTRACT_METHOD_IN_CONCRETE_CLASS_ERR,
	HASNT_SUPER_CLASS_ERR,
	SUPER_NOT_IN_MEMBER_EXPRESSION_ERR,
	FIELD_OF_SUPER_REFERENCED_ERR,
	FIELD_NAME_DUPLICATE_ERR,
	FIELD_OVERRIDED_ERR,
	ARRAY_METHOD_NOT_FOUND_ERR,
	STRING_METHOD_NOT_FOUND_ERR,
	INSTANCEOF_OPERAND_NOT_REFERENCE_ERR,
	INSTANCEOF_TYPE_NOT_REFERENCE_ERR,
	INSTANCEOF_MUST_RETURN_TRUE_ERR,
	INSTANCEOF_FOR_NOT_CLASS_ERR,
	INSTANCEOF_MUST_RETURN_FALSE_ERR,
	DOWN_CAST_OPERAND_IS_NOT_CLASS_ERR,
	DOWN_CAST_TARGET_IS_NOT_CLASS_ERR,
	DOWN_CAST_DO_NOTHING_ERR,
	DOWN_CAST_TO_BAD_CLASS_ERR,
    CATCH_TYPE_IS_NOT_CLASS_ERR,
    CATCH_TYPE_IS_NOT_EXCEPTION_ERR,
    THROW_TYPE_IS_NOT_CLASS_ERR,
    THROW_TYPE_IS_NOT_EXCEPTION_ERR,
    RETHROW_OUT_OF_CATCH_ERR,
    THROWS_TYPE_NOT_FOUND_ERR,
    THROWS_TYPE_ID_NOT_EXCEPTION_ERR,
    EXCEPTION_HAS_TO_BE_THROWN_ERR,
	IF_CONDITION_NOT_BOOLEAN_ERR,
	WHILE_CONDITION_NOT_BOOLEAN_ERR,
	FOR_CONDITION_NOT_BOOLEAN_ERR,
	OVERRIDE_METHOD_ACCESSIBILITY_ERR,
	BAD_PARAMETER_TYPE_ERR,
	BAD_RETURN_TYPE_ERR,
	BAD_PARAMETER_COUNT_ERR,
	TYPE_NAME_NOT_FOUND_ERR,
	INTERFACE_INHERIT_ERR,
	CONSTRUCTOR_CALLED_ERR,
	PACKAGE_CLASS_ACCESS_ERR,
	THIS_OUT_OF_CLASS_ERR,
	SUPER_OUT_OF_CLASS_ERR,
	EOF_IN_C_COMMENT_ERR,
	EOF_IN_STRING_LITERAL_ERR,
	TOO_LONG_CHARACTER_LITERAL_ERR,
    ASSIGN_TO_METHOD_ERR,
	COMPILE_ERROR_COUNT_PLUS_1
} CompileError;

typedef enum {
	BOOLEAN_EXPRESSION = 1,
	INT_EXPRESSION,
	DOUBLE_EXPRESSION,
	STRING_EXPRESSION,
	IDENTIFIER_EXPRESSION,
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
	NULL_EXPRESSION,
	THIS_EXPRESSION,
	SUPER_EXPRESSION,
	ARRAY_LITERAL_EXPRESSION,
	INDEX_EXPRESSION,
	MEMBER_EXPRESSION,
	INCREMENT_EXPRESSION,
	DECREMENT_EXPRESSION,
	INSTANCEOF_EXPRESSION,
	DOWN_CAST_EXPRESSION,
	CAST_EXPRESSION,
	UP_CAST_EXPRESSION,
	NEW_EXPRESSION,
	ARRAY_CREATION_EXPRESSION,
	EXPRESSION_KIND_COUNT_PLUS_1
} ExpressionKind;

#define dkc_is_class_object(type) ((type)->basic_type == DVM_CLASS_TYPE && (type)->derive == NULL)

#define dkc_is_object(type) (dkc_is_string(type) || dkc_is_array(type) || dkc_is_class_object(type))

#define dkc_is_delegate(type) ((type)->basic_type == DVM_DELEGATE_TYPE && (type)->derive == NULL)

#define dkc_is_array(type)  ((type)->derive && ((type)->derive->tag == ARRAY_DERIVE))

#define dkc_is_string(type) ((type)->basic_type == DVM_STRING_TYPE && (type)->derive == NULL)

#define dkc_is_int(type)    ((type)->basic_type == DVM_INT_TYPE && (type)->derive == NULL)

#define dkc_is_double(type) ((type)->basic_type == DVM_DOUBLE_TYPE && (type)->derive == NULL)

#define dkc_is_boolean(type) ((type)->basic_type == DVM_BOOLEAN_TYPE && (type)->derive == NULL)


typedef struct PackageName_tag {
	char 	*name;
	struct PackageName_tag 	*next;
} PackageName;

typedef enum {
	DKH_SOURCE,
	DKM_SOURCE
} SourceSuffix;

typedef struct RequireList_tag {
	PackageName		*package_name;
	int 			line_number;
	struct RequireList_tag *next;
} RequireList;

typedef struct RenameList_tag {
	PackageName *package_name;
	char 		*original_name;
	char 		*renamed_name;
	int 		line_number;
	struct RequireList_tag *next;
} RenameList;

typedef struct ArgumentList_tag {
	Expression *expression;
	struct ArgumentList_tag *next;
} ArgumentList;

typedef struct ParameterList_tag {
	char				*name;
	TypeSpecifier		*type;
	int					line_number;
	struct ParameterList_tag *next;
} ParameterList;

typedef struct {
    char *identifier;
    ClassDefinition *class_definition;
    int line_number;
} ExceptionRef;

typedef struct ExceptionList_tag {
    ExceptionRef *ref;
    struct ExceptionList_tag *next;
} ExceptionList;

typedef enum {
	FUNCTION_DERIVE,
	ARRAY_DERIVE
} DeriveTag;

typedef struct {
	ParameterList *parameter_list;
    ExceptionList *throws;
} FunctionDerive;

typedef struct {
	int 	dummy;	//
} ArrayDerive;

typedef struct TypeDerive_tag {
	DeriveTag	tag;
	union {
		FunctionDerive	function_derive;
		ArrayDerive		array_derive;
	} u;
	struct TypeDerive_tag *next;
} TypeDerive;

struct TypeSpecifier_tag {
	DVM_BasicType	basic_type;
    char            *identifier;
	union {
        struct {
            ClassDefinition *class_definition;
            int class_index;
        } class_ref;
        struct {
            // TODO:enum
            // ClassDefinition *class_definition;
            int enum_index;
        } enum_ref;
    } u;
	int             line_number;
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

typedef enum {
	VARIABLE_IDENTIFIER,
	FUNCTION_IDENTIFIER
} IdentifierKind;

typedef struct {
	FunctionDefinition  *function_definition;
	int                 function_index;
} FunctionIdentifier;

typedef struct {
	char 		*name;
	IdentifierKind  kind;
	union {
		FunctionIdentifier  function;
		Declaration			*declaration;
	} u;
} IdentifierExpression;

typedef struct {
	Expression *left;
	Expression *right;
} CommaExpression;

typedef struct {
	Expression *array;
	Expression *index;
} IndexExpression;

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
	AssignmentOperator  operator;
	Expression          *left;
	Expression          *operand;
} AssignExpression;

typedef struct {
	Expression 		*function;
	ArgumentList	*argument;
} FunctionCallExpression;

typedef struct {
	Expression          *expression;
	char                *member_name;
	MemberDeclaration   *declaration;
	int                 method_index;
} MemberExpression;

typedef struct ExpressionList_tag {
	Expression 		*expression;
	struct ExpressionList_tag *next;
} ExpressionList;

typedef struct {
	Expression *operand;
} IncrementOrDecrement;

typedef struct {
	Expression      *operand;
	TypeSpecifier   *type;
} InstanceofExpression;

typedef struct {
	Expression      *operand;
	TypeSpecifier   *type;
} DownCastExpression;

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

typedef struct  {
	ClassDefinition *interface_definition;
	Expression      *operand;
	int             interface_index;
} UpCastExpression;

typedef struct {
	char            *class_name;
	ClassDefinition *class_definition;
	int             class_index;
	char            *method_name;
	MemberDeclaration *method_declaration;
	ArgumentList    *argument;
} NewExpression;

typedef struct ArrayDimension_tag {
	Expression *expression;
	struct ArrayDimension_tag *next;
} ArrayDimension;

typedef struct {
	TypeSpecifier 	*type;
	ArrayDimension	*dimension;
} ArrayCreation;

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
		MemberExpression		member_expression;
		ExpressionList 			*array_literal;
		IndexExpression 		index_expression;
		IncrementOrDecrement	inc_dec;
		InstanceofExpression    instanceof_expression;
		DownCastExpression      down_cast;
		CastExpression			cast;
		UpCastExpression        up_cast;
		NewExpression           new_expression;
		ArrayCreation 			array_creation;
	} u;
};


typedef struct Statement_tag Statement;

typedef enum {
	UNDEFINED_BLOCK = 1,
	FUNCTION_BLOCK,
	WHILE_STATEMENT_BLOCK,
	FOR_STATEMENT_BLOCK,
    TRY_CLAUSE_BLOCK,
    CATCH_CLAUSE_BLOCK,
    FINALLY_CLAUSE_BLOCK
} BlockType;

typedef struct {
	Statement	*statement;
	int 		continue_label;
	int 		break_label;
} StatementBlockInfo;

typedef struct {
	FunctionDefinition 	*function;
	int 				end_label;
} FunctionBlockInfo;

typedef struct StatementList_tag {
	Statement                   *statement;
	struct StatementList_tag    *next;
} StatementList;

typedef struct Block_tag {
	BlockType           type;
	struct Block_tag    *outer_block;
	StatementList       *statement_list;
	DeclarationList     *declaration_list;
	union {
		StatementBlockInfo  statement;
		FunctionBlockInfo   function;
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

typedef struct CatchClause_tag {
	TypeSpecifier   *type;
	char            *variable_name;
	Declaration     *variable_declaration;
	Block           *block;
	int             line_number;
	struct CatchClause_tag *next;
} CatchClause;

typedef struct {
	Block 	*try_block;
	CatchClause *catch_clause;
	Block 	*finally_block;
} TryStatement;

typedef struct {
	Expression *exception;
    Declaration *variable_declaration;
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
    STATEMENT_TYPE_COUNT_PLUS_1
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
		ThrowStatement  throw_s;
		Declaration 	*declaration_s;
    } u;
};

struct FunctionDefinition_tag {
	TypeSpecifier 	*type;
	PackageName 	*package_name;
	char			*name;
	ParameterList	*parameter;
	Block			*block;
	int 			local_variable_count;
	Declaration		**local_variable;
	ClassDefinition	*class_definition;
    ExceptionList   *throws;
	int 			end_line_number;
	struct FunctionDefinition_tag *next;
};

typedef struct ExtendsList_tag {
	char 				*identifier;
	ClassDefinition 	*class_definition;
	struct ExtendsList_tag *next;
} ExtendsList;

typedef enum {
	METHOD_MEMBER,
	FIELD_MEMBER
} MemberKind;

typedef struct {
	DVM_Boolean		is_constructor;
	DVM_Boolean 	is_abstract;
	DVM_Boolean 	is_virtual;
	DVM_Boolean 	is_override;
	FunctionDefinition *function_definition;
	int 			method_index;
} MethodMember;

typedef struct {
	char            *name;
	TypeSpecifier   *type;
    DVM_Boolean     is_final;
    Expression      *initializer;
	int             field_index;
} FieldMember;

struct MemberDeclaration_tag {
	MemberKind kind;
	DVM_AccessModifier access_modifier;
	union {
		MethodMember method;
		FieldMember field;
	} u;
	int line_number;
	struct MemberDeclaration_tag *next;
};

struct ClassDefinition_tag {
	DVM_Boolean 		is_abstract;
	DVM_AccessModifier 		access_modifier;
	DVM_ClassOrInterface	class_or_interface;
	PackageName			*package_name;
	char 				*name;
	ExtendsList 		*extends;
	ClassDefinition		*super_class;
	ExtendsList			*interface_list;
	MemberDeclaration 	*member;
	int 				line_number;
	struct ClassDefinition_tag *next;
};

typedef enum {
	DKC_FILE_INPUT_MODE = 1,
	DKC_STRING_INPUT_MODE
} DKC_InputMode;

typedef enum {
	EUC_ENCODING = 1,
	SHIFT_JIS_ENCODING,
	UTF_8_ENCODING
} Encoding;

typedef struct {
    DKC_InputMode input_mode;
    union {
        struct {
            FILE *fp;
        } file;
        struct {
            char **lines;
        } string;
    } u;
} SourceInput;

typedef struct CompilerList_tag {
    DKC_Compiler    *compiler;
    struct CompilerList_tag *next;
} CompilerList;

struct DKC_Compiler_tag {
	MEM_Storage 		compile_storage;
	PackageName			*package_name;
	SourceSuffix		source_suffix;
	char 				*path;
	RequireList			*require_list;
	RenameList			*rename_list;
	FunctionDefinition 	*function_list;
	int					dvm_function_count;
	DVM_Function		*dvm_function;
	int 				dvm_class_count;
	DVM_Class			*dvm_class;
	DeclarationList		*declaration_list;
	StatementList		*statement_list;
	ClassDefinition		*class_definition_list;
	int 				current_line_number;
	Block 				*current_block;
	ClassDefinition		*current_class_definition;
	TryStatement		*current_try_statement;
	CatchClause         *current_catch_clause;
	int 				current_finally_label;
	DKC_InputMode		input_mode;
	CompilerList		*required_list;
	int 				array_method_count;
	FunctionDefinition	*array_method;
	int 				string_method_count;
	FunctionDefinition	*string_method;
	Encoding			source_encoding;
};

typedef enum {
	ABSTRACT_MODIFIER,
	PUBLIC_MODIFIER,
	PRIVATE_MODIFIER,
	OVERRIDE_MODIFIER,
	VIRTUAL_MODIFIER,
	NOT_SPECIFIED_MODIFIER
} ClassOrMemberModifierKind;

typedef struct {
	ClassOrMemberModifierKind   is_abstract;
	ClassOrMemberModifierKind   access_modifier;
	ClassOrMemberModifierKind   is_virtual;
	ClassOrMemberModifierKind   is_override;
} ClassOrMemberModifierList;

typedef struct {
	char *string;
} VString;

typedef struct {
	DVM_Char *string;
} VWString;

typedef struct {
    char *package_name;
    SourceSuffix suffix;
    char **source_string;    
} BuiltinScript;

// diksam.l
void dkc_set_source_string(char **source);

// create.c
DeclarationList *dkc_chain_declaration(DeclarationList *list,
									   Declaration *declaration);

Declaration *dkc_alloc_declaration(TypeSpecifier *type, char *identifier);

PackageName *dkc_create_package_name(char *identifier);

PackageName *dkc_chain_package_name(PackageName *list, char *identifier);

RequireList *dkc_create_require_list(PackageName *package_name);

RequireList *dkc_chain_require_list(RequireList *list, RequireList *add);

RenameList *dkc_create_rename_list(PackageName *package_name, char *identifier);

RenameList *dkc_chain_rename_list(RenameList *list, RenameList *add);

void dkc_set_require_and_rename_list(RequireList *require_list, RenameList *rename_list);


FunctionDefinition *dkc_create_function_definition(TypeSpecifier *type, 
                                                    char *identifier,
                                                    ParameterList *parameter_list,
                                                    ExceptionList *throws,
                                                    Block *block);

void dkc_function_define(TypeSpecifier *type, char *identifier, 
                        ParameterList *parameter_list, ExceptionList *throws,
						 Block *block);

ParameterList *dkc_create_parameter(TypeSpecifier *type, char *identifier);

ParameterList *dkc_chain_parameter(ParameterList *list, TypeSpecifier *type, char *identifier);

ArgumentList *dkc_create_argument_list(Expression *expression);

ArgumentList *dkc_chain_argument_list(ArgumentList *list, Expression *expression);

ExpressionList *dkc_create_expression_list(Expression *expression);

ExpressionList *dkc_chain_expression_list(ExpressionList *list, Expression *expression);

StatementList *dkc_create_statement_list(Statement *statement);

StatementList *dkc_chain_statement_list(StatementList *list, Statement *statement);

TypeSpecifier *dkc_create_type_specifier(DVM_BasicType basic_type);

TypeSpecifier *dkc_create_identifier_type_specifier(char *identifier);

TypeSpecifier *dkc_create_array_type_specifier(TypeSpecifier *base);

Expression *dkc_alloc_expression(ExpressionKind kind);

Expression *dkc_create_comma_expression(Expression *left, Expression *right);

Expression *dkc_create_binary_expression(ExpressionKind _operator, Expression *left, Expression *right);

Expression *dkc_create_minus_expression(Expression *operand);

Expression *dkc_create_instanceof_expression(Expression *operand, TypeSpecifier *type);

Expression *dkc_create_identifier_expression(char *identifier);

Expression *dkc_create_function_call_expression(Expression *operand, ArgumentList *arguments);

Expression *dkc_create_down_cast_expression(Expression *operand, TypeSpecifier *type);

Expression *dkc_create_member_expression(Expression *expression, char *member_name);

Expression *dkc_create_boolean_expression(DVM_Boolean value);

Expression *dkc_create_null_expression(void);

Expression *dkc_create_new_expression(char *class_name, char *method_name,
									  ArgumentList *argument);

Expression *dkc_create_array_literal_expression(ExpressionList *list);

Expression *dkc_create_basic_array_creation(DVM_BasicType basic_type,
											  ArrayDimension *dim_expr_list,
											  ArrayDimension *dim_list);

Expression *dkc_create_class_array_creation(TypeSpecifier *type,
											  ArrayDimension *dim_expr_list,
											  ArrayDimension *dim_list);

Expression *dkc_create_this_expression(void);

Expression *dkc_create_super_expression(void);

ArrayDimension *dkc_create_array_dimension(Expression *expression);

ArrayDimension *dkc_chain_array_dimension(ArrayDimension *list, ArrayDimension *dim);

Statement *dkc_alloc_statement(StatementType type);

Statement *dkc_create_if_statement(Expression *condition, Block *then_block,
                                   Elseif *elseif_list, Block *else_block);

Elseif *dkc_chain_elseif_list(Elseif *list, Elseif *add);

Elseif *dkc_create_elseif(Expression *expression, Block *block);

Statement *dkc_create_while_statement(char *label, Expression *condition, Block *block);

Statement *dkc_create_foreach_statement(char *label, char *variable, Expression *collection, Block *block);

Statement *dkc_create_for_statement(char *label, Expression *init, Expression *condition, Expression *post, Block *block);

Statement *dkc_create_do_while_statement(char *label, Block *block, Expression *condition);

Block *dkc_alloc_block(void);

Block *dkc_open_block(void);

Block *dkc_close_block(Block *block, StatementList *statement_list);

Statement *dkc_create_expression_statement(Expression *expression);

Statement *dkc_create_return_statement(Expression *expression);

Statement *dkc_create_break_statement(char *label);

Statement *dkc_create_continue_statement(char *label);

Statement *dkc_create_try_statement(Block *try_block, CatchClause *catch_clause, Block *finally_block);

CatchClause *dkc_create_catch_clause(TypeSpecifier *type, char *variable_name, Block *block);

CatchClause *dkc_start_catch_clause(void);

CatchClause *dkc_end_catch_clause(CatchClause *catch_clause, TypeSpecifier *type, char *variable_name, Block *block);

CatchClause *dkc_chain_catch_list(CatchClause *list, CatchClause *add);

Statement *dkc_create_throw_statement(Expression *expression);

Statement *dkc_create_declaration_statement(TypeSpecifier *type, char *identifier, Expression *initializer);

void dkc_start_class_definition(ClassOrMemberModifierList *modifier, DVM_ClassOrInterface class_or_interface,
                                char *identifier,
                                ExtendsList *extends);

void dkc_class_define(MemberDeclaration *member_list);

ExtendsList *dkc_create_extends_list(char *identifier);

ExtendsList *dkc_chain_extends_list(ExtendsList *list, char *add);

ClassOrMemberModifierList dkc_create_class_or_member_modifier(ClassOrMemberModifierKind modifier);

ClassOrMemberModifierList dkc_chain_class_or_member_modifier(ClassOrMemberModifierList list,
                                                             ClassOrMemberModifierList add);

MemberDeclaration *dkc_create_member_declaration(ClassOrMemberModifierList *modifier,
                                                 FunctionDefinition *function_definition, DVM_Boolean is_constructor);

MemberDeclaration *dkc_chain_member_declaration(MemberDeclaration *list, MemberDeclaration *add);

FunctionDefinition *dkc_method_function_define(TypeSpecifier *type, char *identifier, 
                                                ParameterList *parameter_list,
                                                ExceptionList *throws,
                                                Block *block);

FunctionDefinition *dkc_constructor_function_define(char *identifier, 
                                                    ParameterList *parameter_list, 
                                                    ExceptionList *throws,
                                                    Block *block);

MemberDeclaration *dkc_create_field_member(ClassOrMemberModifierList *modifier, DVM_Boolean is_final, TypeSpecifier *type, char *name, Expression *initializer);

MemberDeclaration *dkc_create_method_member(ClassOrMemberModifierList *modifier,
                                            FunctionDefinition *function_definition, DVM_Boolean is_constructor);
ExceptionList *dkc_create_throws(char *identifier);
ExceptionList *dkc_chain_exception_list(ExceptionList *list, char *identifier);


// string.c
char *dkc_create_identifier(char *string);

void dkc_open_string_literal(void);

void dkc_add_string_literal(int letter);

void dkc_reset_string_literal_buffer(void);

DVM_Char *dkc_close_string_literal(void);

int dkc_close_character_literal(void);

// fix_tree.c
void dkc_fix_tree(DKC_Compiler *compiler);

// generate.c
DVM_TypeSpecifier *dkc_copy_type_specifier(TypeSpecifier *src);

DVM_Executable *dkc_generate(DKC_Compiler *compiler);


// util.c
DKC_Compiler *dkc_get_current_compiler(void);

void dkc_set_current_compiler(DKC_Compiler *compiler);

void *dkc_malloc(size_t size);

char *dkc_strdup(char *src);

TypeSpecifier *dkc_alloc_type_specifier(DVM_BasicType type);

TypeDerive *dkc_alloc_type_derive(DeriveTag derive_tag);

TypeSpecifier *dkc_alloc_type_specifier2(TypeSpecifier *src);

DVM_Boolean dkc_compare_parameter(ParameterList *parameter1, ParameterList *parameter2);

DVM_Boolean dkc_compare_type(TypeSpecifier *type1, TypeSpecifier *type2);

DVM_Boolean dkc_compare_package_name(PackageName *package1, PackageName *package2);

FunctionDefinition *dkc_search_function(char *name);

Declaration *dkc_search_declaration(char *identifier, Block *block);

ClassDefinition *dkc_search_class(char *identifier);

MemberDeclaration *dkc_search_member(ClassDefinition *class_definition, char *member_name);

void dkc_vstr_clear(VString *v);

void dkc_vstr_append_string(VString *v, char *string);

void dkc_vstr_append_character(VString *v, int ch);

void dkc_vwstr_clear(VWString *v);

void dkc_vwstr_append_string(VWString *v, DVM_Char *string);

void dkc_vwstr_append_character(VWString *v, int ch);

char *dkc_get_type_name(TypeSpecifier *type);

char *dkc_get_basic_type_name(DVM_BasicType type);

DVM_Char *dkc_expression_to_string(Expression *expression);

char *dkc_package_name_to_string(PackageName *package_name);

// wchar.c
size_t dkc_wcslen(DVM_Char *str);

DVM_Char *dkc_wcscpy(DVM_Char *dest, DVM_Char *src);

wchar_t *dkc_mbstowcs_alloc(int line_number, const char *src);


// error.c
void dkc_compile_error(int line_number, CompileError id, ...);


#endif //DIKSAM_DIKSAMC_H
