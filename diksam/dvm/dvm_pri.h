//
// Created by sulvto on 18-6-11.
//

#ifndef DIKSAM_DVM_PRI_H
#define DIKSAM_DVM_PRI_H

typedef enum {
	NATIVE_FUNCTION,
	DIKSAM_FUNCTION
} FunctionKind;

typedef struct {
	DVM_NativeFunctionProc *proc;
	int 		argument_count;
} NativeFunction;

typedef struct {
	DVM_Executable 	*executable;
	int 			index;
} DiksamFunction;

typedef struct {
	char *name;
	FunctionKind kind;
	union {
		NativeFunction native_function;
		DiksamFunction diksam_function;
	} u;
} Function;

typedef enum {
	STRING_OBJECT = 1,
	OBJECT_TYPE_COUNT_PLUS_1
} ObjectType;

struct DVM_String_tag {
	DVM_Boolean is_literal;
	DVM_Char *string;
};

typedef enum {
	INT_ARRAY = 1,
	DOUBLE_ARRAY,
	OBJECT_ARRAY
} ArrayType;

typedef struct DVM_Array_tag {
	ArrayType 	type;
	int			size;
	int 		alloc_size;
	union {
		int 	*int_array;
		double 	*double_array;
		DVM_Object **object;
	} u;
};

struct DVM_Object_tag {
	ObjectType type;
	unsigned int marked:1;
	union {
		DVM_String 	string;
		DVM_Array 	array;
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

// heap.c
DVM_Object *dvm_literal_to_dvm_string_i(DVM_VirtualMachine *dvm, DVM_Char *string);

DVM_Object *dvm_create_dvm_string_i(DVM_VirtualMachine *dvm, DVM_Char *string);

void dvm_garbage_collect(DVM_VirtualMachine *dvm);

// native.c
void dvm_add_native_functions(DVM_VirtualMachine *dvm);

// wchar.c

// util.c
void dvm_initialize_value(DVM_TypeSpecifier *type, DVM_Value *value);

// error.c



#endif //DIKSAM_DVM_PRI_H
