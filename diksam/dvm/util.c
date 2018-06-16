//
// Created by sulvto on 18-6-15.
//
#include "DVM_code.h"

static void dvm_initialize_value(DVM_TypeSpecifier *type, DVM_Value *value) {
    switch (type) {
        case DVM_BOOLEAN_TYPE:
        case DVM_INT_TYPE:
            value->int_value = 0;
            break;
        case DVM_DOUBLE_TYPE:
            value->double_value = 0.0;
            break;
        case DVM_STRING_TYPE:
            value->object = dvm_literal_to_dvm_string_i(dvm, L"");
            break;
        default:
            DBG_assert(0, ("basic_type..%d", type));
    }
}