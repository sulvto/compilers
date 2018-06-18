//
// Created by sulvto on 18-6-15.
//
#include <stdio.h>
#include "DKC.h"
#include "MEM.h"
#include "DBG.h"
#include "dvm_pri.h"

DVM_VirtualMachine *DVM_create_virtual_machine(void) {
    DVM_VirtualMachine *dvm = MEM_malloc(sizeof(DVM_VirtualMachine));
    dvm->stack.alloc_size = STACK_ALLOC_SIZE;
    dvm->stack.stack = MEM_malloc(sizeof(DVM_Value) * STACK_ALLOC_SIZE);
    dvm->stack.pointer_flags = MEM_malloc(sizeof(DVM_Boolean) * STACK_ALLOC_SIZE);
    dvm->stack.stack_pointer = 0;
    dvm->heap.current_heap_size = 0;
    dvm->heap.header = NULL;
    dvm->heap.current_threshold = HEAP_THRESHOLD_SIZE;
    dvm->function = NULL;
    dvm->function_count = 0;
    dvm->current_executable = NULL;
	dvm->current_function = NULL;
	dvm->current_exception = dvm_null_object_ref;
	dvm->function = NULL;
	dvm->function_count = 0;
	dvm->_class = NULL;
	dvm->class_count = 0;
	dvm->executable_list = NULL;
	dvm->executable_entry = NULL;
	dvm->top_level = NULL;

	dvm_add_native_functions(dvm);

	set_built_in_method(dvm);

    return dvm;
}

void DVM_add_native_function(DVM_VirtualMachine *dvm, char *package_name, char *function_name,
                                  DVM_NativeFunctionProc *proc, int arg_count, DVM_Boolean is_method,
                                  DVM_Boolean return_pointer) {
    dvm->function = MEM_realloc(dvm->function, sizeof(Function) * (dvm->function_count + 1));
	dvm->function[dvm->function_count] = MEM_malloc(sizeof(Function));
	dvm->function[dvm->function_count]->package_name = MEM_strdup(package_name);
	dvm->function[dvm->function_count]->name = MEM_strdup(function_name);
	dvm->function[dvm->function_count]->is_implemented = DVM_TRUE;
    dvm->function[dvm->function_count]->kind = NATIVE_FUNCTION;
	dvm->function[dvm->function_count]->u.native_function.proc = proc;
	dvm->function[dvm->function_count]->u.native_function.argument_count= arg_count;
	dvm->function[dvm->function_count]->u.native_function.is_method = is_method;
	dvm->function[dvm->function_count]->u.native_function.return_pointer = return_pointer;

    dvm->function_count++;
}

static int search_function(DVM_VirtualMachine *dvm, char *package_name, char *name) {
    for (int i = 0; i < dvm->function_count; i++) {
        // TODO package_name
        if (strcmp(dvm->function[i]->name, name) == 0) {
            return i;
        }
    }

    return FUNCTION_NOT_FOUND;
}

static void convert_code(DVM_VirtualMachine *dvm,
                         DVM_Executable *executable,
                         DVM_Byte *code, int code_size,
                         DVM_Function *function) {
    for (int i = 0; i < code_size; i++) {
        if (code[i] == DVM_PUSH_STACK_INT
            ||code[i] == DVM_PUSH_STACK_DOUBLE
            ||code[i] == DVM_PUSH_STACK_OBJECT
            ||code[i] == DVM_POP_STACK_INT
            ||code[i] == DVM_POP_STACK_DOUBLE
            ||code[i] == DVM_POP_STACK_OBJECT) {
            DBG_assert(function != NULL, ("function == NULL!\n"));
            int src_index = GET_2BYTE_INT(&code[i + 1]);
            unsigned int dest_index;
            if (src_index >= function->parameter_count) {
                dest_index = src_index + CALL_INFO_ALIGN_SIZE;
            } else {
                dest_index = src_index;
            }

            SET_2BYTE_INT(&code[i + 1], dest_index);
        } else if (code[i] == DVM_PUSH_FUNCTION){
            int inx_in_exe = GET_2BYTE_INT(&code[i + 1]);
	        unsigned int func_inx = search_function(dvm, executable->function[inx_in_exe].package_name,
	                                                executable->function[inx_in_exe].name);
            SET_2BYTE_INT(&code[i + 1], func_inx);
        }

        OpcodeInfo *info = &dvm_opcode_info[code[i]];
        for (int j = 0; info->parameter[j] != '\0'; j++) {
            switch (info->parameter[j]) {
                case 'b':
                    i++;
                    break;
                case 's':
                case 'p':
                    i += 2;
                    break;
                default :
                    DBG_assert(0, ("parameter..%s, j..%d", info->parameter, j));
            }
        }
    }
}

