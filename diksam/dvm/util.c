//
// Created by sulvto on 18-6-15.
//
#include "dvm_pri.h"

void dvm_initialize_value(DVM_TypeSpecifier *type, DVM_Value *value) {
	if (type->derive_count > 0) {
		// array
		if (type->derive[0].tag == DVM_ARRAY_DERIVE) {
			value->object = dvm_null_object_ref;
		} else {
			DBG_assert(0, ("tag..%d", type->derive[0].tag));
		}
	} else {
	switch (type->basic_type) {
		case DVM_VOID_TYPE:
		case DVM_BOOLEAN_TYPE:
		case DVM_INT_TYPE:
	        value->int_value = 0;
	        break;
	    case DVM_DOUBLE_TYPE:
	        value->double_value = 0.0;
	        break;
	    case DVM_STRING_TYPE:
	    case DVM_CLASS_TYPE:
	        value->object = dvm_null_object_ref;
	        break;
		case DVM_NULL_TYPE:
		case DVM_BASE_TYPE:
	    default:
	        DBG_assert(0, ("basic_type..%d", type->basic_type));
	}
	}
}