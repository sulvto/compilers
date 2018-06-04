//
// Created by sulvto on 18-6-2.
//

#ifndef CROWBAR_CRB_DEV_H
#define CROWBAR_CRB_DEV_H

typedef enum {
    CRB_FALSE = 0,
    CRB_TRUE = 1
} CRB_Boolean;

typedef enum {
    CRB_BOOLEAN_VALUE = 1,
    CRB_INT_VALUE,
    CRB_DOUBLE_VALUE,
    CRB_STRING_VALUE,
    CRB_NATIVE_POINTER_VALUE,
    CRB_ARRAY_TYPE,
    CRB_NULL_VALUE
} CRB_ValueType;

typedef struct {
    char *name;
} CRB_NativePointerInfo;

typedef struct {
    CRB_NativePointerInfo   *info;
    void                    *pointer;
} CRB_NativePointer;

typedef struct {
    CRB_ValueType type;
    union {
        CRB_Boolean boolean_value;
        int int_value;
        double double_value;
        CRB_NativePointer native_pointer;
        CRB_Object *object;
    } u;
} CRB_Value;

#endif //CROWBAR_CRB_DEV_H