static void implement_diksam_function(DVM_VirtualMachine *dvm, int dest_index, ExecutableEntry *executable_entry, int src_index) {
    dvm->function[dest_index]->u.diksam_function.executable = executable_entry;
    dvm->function[dest_index]->u.diksam_function.index = src_index;
}


static void add_functions(DVM_VirtualMachine *dvm, ExecutableEntry *executable_entry) {
    int src_index;
    int dest_index;
    int add_func_count = 0;

    DVM_Boolean *new_func_flags = MEM_malloc(sizeof(DVM_Boolean) * executable_entry->executable->function_count);

    for (src_index = 0; src_index < executable_entry->executable->function_count; src_index++) {
        for (dest_index = 0; dest_index < dvm->function_count; dest_index++) {
            if (0 == strcmp(executable_entry->executable->function[src_index].name, dvm->function[dest_index]->name)
                && dvm_compare_package_name(
                    (executable_entry->executable->function[src_index].package_name, dvm->function[dest_index]->package_name))) {
                if (executable_entry->executable->function[src_index].is_implemented
                        &&dvm->function[dest_index]->is_implemented)  {
                    char *package_name;
                    if (dvm->function[dest_index]->package_name) {
                        package_name = dvm->function[dest_index]->package_name;
                    } else {
                        package_name = "";
                    }

                        dvm_error_i(NULL, NULL, NO_LINE_NUMBER_PC,
                                  FUNCTION_MULTIPLE_DEFINE_ERR,
                                  DVM_STRING_MESSAGE_ARGUMENT, "package", package_name,
                                    DVM_STRING_MESSAGE_ARGUMENT, "name", dvm->function[dest_index]->name,
                                  DVM_MESSAGE_ARGUMENT_END);
                }
                new_func_flags[src_index] = DVM_FALSE;
                if (executable_entry->executable->function[src_index].is_implemented) {
                    implement_diksam_function(dvm, dest_index, executable_entry, src_index);
                }

                break;
            }
        }
        if (dest_index == dvm->function_count) {
            new_func_flags[src_index] = DVM_TRUE;
            add_func_count++;
        }
    }


    dvm->function = MEM_realloc(dvm->function, sizeof(Function) * (dvm->function_count + add_func_count));

    for (src_index = 0, dest_index = dvm->function_count;
         src_index < executable_entry->executable->function_count;
         src_index++) {
        if (!new_func_flags[src_index]) {
            continue;
        }
        dvm->function[dest_index] = MEM_malloc(sizeof(Function));
        if (executable_entry->executable->function[src_index].package_name) {
            dvm->function[dest_index]->package_name = MEM_strdup(
                    executable_entry->executable->function[src_index].package_name);
        } else {
            dvm->function[dest_index]->package_name = NULL;
        }
        dvm->function[dest_index]->name = MEM_strdup(executable_entry->executable->function[src_index].name);
        dvm->function[dest_index]->kind = DIKSAM_FUNCTION;
        dvm->function[dest_index]->is_implemented = executable_entry->executable->function[src_index].is_implemented;
        if (dvm->function[dest_index]->is_implemented ) {
            implement_diksam_function(dvm, dest_index, executable_entry, src_index);
        }

        dest_index++;
    }
    dvm->function_count += add_func_count;
    MEM_free(new_func_flags);
}


