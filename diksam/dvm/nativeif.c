//
// Created by sulvto on 18-6-16.
//
#include "MEM.h"
#include "DBG.h"
#include "dvm_pri.h"

static void check_array(DVM_ObjectRef array, int index,
                        DVM_Executable *executable,
                        Function *function,
                        int pc) {
	if (array.data == NULL) {
		dvm_error_i(executable, function, pc, NULL_POINTER_ERR, DVM_MESSAGE_ARGUMENT_END);
	}

	if (index < 0 || index >= array.data->u.array.size) {
		dvm_error_i(executable, function, pc, INDEX_OUT_OF_BOUNDS_ERR,
		            DVM_INT_MESSAGE_ARGUMENT, "index", index,
		            DVM_INT_MESSAGE_ARGUMENT, "size", array.data->u.array.size,
		            DVM_MESSAGE_ARGUMENT_END);
	}
}

int DVM_array_get_int(DVM_VirtualMachine *dvm, DVM_ObjectRef array, int index) {
    check_array(array, index,
                dvm->current_executable->executable,
                dvm->current_function,
                dvm->pc);
    return array.data->u.array.u.int_array[index];
}

double DVM_array_get_double(DVM_VirtualMachine *dvm, DVM_ObjectRef array, int index) {
    check_array(array, index,
                dvm->current_executable->executable,
                dvm->current_function,
                dvm->pc);
    return array.data->u.array.u.double_array[index];
}

DVM_ObjectRef DVM_array_get_object(DVM_VirtualMachine *dvm, DVM_ObjectRef array, int index) {
    check_array(array, index,
                dvm->current_executable->executable,
                dvm->current_function,
                dvm->pc);
    return array.data->u.array.u.object[index];
}


void DVM_array_set_int(DVM_VirtualMachine *dvm, DVM_ObjectRef array, int index, int value) {
    check_array(array, index,
                dvm->current_executable->executable,
                dvm->current_function,
                dvm->pc);
    array.data->u.array.u.int_array[index] = value;
}

void DVM_array_set_double(DVM_VirtualMachine *dvm, DVM_ObjectRef array, int index, double value) {
    check_array(array, index,
                dvm->current_executable->executable,
                dvm->current_function,
                dvm->pc);
    array.data->u.array.u.double_array[index] = value;
}

void DVM_array_set_object(DVM_VirtualMachine *dvm, DVM_ObjectRef array, int index, DVM_ObjectRef value) {
	check_array(array, index,
                dvm->current_executable->executable,
                dvm->current_function,
                dvm->pc);
	array.data->u.array.u.object[index] = value;
}

int DVM_array_size(DVM_VirtualMachine *dvm, DVM_Object *array) {
    return array->u.array.size;
}

static void resize_array(DVM_VirtualMachine *dvm, DVM_Object *array, int new_size) {

}


void DVM_array_resize(DVM_VirtualMachine *dvm, DVM_Object *array, int new_size) {
	resize_array(dvm, array, new_size);
	switch (array->u.array.type) {
		case INT_ARRAY:
			for (int i = array->u.array.size; i < new_size; i++) {
				array->u.array.u.int_array[i] = 0;
			}
			break;
		case DOUBLE_ARRAY:
			for (int i = array->u.array.size; i < new_size; i++) {
				array->u.array.u.double_array[i] = 0.0;
			}
			break;
		case OBJECT_ARRAY:
			for (int i = array->u.array.size; i < new_size; i++) {
				array->u.array.u.object[i] = dvm_null_object_ref;
			}
			break;
		case FUNCTION_INDEX_ARRAY:
		default:
			DBG_panic(("array->u.array.type..%d", array->u.array.type));
	}
}

void DVM_array_insert(DVM_VirtualMachine *dvm, DVM_Object *array, int pos, DVM_Value value) {
	resize_array(dvm, array, array->u.array.size + 1);
	switch (array->u.array.type) {
		case INT_ARRAY:
			memmove(&array->u.array.u.int_array[pos + 1], &array->u.array.u.int_array[pos],
			        sizeof(int) * (array->u.array.size - pos));
			array->u.array.u.int_array[pos] = value.int_value;
			break;
		case DOUBLE_ARRAY:
			memmove(&array->u.array.u.double_array[pos + 1], &array->u.array.u.double_array[pos],
			        sizeof(int) * (array->u.array.size - pos));
			array->u.array.u.double_array[pos] = value.double_value;
			break;
		case OBJECT_ARRAY:
			memmove(&array->u.array.u.object[pos + 1], &array->u.array.u.object[pos],
			        sizeof(int) * (array->u.array.size - pos));
			array->u.array.u.object[pos] = value.object;
			break;
		case FUNCTION_INDEX_ARRAY:
		default:
			DBG_panic(("array->u.array.type..%d", array->u.array.type));
	}

	array->u.array.size++;
}

void DVM_array_remove(DVM_VirtualMachine *dvm, DVM_Object *array, int pos) {
	// TODO
	printf("TODO\n");
}

int DVM_string_length(DVM_VirtualMachine *dvm, DVM_Object *string) {
	// TODO
	printf("TODO\n");
	return 0;
}

DVM_Value DVM_string_substr(DVM_VirtualMachine *dvm, DVM_Object *string, int pos, int length) {
	DVM_Value value;
	// TODO
	printf("TODO\n");
	return value;
}

static int get_field_index_sub(ExecutableClass *executable_class, char *field_name, int *super_count) {
    int index;
    if (executable_class->super_class) {
        index = get_field_index_sub(executable_class->super_class, field_name, super_count);
        if (index != FIELD_NOT_FOUND) {
            return index;
        }
    }

    for (int i = 0; i < executable_class->dvm_class->field_count; i++) {
        if (!strcmp(executable_class->dvm_class->field[i].name, field_name)) {
            return i + *super_count;
        }
    }
    *super_count += executable_class->dvm_class->field_count;

    return FIELD_NOT_FOUND;
}

int DVM_get_field_index(DVM_VirtualMachine *dvm, DVM_ObjectRef object, char *field_name) {
    int super_count = 0;

    return get_field_index_sub(object.v_table->executable_class, field_name, &super_count);
}

