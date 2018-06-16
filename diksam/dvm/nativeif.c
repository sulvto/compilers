//
// Created by sulvto on 18-6-16.
//


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

void DVM_array_size(DVM_VirtualMachine *dvm, DVM_Object *array) {
    return array->u.array.size;
}