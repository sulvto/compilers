//
// Created by sulvto on 18-6-11.
//

#ifndef DIKSAM_DVM_H
#define DIKSAM_DVM_H

typedef struct DVM_Executable_tag DVM_Executable;
typedef struct DVM_ExecutableList_tag DVM_ExecutableList;
typedef struct DVM_VirtualMachine_tag DVM_VirtualMachine;

typedef struct DVM_Object_tag DVM_Object;
typedef struct DVM_String_tag DVM_String;

typedef struct DVM_VTable_tag DVM_VTable;

typedef enum {
	DVM_FALSE = 0,
	DVM_TRUE = 1
} DVM_Boolean;

typedef struct {
	DVM_VTable 	*v_table;
	DVM_Object 	*data;
} DVM_ObjectRef;

typedef union {
	int 			int_value;
	double 			double_value;
	DVM_ObjectRef 	*object;
} DVM_Value;

DVM_VirtualMachine *DVM_create_virtual_machine(void);

void DVM_add_executable(DVM_VirtualMachine *dvm, DVM_ExecutableList *executable_list);

DVM_Value DVM_execute(DVM_VirtualMachine *dvm);

void DVM_dispose_executable_list(DVM_ExecutableList *executable_list);

void DVM_dispose_virtual_machine(DVM_Value *dvm);

#endif //DIKSAM_DVM_H