static void add_static_variables(ExecutableEntry *executable_entry, DVM_Executable *executable) {
    executable_entry->static_v.variable_count = executable->global_variable_count;
	executable_entry->static_v.variable = MEM_malloc(sizeof(DVM_Value) * executable->global_variable_count);

    for (int i = 0; i < executable->global_variable_count; i++) {
        if (executable->global_variable[i].type->basic_type == DVM_STRING_TYPE) {
            executable_entry->static_v.variable[i].object = dvm_null_object_ref;
        }
    }

    for (int i = 0; i < executable->global_variable_count; i++) {
        dvm_initialize_value(executable->global_variable[i].type,
                         &executable_entry->static_v.variable[i]);
    }
}

static DVM_Class *search_class_from_executable(DVM_Executable *executable, char *name) {
    for (int i = 0; i < executable->class_count; i++) {
        if (0 == strcmp(executable->class_definition[i].name, name)) {
            return &executable->class_definition[i];
        }
    }

    DBG_panic(("class %s not found.", name));

    return NULL;
}


static int set_field_types(DVM_Executable *executable, DVM_Class *pos, DVM_TypeSpecifier **field_type, int index) {

    DVM_Class *next;
    if (pos->super_class) {
        next = search_class_from_executable(executable, pos->super_class->name);
        index = set_field_types(executable, next, field_type, index);
    }

    for (int i = 0; i < pos->field_count; i++) {
        field_type[index] = pos->field[i].type;
        index++;
    }

    return index;
}

static void add_fields(DVM_Executable *executable, DVM_Class *src, ExecutableClass *dest) {

    int field_count = 0;
    for (DVM_Class *pos = src;;) {
        field_count += pos->field_count;
        if (pos->super_class == NULL) break;

        pos = search_class_from_executable(executable, pos->super_class->name);
    }

    dest->field_count = field_count;

    dest->field_type = MEM_malloc(sizeof(DVM_TypeSpecifier*) * field_count);
    set_field_types(executable, src, dest->field_type, 0);
}

static DVM_VTable *alloc_v_table(ExecutableClass *executable_class) {
    DVM_VTable *v_table = MEM_malloc(sizeof(DVM_VTable));
    v_table->executable_class = executable_class;
    v_table->table = NULL;

    return v_table;
}

static void set_v_table(DVM_VirtualMachine *dvm, DVM_Class *class_p, DVM_Method *src, VTableItem *dest,DVM_Boolean set_name) {
	if (set_name) {
		dest->name = MEM_strdup(src->name);
	}

	char *function_name = dvm_create_method_function_mathod(class_p->name, src->name);
	int function_index = search_function(dvm, class_p->package_name, function_name);
	if (function_index == FUNCTION_NOT_FOUND && !src->is_abstract) {
		dvm_class_i(NULL, NULL, NO_LINE_NUMBER_PC, FUNCTION_NOT_FOUND_ERR,
		            DVM_STRING_MESSAGE_ARGUMENT, "name", function_name,
		            DVM_MESSAGE_ARGUMENT_END);
	}
	MEM_free(function_name);
	dest->index = function_index;
}

static int add_method(DVM_VirtualMachine *dvm, DVM_Executable *executable, DVM_Class *pos, DVM_VTable *v_table) {
    DVM_Class *next;
    int super_method_count = 0;
    int method_count = 0;
    if (pos->super_class) {
        next = search_class_from_executable(executable, pos->super_class->name);
        super_method_count = add_method(dvm, executable, next, v_table);
    }

    method_count = super_method_count;
    for (int i = 0; i < pos->method_count; i++) {

        int j;
        for (j = 0; j < super_method_count; j++) {
            if (0 == strcmp(pos->method[i].name, v_table->table[j].name)) {
                set_v_table(dvm, pos, &pos->method[i], &v_table->table[j], DVM_FALSE);
                break;
            }
        }

        if (j == super_method_count && !pos->method[i].is_override) {
            v_table->table = MEM_realloc(v_table->table, sizeof(VTableItem) * (method_count + 1));
            set_v_table(dvm, pos, &pos->method[i], &v_table->table[method_count], DVM_TRUE);
            method_count++;
            v_table->table_size = method_count;
        }
    }

    return method_count;
}

