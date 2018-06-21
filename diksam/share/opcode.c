//
// Created by sulvto on 18-6-18.
//

#include "share.h"

DVM_ObjectRef dvm_null_object_ref = {
		NULL,
		NULL
};

OpcodeInfo dvm_opcode_info[] = {
		{"dummy", "", 0},
		{"push_int_1byte", "b", 1}
};