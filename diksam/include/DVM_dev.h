//
// Created by sulvto on 18-6-16.
//

#ifndef DIKSAM_DVM_DEV_H
#define DIKSAM_DVM_DEV_H

#include "DVM.h"
#include "DVM_code.h"

#define DVM_DIKSAM_DEFAULT_PACKAGE_P1   "diksam"
#define DVM_DIKSAM_DEFAULT_PACKAGE_P2   "lang"
#define DVM_DIKSAM_DEFAULT_PACKAGE \
(DVM_DIKSAM_DEFAULT_PACKAGE_P1 "." DVM_DIKSAM_DEFAULT_PACKAGE_P2)

typedef struct DVM_Array_tag DVM_Array;

typedef DVM_Value DVM_NativeFunctionProc(DVM_VirtualMachine *dvm, int arg_count, DVM_Value *args);


typedef enum {
	DVM_INT_MESSAGE_ARGUMENT,
	DVM_DOUBLE_MESSAGE_ARGUMENT,
	DVM_STRING_MESSAGE_ARGUMENT,
	DVM_POINTER_MESSAGE_ARGUMENT,
	DVM_CHARACTER_MESSAGE_ARGUMENT,
	DVM_MESSAGE_ARGUMENT_END
} DVM_MessageArgumentType;

typedef struct {
	char *format;
} DVM_ErrorDefinition;

typedef struct {
	DVM_ErrorDefinition *message_format;
} DVM_NativeLibInfo;

// execute.c
void DVM_add_native_function(DVM_VirtualMachine *dvm, char *package_name, char *function_name,
                                  DVM_NativeFunctionProc *proc, int arg_count, DVM_Boolean is_method,
                                  DVM_Boolean return_pointer);

// nativeif.c

int DVM_array_get_int(DVM_VirtualMachine *dvm, DVM_ObjectRef array, int index);

double DVM_array_get_double(DVM_VirtualMachine *dvm, DVM_ObjectRef array, int index);

DVM_ObjectRef DVM_array_get_object(DVM_VirtualMachine *dvm, DVM_ObjectRef array, int index);

void DVM_array_set_int(DVM_VirtualMachine *dvm, DVM_ObjectRef array, int index, int value);

void DVM_array_set_double(DVM_VirtualMachine *dvm, DVM_ObjectRef array, int index, double value);

void DVM_array_set_object(DVM_VirtualMachine *dvm, DVM_ObjectRef array, int index, DVM_ObjectRef value);

int DVM_array_size(DVM_VirtualMachine *dvm, DVM_Object *array);

void DVM_array_resize(DVM_VirtualMachine *dvm, DVM_Object *array, int new_size);

void DVM_array_insert(DVM_VirtualMachine *dvm, DVM_Object *array, int pos, DVM_Value value);

void DVM_array_remove(DVM_VirtualMachine *dvm, DVM_Object *array, int pos);

int DVM_string_length(DVM_VirtualMachine *dvm, DVM_Object *string);

DVM_Value DVM_string_substr(DVM_VirtualMachine *dvm, DVM_Object *string, int pos, int length);


// heap.c
DVM_ObjectRef DVM_create_dvm_string(DVM_VirtualMachine *dvm, DVM_Char *string);

DVM_ObjectRef DVM_create_dvm_int(DVM_VirtualMachine *dvm, int size);

DVM_ObjectRef DVM_create_dvm_double(DVM_VirtualMachine *dvm, int size);

DVM_ObjectRef DVM_create_dvm_object(DVM_VirtualMachine *dvm, int size);

// wchar.c

#endif //DIKSAM_DVM_DEV_H