static void add_methods(DVM_VirtualMachine *dvm, DVM_Executable *executable, DVM_Class *src, ExecutableClass *dest) {
	DVM_VTable *v_table = alloc_v_table(dest);
	int method_count = add_method(dvm, executable, src, v_table);
	dest->class_table = v_table;
	dest->interface_count = src->interface_count;
	dest->interface_v_table = MEM_malloc(sizeof(DVM_VTable *) * src->interface_count);

    for (int i = 0; i < src->interface_count; i++) {
        dest->interface_v_table[i] = alloc_v_table(dest);
        DVM_Class *interface = search_class_from_executable(executable, src->interface[i].name);
        dest->interface_v_table[i]->table = MEM_malloc(sizeof(VTableItem) * interface->method_count);
        dest->interface_v_table[i]->table_size = interface->method_count;
        for (int method_index = 0; method_index < interface->method_count; method_index++) {
            set_v_table(dvm, src, &interface->method[method_index], &dest->interface_v_table[i]->table[method_index],
                        DVM_TRUE);
        }
    }
}

static void add_class(DVM_VirtualMachine *dvm, DVM_Executable *executable, DVM_Class *src, ExecutableClass *dest) {
	add_fields(executable, src, dest);
	add_methods(dvm, executable, src, dest);
}

static int search_class(DVM_VirtualMachine *dvm, char *package_name, char *name) {
	for (int i = 0; i < dvm->class_count; i++) {
	    if (dvm_compare_package_name(dvm->_class[i]->package_name, package_name)
	        && 0 == strcmp(dvm->_class[i]->name, name)) {
	        return i;
	    }
	}

    dvm_error_i(NULL, NULL, NO_LINE_NUMBER_PC, CLASS_NOT_FOUND_ERR,
                DVM_STRING_MESSAGE_ARGUMENT, "name", name,
                DVM_MESSAGE_ARGUMENT_END);

    return 0;
}

static void set_super_class(DVM_VirtualMachine *dvm, DVM_Executable *executable, int old_class_count) {

	int super_class_index;
	DVM_Class *dvm_class;
	for (int class_index = old_class_count; class_index < dvm->class_count; class_index++) {
	    dvm_class = search_class_from_executable(executable, dvm->_class[class_index]->name);

	    if (dvm_class->super_class == NULL) {
	        dvm->_class[class_index]->super_class = NULL;
	    } else {
	        super_class_index = search_class(dvm,
	                                         dvm_class->super_class->package_name,
	                                         dvm_class->super_class->name);
	        dvm->_class[class_index]->super_class = dvm->_class[super_class_index];
	    }
	    dvm->_class[class_index]->interface = MEM_malloc(sizeof(ExecutableClass *) * dvm_class->interface_count);
	    for (int if_index = 0; if_index < dvm_class->interface_count; if_index++) {
	        int interface_index = search_class(dvm, dvm_class->interface[if_index].package_name,
	                                           dvm_class->interface[if_index].name);
	        dvm->_class[class_index]->interface[if_index] = dvm->_class[interface_index];
	    }
	}
}

