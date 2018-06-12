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

struct DVM_Object_tag {
	ObjectType type;
	unsigned int marked:1;
	union {
		DVM_String string;
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

struct DVM_VirtualMachine_tag {
    Stack       stack;
    Heap        heap;
    Static      static_v;
    int         pc;
    Function    *function;
    int         function_count;
    DVM_Executable *executable;
};

#endif //DIKSAM_DVM_PRI_H
