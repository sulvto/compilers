//
// Created by sulvto on 18-6-11.
//

#ifndef DIKSAM_DVM_PRI_H
#define DIKSAM_DVM_PRI_H

#include "DVM_code.h"
#include "DVM_dev.h"
#include "share.h"

#define STACK_ALLOC_SIZE    (4096)
#define HEAP_THRESHOLD_SIZE (1024 * 256)

#define NULL_STRING         (L"true")
#define TRUE_STRING         (L"true")
#define FALSE_STRING        (L"false")
#define BUILT_IN_METHOD_PACKAGE_NAME ("$built_in")

#define NO_LINE_NUMBER_PC   (-1)
#define FUNCTION_NOT_FOUND  (-1)
#define FIELD_NOT_FOUND  (-1)
#define CALL_FROM_NATIVE    (-1)

#define LINE_BUF_SIZE           (1024)
#define MESSAGE_ARGUMENT_MAX    (256)

#define GET_2BYTE_INT(p) (((p)[0] << 8) + (p)[1])
#define SET_2BYTE_INT(p, value) (((p)[0] = (value) >> 8), ((p)[1] = value & 0xff))

#define is_pointer_type(type) ((type)->basic_type == DVM_STRING_TYPE \
								|| (type)->basic_type == DVM_CLASS_TYPE \
								|| ((type)->basic_type == DVM_NULL_TYPE \
								|| (type)->derive_count > 0 \
									&& (type)->derive[0].tag == DVM_ARRAY_DERIVE))

#define is_object_null(obj) ((obj).data == NULL)

typedef struct ExecutableEntry_tag ExecutableEntry;

typedef enum {
	BAD_MULTIBYTE_CHARACTER_ERR = 1,
	FUNCTION_NOT_FOUND_ERR,
	FUNCTION_MULTIPLE_DEFINE_ERR,
	INDEX_OUT_OF_BOUNDS_ERR,
	NULL_POINTER_ERR,
	LOAD_FILE_NOT_FOUND_ERR,
	LOAD_FILE_ERR,
	CLASS_MULTIPLE_DEFINE_ERR,
	CLASS_NOT_FOUND_ERR,
	DYNAMIC_LOAD_WITHOUT_PACKAGE_ERR,
	RUNTIME_ERROR_COUNT_PLUS_1
} RuntimeError;

typedef struct {
	DVM_Char *string;
} VString;

typedef enum {
	NATIVE_FUNCTION,
	DIKSAM_FUNCTION
} FunctionKind;

typedef struct {
	DVM_NativeFunctionProc  *proc;
	int 		            argument_count;
	DVM_Boolean             is_method;
	DVM_Boolean             return_pointer;
} NativeFunction;

typedef struct {
	ExecutableEntry 	*executable;
	int 			index;
} DiksamFunction;

typedef struct {
	char            *name;
	char            *package_name;
	FunctionKind    kind;
	DVM_Boolean     is_implemented;

	union {
		NativeFunction native_function;
		DiksamFunction diksam_function;
	} u;
} Function;

typedef struct {
	Function    *caller;
	int         caller_address;
	int         base;
} CallInfo;

#define revalue_up_align(val)   ((val) ? (((val) - 1) / sizeof(DVM_Value) + 1) : 0)
#define CALL_INFO_ALIGN_SIZE    (revalue_up_align(sizeof(CallInfo)))

typedef enum {
	STRING_OBJECT = 1,
	ARRAY_OBJECT,
	CLASS_OBJECT,
	OBJECT_TYPE_COUNT_PLUS_1
} ObjectType;

struct DVM_String_tag {
	int         length;
	DVM_Boolean is_literal;
	DVM_Char    *string;
};

typedef enum {
	INT_ARRAY = 1,
	DOUBLE_ARRAY,
	OBJECT_ARRAY,
	FUNCTION_INDEX_ARRAY
} ArrayType;

typedef struct DVM_Array_tag {
	ArrayType 	type;
	int			size;
	int 		alloc_size;
	union {
		int 	*int_array;
		double 	*double_array;
		DVM_ObjectRef *object;
		int     *function_index;
	} u;
};

typedef struct {
	int field_count;
	DVM_Value *field;
} DVM_ClassObject;

struct DVM_Object_tag {
	ObjectType type;
	unsigned int marked:1;
	union {
		DVM_String 	    string;
		DVM_Array 	    array;
		DVM_ClassObject class_object;
	} u;
	struct DVM_Object_tag *prev;
	struct DVM_Object_tag *next;
};