static void add_classes(DVM_VirtualMachine *dvm, ExecutableEntry *executable_entry) {
	int src_index;
	int dest_index;
	int add_class_count = 0;
	int old_class_count;

	DVM_Executable *executable = executable_entry->executable;

	DVM_Boolean *new_class_flags = MEM_malloc(sizeof(DVM_Boolean) * executable->class_count);


	for (src_index = 0; src_index < executable->class_count; src_index++) {
		for (dest_index = 0; dest_index < dvm->class_count; dest_index++) {
		    if (0 == strcmp(dvm->_class[dest_index]->name, executable->class_definition[src_index].name)
		        && dvm_compare_package_name(dvm->_class[dest_index]->package_name,
		                                    executable->class_definition[src_index].package_name)) {
		        if (executable->class_definition[src_index].is_implemented
		                &&dvm->_class[dest_index]->is_implemented) {
		            char *package_name;
		            if (dvm->_class[dest_index]->package_name) {
		                package_name = dvm->_class[dest_index]->package_name;
		            } else {
		                package_name = "";
		            }

		            dvm_error_i(NULL, NULL, NO_LINE_NUMBER_PC,
		                        CLASS_MULTIPLE_DEFINE_ERR,
		                        DVM_STRING_MESSAGE_ARGUMENT, "package", package_name,
		                        DVM_STRING_MESSAGE_ARGUMENT, "name", dvm->_class[dest_index]->name,
		                        DVM_MESSAGE_ARGUMENT_END);
		        }

		        new_class_flags[src_index] = DVM_FALSE;

		        if (executable->class_definition[src_index].is_implemented) {
		            add_class(dvm, executable, &executable->class_definition[src_index], dvm->_class[dest_index]);
		        }
		        break;
		    }
		}
		if (dest_index == dvm->class_count) {
		    new_class_flags[src_index] = DVM_TRUE;
		    add_class_count++;
		}
	}
	dvm->_class = MEM_realloc(dvm->_class, sizeof(ExecutableClass *) * (dvm->class_count + add_class_count));

	for (src_index = 0, dest_index = dvm->class_count;
	     src_index < executable->class_count;
	        src_index++) {
	    if (!new_class_flags[src_index]) {
	        continue;
	    }

		dvm->_class[dest_index] = MEM_malloc(sizeof(ExecutableClass));
		dvm->_class[dest_index]->dvm_class = &executable->class_definition[src_index];
		dvm->_class[dest_index]->executable = executable_entry;
		if (executable->class_definition[src_index].package_name) {
		    dvm->_class[dest_index]->package_name =
		            MEM_strdup(executable->class_definition[src_index].package_name);
		} else {
		    dvm->_class[dest_index]->package_name = NULL;
		}
		dvm->_class[dest_index]->name = MEM_strdup(executable->class_definition[src_index].name);
		dvm->_class[dest_index]->is_implemented = executable->class_definition[src_index].is_implemented;
		dvm->_class[dest_index]->class_index = dest_index;
		if (dvm->_class[dest_index]->is_implemented) {
		    add_class(dvm, executable, &executable->class_definition[src_index], dvm->_class[dest_index]);
		}
		dest_index++;
	}

	old_class_count = dvm->class_count;
	dvm->class_count += add_class_count;

	set_super_class(dvm, executable, old_class_count);

	MEM_free(new_class_flags);
}

static ExecutableEntry *add_executable_to_dvm(DVM_VirtualMachine *dvm, DVM_Executable *executable, DVM_Boolean is_top_level) {

    ExecutableEntry *entry_pos;
    ExecutableEntry *new_entry = MEM_malloc(sizeof(ExecutableEntry));
    new_entry->executable = executable;
    new_entry->next = NULL;

    if (dvm->executable_entry == NULL) {
        dvm->executable_entry = new_entry;
    } else {
        for (entry_pos = dvm->executable_entry; entry_pos->next; entry_pos = entry_pos->next);
        entry_pos->next = new_entry;
    }

    add_functions(dvm, new_entry);
    add_classes(dvm, new_entry);

    convert_code(dvm, executable, executable->code,
                 executable->code_size, NULL);

    for (int i = 0; i < executable->function_count; i++) {
        if (executable->function[i].is_implemented) {
            convert_code(dvm, executable, executable->function[i].code,
                         executable->function[i].code_size,
                         &executable->function[i]);
        }
    }


    add_static_variables(new_entry, executable);

    if (is_top_level) {
        dvm->top_level = new_entry;
    }

    return new_entry;
}

void DVM_set_executable(DVM_VirtualMachine *dvm, DVM_ExecutableList *executable_list) {
    dvm->executable_list = executable_list;

    int old_class_count = dvm->class_count;
    for (DVM_ExecutableItem *pos = executable_list->list; pos; pos = pos->next) {
        add_executable_to_dvm(dvm, pos->executable, (pos->executable == executable_list->top_level));
    }
}