typedef struct {
	int 		alloc_size;
	int 		stack_pointer;
	DVM_Value 	*stack;
	DVM_Boolean *pointer_flags;
} Stack;

typedef struct {
	int 	current_heap_size;
	int 	current_threshold;
	DVM_Object *header;
} Heap;

typedef struct {
	int 		variable_count;
	DVM_Value 	*variable;
} Static;

typedef struct ExecutableClass_tag {
    DVM_Class           *dvm_class;
    ExecutableEntry     *executable;
    char                *package_name;
    char                *name;
    DVM_Boolean         is_implemented;
    int                 class_index;
    struct ExecutableClass_tag *super_class;
    DVM_VTable          *class_table;
    int                 interface_count;
    struct ExecutableClass_tag  **interface;
    DVM_VTable          **interface_v_table;
    int                 field_count;
    DVM_TypeSpecifier   **field_type;
} ExecutableClass;

typedef struct {
	char 	*name;
	int 	index;
} VTableItem;

struct DVM_VTable_tag {
	ExecutableClass 	*executable_class;
	int 				table_size;
	VTableItem 			*table;
};

struct ExecutableEntry_tag {
	DVM_Executable		*executable;
    int                 *class_table;
    int                 *function_table;
	Static 				static_v;
	struct ExecutableEntry_tag *next;
};

struct DVM_VirtualMachine_tag {
    Stack       		stack;
    Heap        		heap;
    ExecutableEntry     *current_executable;
	// 链接后的函数(所有)
	Function    		*current_function;
	DVM_ObjectRef   	current_exception;
	int         		pc;
	Function 			**function;
    int         		function_count;
	ExecutableClass 	**_class;
	int 				class_count;
    DVM_ExecutableList 	*executable_list;
	ExecutableEntry 	*executable_entry;
	ExecutableEntry 	*top_level;
	DVM_VTable			*array_v_table;
	DVM_VTable			*string_v_table;
};

// execute.c
void dvm_expand_stack(DVM_VirtualMachine *dvm, int need_stack_size);

DVM_Value dvm_execute_i(DVM_VirtualMachine *dvm, Function *function, DVM_Byte *code, int code_size, int base);

void dvm_push_object(DVM_VirtualMachine *dvm, DVM_Value value);

DVM_Value dvm_pop_object(DVM_VirtualMachine *dvm);

// heap.c
DVM_ObjectRef dvm_literal_to_dvm_string_i(DVM_VirtualMachine *dvm, DVM_Char *string);

DVM_ObjectRef dvm_create_dvm_string_i(DVM_VirtualMachine *dvm, DVM_Char *string);

DVM_ObjectRef dvm_create_array_int_i(DVM_VirtualMachine *dvm, int size);

DVM_ObjectRef dvm_create_array_double_i(DVM_VirtualMachine *dvm, int size);

DVM_ObjectRef dvm_create_array_object_i(DVM_VirtualMachine *dvm, int size);

DVM_ObjectRef dvm_create_class_object_i(DVM_VirtualMachine *dvm, int class_index);

void dvm_garbage_collect(DVM_VirtualMachine *dvm);

// native.c
void dvm_add_native_functions(DVM_VirtualMachine *dvm);

// load.c
int dvm_search_function(DVM_VirtualMachine *dvm, char *package_name, char *name);
void dvm_dynamic_load(DVM_VirtualMachine *dvm, DVM_Executable *callee_executable ,
                        Function *caller, int pc, Function *function);

// wchar.c

// util.c
void dvm_initialize_value(DVM_TypeSpecifier *type, DVM_Value *value);

void dvm_vstr_clear(VString *v);

void dvm_vstr_append_string(VString *v, DVM_Char *string);

void dvm_vstr_append_character(VString *v, DVM_Char ch);

// error.c
void dvm_error_i(DVM_Executable *executable, Function *function, int pc, RuntimeError id, ...);

void dvm_error_n(DVM_VirtualMachine *dvm, DVM_ErrorDefinition *error_definition, RuntimeError id, ...);

int dvm_conv_pc_to_line_number(DVM_Executable *executable, Function *function, int pc);



extern OpcodeInfo       dvm_opcode_info[];
extern DVM_ObjectRef    dvm_null_object_ref;
extern DVM_ErrorDefinition dvm_error_message_format[];

#endif //DIKSAM_DVM_PRI_H
