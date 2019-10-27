//
// Created by sulvto on 18-6-11.
//
#include <math.h>
#include <string.h>
#include <DVM_code.h>
#include "MEM.h"
#include "DBG.h"
#include "dvm_pri.h"

extern OpcodeInfo dvm_opcode_info[];

DVM_Value dvm_execute_i(DVM_VirtualMachine *dvm, Function *function,
                         DVM_Byte *code, int code_size, int base);

void dvm_expand_stack(DVM_VirtualMachine *dvm, int need_stack_size) {
    int rest = dvm->stack.alloc_size - dvm->stack.stack_pointer;
    if (rest <= need_stack_size) {
        int revalue_up = ((rest / STACK_ALLOC_SIZE) + 1) * STACK_ALLOC_SIZE;
        dvm->stack.alloc_size += revalue_up;
        dvm->stack.stack = MEM_realloc(dvm->stack.stack, dvm->stack.alloc_size * sizeof(DVM_Value));
        dvm->stack.pointer_flags =
                MEM_realloc(dvm->stack.pointer_flags,
                            dvm->stack.alloc_size * sizeof(DVM_Boolean));
    }
}

static void invoke_native_function(DVM_VirtualMachine *dvm, Function *caller, Function *callee, int pc, int *sp_p,
                                   int base) {

	(*sp_p)--;
	DVM_Value *stack = dvm->stack.stack;
	DBG_assert(callee->kind == NATIVE_FUNCTION, ("callee->kind..%d", callee->kind));

	int argument_count;
	if (callee->u.native_function.is_method) {
		argument_count = callee->u.native_function.argument_count + 1;
	} else {
		argument_count = callee->u.native_function.argument_count;
	}
	CallInfo *call_info = (CallInfo *) &dvm->stack.stack[*sp_p];
	call_info->caller = caller;
	call_info->caller_address = pc;
	call_info->base = base;
	for (int i = 0; i < CALL_INFO_ALIGN_SIZE; i++) {
		dvm->stack.pointer_flags[*sp_p + i] = DVM_FALSE;
	}
	*sp_p += CALL_INFO_ALIGN_SIZE;
	dvm->current_function = callee;

	DVM_Value ret = callee->u.native_function.proc(dvm, callee->u.native_function.argument_count,
	                                               &stack[*sp_p - argument_count - CALL_INFO_ALIGN_SIZE]);
	dvm->current_function = caller;


	*sp_p -= argument_count + CALL_INFO_ALIGN_SIZE;
	stack[*sp_p] = ret;
	dvm->stack.pointer_flags[*sp_p] = callee->u.native_function.return_pointer;

	(*sp_p)++;

}

static void initialize_local_variables(DVM_VirtualMachine *dvm,
                                       DVM_Function *function, int from_sp) {
    for (int i = 0, sp_index = from_sp;
         i < function->local_variable_count;
         i++, sp_index++) {
        dvm->stack.pointer_flags[i] = DVM_FALSE;
    }

    for (int i = 0, sp_index = from_sp;
         i < function->local_variable_count;
         i++, sp_index++) {
        dvm_initialize_value(function->local_variable[i].type,
                             &dvm->stack.stack[sp_index]);

        if (function->local_variable[i].type->basic_type == DVM_STRING_TYPE) {
            dvm->stack.pointer_flags[i] = DVM_FALSE;
        }
    }
}

static void invoke_diksam_function(DVM_VirtualMachine *dvm, Function **caller_p, Function *callee, DVM_Byte **code_p,
                                   int *code_size_p, int *pc_p, int *sp_p, int *base_p,
                                   ExecutableEntry **executable_entry_p, DVM_Executable **executable_p) {

	if (!callee->is_implemented) {
		dvm_dynamic_load(dvm, *executable_p, *caller_p, *pc_p, callee);
	}

	*executable_entry_p = callee->u.diksam_function.executable;
	*executable_p = (*executable_entry_p)->executable;
	DVM_Function *callee_p = &(*executable_p)->function[callee->u.diksam_function.index];
	dvm_expand_stack(dvm, CALL_INFO_ALIGN_SIZE
	                  + callee_p->local_variable_count
	                  + (*executable_p)->function[callee->u.diksam_function.index].code_block.need_stack_size);
	CallInfo *callinfo = (CallInfo *) &dvm->stack.stack[*sp_p - 1];
	callinfo->caller = *caller_p;
	callinfo->caller_address = *pc_p;
	callinfo->base = *base_p;
	for (int i = 0; i < CALL_INFO_ALIGN_SIZE; i++) {
		dvm->stack.pointer_flags[*sp_p - 1 + i] = DVM_FALSE;
	}

	*base_p = *sp_p - callee_p->parameter_count - 1;
	if (callee_p->is_method) {
		(*base_p)--;
	}
	*caller_p = callee;

	initialize_local_variables(dvm, callee_p, *sp_p + CALL_INFO_ALIGN_SIZE - 1);

	*sp_p += CALL_INFO_ALIGN_SIZE + callee_p->local_variable_count - 1;
	*pc_p = 0;

	*code_p = (*executable_p)->function[callee->u.diksam_function.index].code_block.code;
	*code_size_p = (*executable_p)->function[callee->u.diksam_function.index].code_block.code_size;
}

static DVM_Boolean do_return(DVM_VirtualMachine *dvm, Function **function_p,
                             DVM_Byte **code_p, int *code_size_p, int *pc_p, int *base_p,
                             ExecutableEntry **executable_entry_p, DVM_Executable **executable_p) {

    DVM_Function *callee_p = &(*executable_p)->function[(*function_p)->u.diksam_function.index];
    int arg_count = callee_p->parameter_count;
    if (callee_p->is_method) {
        arg_count++;
    }
    CallInfo *call_info = (CallInfo *)&dvm->stack.stack[*base_p + arg_count];

    DVM_Function *caller_p;

    if (call_info->caller) {
        *executable_entry_p = call_info->caller->u.diksam_function.executable;
        *executable_p = (*executable_entry_p)->executable;
        if (call_info->caller->kind == DIKSAM_FUNCTION) {
            caller_p = &(*executable_p)->function[call_info->caller->u.diksam_function.index];
            *code_p = caller_p->code_block.code;
            *code_size_p = caller_p->code_block.code_size;
        }
    } else {
        *executable_entry_p = dvm->top_level;
        *executable_p = dvm->top_level->executable;
        *code_p = dvm->top_level->executable->top_level.code;
        *code_size_p = dvm->top_level->executable->top_level.code_size;
    }
    *function_p = call_info->caller;
    dvm->stack.stack_pointer = *base_p;
    *pc_p = call_info->caller_address + 1;
    *base_p = call_info->base;

    return call_info->caller_address == CALL_FROM_NATIVE;
}

static DVM_Boolean return_function(DVM_VirtualMachine *dvm, Function **function_p,
                                   DVM_Byte **code_p, int *code_size_p, int *pc_p, int *base_p,
                                   ExecutableEntry **executable_entry_p, DVM_Executable **executable_p) {
    DVM_Value return_value = dvm->stack.stack[dvm->stack.stack_pointer - 1];
    dvm->stack.stack_pointer--;

    DVM_Function *callee_p = &(*executable_p)->function[(*function_p)->u.diksam_function.index];

    DVM_Boolean ret = do_return(dvm, function_p, code_p, code_size_p, pc_p, base_p, executable_entry_p, executable_p);

    dvm->stack.stack[dvm->stack.stack_pointer] = return_value;
    dvm->stack.pointer_flags[dvm->stack.stack_pointer] = is_pointer_type(callee_p->type);
    dvm->stack.stack_pointer++;

    return ret;
}

#define STI(dvm, sp) ((dvm)->stack.stack[(dvm)->stack.stack_pointer + (sp)].int_value)
#define STD(dvm, sp) ((dvm)->stack.stack[(dvm)->stack.stack_pointer + (sp)].double_value)
#define STO(dvm, sp) ((dvm)->stack.stack[(dvm)->stack.stack_pointer + (sp)].object)

#define STI_I(dvm, sp) ((dvm)->stack.stack[(sp)].int_value)
#define STD_I(dvm, sp) ((dvm)->stack.stack[(sp)].double_value)
#define STO_I(dvm, sp) ((dvm)->stack.stack[(sp)].object)

#define STI_WRITE(dvm, sp, r)    \
        ((dvm)->stack.stack[(dvm)->stack.stack_pointer + (sp)].int_value = r, \
         (dvm)->stack.pointer_flags[(dvm)->stack.stack_pointer + (sp)] = DVM_FALSE)
#define STD_WRITE(dvm, sp, r)    \
        ((dvm)->stack.stack[(dvm)->stack.stack_pointer + (sp)].double_value = r, \
         (dvm)->stack.pointer_flags[(dvm)->stack.stack_pointer + (sp)] = DVM_FALSE)
#define STO_WRITE(dvm, sp, r)    \
        ((dvm)->stack.stack[(dvm)->stack.stack_pointer + (sp)].object = r, \
         (dvm)->stack.pointer_flags[(dvm)->stack.stack_pointer + (sp)] = DVM_TRUE)

#define STI_WRITE_I(dvm, sp, r)    \
        ((dvm)->stack.stack[(sp)].int_value = r, (dvm)->stack.pointer_flags[(sp)] = DVM_FALSE)
#define STD_WRITE_I(dvm, sp, r)    \
        ((dvm)->stack.stack[(sp)].double_value = r, (dvm)->stack.pointer_flags[(sp)] = DVM_FALSE)
#define STO_WRITE_I(dvm, sp, r)    \
        ((dvm)->stack.stack[(sp)].object = r, (dvm)->stack.pointer_flags[(sp)] = DVM_TRUE)

#define is_null_pointer(object) (((object)->data == NULL))

static DVM_ObjectRef create_array_sub(DVM_VirtualMachine *dvm, int dim, int dim_index, DVM_TypeSpecifier *type) {
	DVM_ObjectRef ret;
	int size = STI(dvm, -dim);
	if (dim_index == type->derive_count - 1) {
		switch (type->basic_type) {
			case DVM_VOID_TYPE:
				DBG_panic(("creating void array"));
				break;
			case DVM_BOOLEAN_TYPE:
			case DVM_INT_TYPE:
				ret = dvm_create_array_int_i(dvm, size);
				break;
			case DVM_DOUBLE_TYPE:
				ret = dvm_create_array_double_i(dvm, size);
				break;
			case DVM_STRING_TYPE:
			case DVM_CLASS_TYPE:
				ret = dvm_create_array_object_i(dvm, size);
				break;
			case DVM_NULL_TYPE:
			case DVM_BASE_TYPE:
			default:
				DBG_assert(0, ("type->basic_type..%d\n", type->basic_type));
				break;
		}
	} else if (type->derive[dim_index].tag == DVM_FUNCTION_DERIVE){
		DBG_panic(("Function type in array literal.\n"));
	} else {
		ret = dvm_create_array_object_i(dim, size);
		if (dim_index < dim - 1) {
			STO_WRITE(dvm, 0, ret);
			dvm->stack.stack_pointer++;
			for (int i = 0; i < size; i++) {
				DVM_ObjectRef child = create_array_sub(dim, dim, dim_index + 1, type);
				DVM_array_set_object(dvm, ret, i, child);
			}
			dvm->stack.stack_pointer--;
		}
	}

	return ret;
}

static DVM_ObjectRef chain_string(DVM_VirtualMachine *dvm, DVM_ObjectRef string1, DVM_ObjectRef string2) {
	DVM_Char *result;
	int result_length;
	DVM_Char *left;
	int left_length;
	DVM_Char *right;
	int right_length;

	if (string1.data == NULL) {
		left = NULL_STRING;
		left_length = dvm_wcslen(NULL_STRING);
	} else {
		left  = string1.data->u.string.string;
		left_length  = string1.data->u.string.length;
	}
	if (string2.data == NULL) {
		right = NULL_STRING;
		right_length = dvm_wcslen(NULL_STRING);
	} else {
		right  = string2.data->u.string.string;
		right_length  = string2.data->u.string.length;
	}

	result_length = right_length + left_length;
	result = MEM_malloc(sizeof(DVM_Char) * (result_length + 1));
	dvm_wcscpy(result, left);
	dvm_wcscat(result, right);

	return dvm_create_dvm_string_i(dvm, result);
}

static void restore_pc(DVM_VirtualMachine *dvm, ExecutableEntry *executable_entry, Function *function, int pc) {
    dvm->current_executable = executable_entry;
    dvm->current_function = function;
    dvm->pc = pc;
}

static DVM_ObjectRef create_array(DVM_VirtualMachine *dvm, int dim, DVM_TypeSpecifier *type) {
	return create_array_sub(dvm, dim, 0, type);
}

static DVM_ObjectRef create_array_literal_int(DVM_VirtualMachine *dvm, int size) {
    DVM_ObjectRef array = dvm_create_array_int_i(dvm, size);
    for (int i = 0; i < size; i++) {
        array.data->u.array.u.int_array[i] = STI(dvm, -size + 1);
    }

    return array;
}

static DVM_ObjectRef create_array_literal_double(DVM_VirtualMachine *dvm, int size) {
    DVM_ObjectRef array = dvm_create_array_object_i(dvm, size);
    for (int i = 0; i < size; i++) {
        array.data->u.array.u.double_array[i] = STD(dvm, -size + 1);
    }

    return array;
}

static DVM_ObjectRef create_array_literal_object(DVM_VirtualMachine *dvm, int size) {
    DVM_ObjectRef array = dvm_create_array_object_i(dvm, size);
    for (int i = 0; i < size; i++) {
        array.data->u.array.u.object[i] = STO(dvm, -size + 1);
    }

    return array;
}

#define check_null_pointer(executable, function, pc, object) \
		{ if ((object)->data == NULL) check_null_pointer_func((executable), (function), (pc), (object)); }

static void check_null_pointer_func(DVM_Executable *executable, Function *function, int pc, DVM_ObjectRef *object){
	if (object->data == NULL) {
		dvm_error_i(executable, function, pc, NULL_POINTER_ERR, DVM_MESSAGE_ARGUMENT_END);
	}
}

static DVM_Boolean check_instanceof_i(DVM_VirtualMachine *dvm, DVM_ObjectRef *object, int target_index, DVM_Boolean *is_interface,
                                      int *interface_index) {
	for (ExecutableClass *pos = object->v_table->executable_class->super_class; pos; pos = pos->super_class) {
		if (pos->class_index == target_index) {
			*is_interface = DVM_FALSE;
			return DVM_TRUE;
		}
	}

	for (int i = 0; i < object->v_table->executable_class->interface_count; i++) {
		if (object->v_table->executable_class->interface[i]->class_index == target_index) {
			*is_interface = DVM_FALSE;
			*interface_index = i;
			return DVM_TRUE;
		}
	}
	return DVM_FALSE;
}

static DVM_Boolean check_instanceof(DVM_VirtualMachine *dvm, DVM_ObjectRef *object, int target_index) {
	DVM_Boolean is_interface_dummy;
	int interface_index_dummy;
	return check_instanceof_i(dvm, object, target_index, &is_interface_dummy, &interface_index_dummy);
}

static void reset_stack_pointer(DVM_Function *dvm_function, int *sp_p, int base) {
    if (dvm_function) {
        *sp_p = base 
                + dvm_function->parameter_count 
                + (dvm_function->is_method ? 1 : 0)
                + CALL_INFO_ALIGN_SIZE
                + dvm_function->local_variable_count;
    } else {
        *sp_p = 0;
    }
}

static DVM_Boolean throw_in_try(DVM_VirtualMachine *dvm, DVM_Executable *executable,
                                ExecutableEntry *executable_entry, 
                                Function *function, int *pc_p,
                                int *sp_p, int base) {
    DVM_CodeBlock *code_block;
    DVM_Function *dvm_function = NULL;

    if (function) {
        code_block = &(function->u.diksam_function.executable
                                                    ->executable
                                                    ->function[function->u.diksam_function.index])
                                                    .code_block;
        dvm_function = &(function->u.diksam_function.executable
                                                    ->executable
                                                    ->function[function->u.diksam_function.index]);
    } else {
        code_block = &executable->top_level;
    }

    int exception_idnex = dvm->current_exception.v_table->executable_class->class_index;
    
    int try_index, catch_index;
    DVM_Boolean throw_in_try = DVM_FALSE;
    DVM_Boolean throw_in_catch = DVM_FALSE;

    for (try_index = 0; try_index < code_block->try_size; try_index++) {
        if ((*pc_p) >= code_block->try[try_index].try_start_pc 
            && (*pc_p) <= code_block->try[try_index].try_end_pc) {
            throw_in_try = DVM_TRUE;
            break;
        }
        for (catch_index = 0; 
            catch_index < code_block->try[try_index].catch_count;
            catch_index++) {
            if ((*pc_p) >= code_block->try[try_index].catch_clause[catch_index].start_pc
                && (*pc_p) <= code_block->try[try_index].catch_clause[catch_index].end_pc) {
                throw_in_catch = DVM_TRUE;
                break;
            }
        }
    }

    if (try_index == code_block->try_size) {
        return DVM_FALSE;
    }

    DBG_assert(throw_in_try || throw_in_catch, ("bad flags"));

    if (throw_in_try) {
        for (catch_index = 0; 
            catch_index < code_block->try[try_index].catch_count;
            catch_index++) {
            int class_index_in_exe = code_block->try[try_index].catch_clause[catch_index].class_index;
            int class_index_in_dvm = executable_entry->class_table[class_index_in_exe];
            if (exception_idnex == class_index_in_dvm
                || check_instanceof(dvm, &dvm->current_exception,
                                    class_index_in_dvm)) {
                *pc_p = code_block->try[try_index].catch_clause[catch_index].start_pc;
                reset_stack_pointer(dvm_function, sp_p, base);
                STO_WRITE(dvm, 0, dvm->current_exception);
                dvm->stack.stack_pointer++;
                dvm->current_exception = dvm_null_object_ref;
                return DVM_TRUE;
            }
        }
    }

    *pc_p = code_block->try[try_index].finally_start_pc;
    reset_stack_pointer(dvm_function, sp_p, base);

    return DVM_TRUE;
}

static void add_stack_trace(DVM_VirtualMachine *dvm, DVM_Executable *executable, 
                            Function *function, int pc) {
    int line_number = dvm_conv_pc_to_line_number(executable, function, pc);
    int class_index = DVM_search_class(dvm, DVM_DIKSAM_DEFAULT_PACKAGE, DIKSAM_STACK_TRACE_CLASS);
    DVM_ObjectRef stack_trace = dvm_create_class_object_i(dvm, class_index);
    STO_WRITE(dvm, 0, stack_trace);
    dvm->stack.stack_pointer++;
    
    int line_number_index = DVM_get_field_index(dvm, stack_trace, "line_number");
    stack_trace.data->u.class_object.field[line_number_index].int_value = line_number;
    
    int file_name_index = DVM_get_field_index(dvm, stack_trace, "file_name");
    DVM_Char *wc_string = dvm_mbstowcs_alloc(dvm, executable->path);
    stack_trace.data->u.class_object.field[file_name_index].object = dvm_create_dvm_string_i(dvm, wc_string);

    int function_name_index = DVM_get_field_index(dvm, stack_trace, "function_name");
    char *function_name;
    if (function) {
        function_name = executable->function[function->u.diksam_function.index].name;
    } else {
        function_name = "top_level";
    }

    wc_string = dvm_mbstowcs_alloc(dvm, function_name);
    stack_trace.data->u.class_object.field[function_name_index].object 
        = dvm_create_dvm_string_i(dvm, wc_string);

    int stack_trace_index = DVM_get_field_index(dvm, dvm->current_exception, "stack_trace");
    DVM_Object *stack_trace_array = dvm->current_exception.data->u.class_object.field[stack_trace_index].object.data;
    
    int array_size = DVM_array_size(dvm, stack_trace_array);
    DVM_Value value;
    value.object = stack_trace;
    DVM_array_insert(dvm, stack_trace_array, array_size, value);

    dvm->stack.stack_pointer--;
}

static DVM_Value invoke_diksam_function_from_native(DVM_VirtualMachine *dvm,
                                                        Function *callee,
                                                        DVM_ObjectRef object,
                                                        DVM_Value *args) {
    ExecutableEntry *current_executable_backup = dvm->current_executable;
    Function *current_function_backup = dvm->current_function;
    int current_pc_backup = dvm->pc;
    DVM_Executable *dvm_executable = callee->u.diksam_function.executable->executable;
    DVM_Function *dvm_function = &dvm_executable->function[callee->u.diksam_function.index];
    int base = dvm->stack.stack_pointer;
    for (int i = 0; i < dvm_function->parameter_count; i++) {
        dvm->stack.stack[dvm->stack.stack_pointer] = args[i];
        dvm->stack.pointer_flags[dvm->stack.stack_pointer] = is_pointer_type(dvm_function->parameter[i].type);
        dvm->stack.stack_pointer++;
    }

    if (!is_null_pointer(&object)) {
        STO_WRITE(dvm, 0, object);
        dvm->stack.stack_pointer++;
    }
    
    CallInfo *call_info = (CallInfo*)&dvm->stack.stack[dvm->stack.stack_pointer];
    call_info->caller = dvm->current_function;
    call_info->caller_address = CALL_FROM_NATIVE;
    call_info->base = 0; // dummy
    for (int i = 0; i < CALL_INFO_ALIGN_SIZE; i++) {
        dvm->stack.pointer_flags[dvm->stack.stack_pointer + i] = DVM_FALSE;
        dvm->pc = 0;
        dvm->current_executable = callee->u.diksam_function.executable;
    }

    dvm->stack.stack_pointer += CALL_INFO_ALIGN_SIZE;
    initialize_local_variables(dvm, dvm_function, dvm->stack.stack_pointer);
    dvm->stack.stack_pointer += dvm_function->local_variable_count;
    DVM_Byte *code = dvm_executable->function[callee->u.diksam_function.index].code_block.code;
    int code_size = dvm_executable->function[callee->u.diksam_function.index].code_block.code_size;

    DVM_Value result = dvm_execute_i(dvm, callee, code, code_size, base);
    dvm->stack.stack_pointer--;
    current_executable_backup = dvm->current_executable;
    current_function_backup = dvm->current_function;
    current_pc_backup = dvm->pc;

    return result;
}

static DVM_Boolean do_throw(DVM_VirtualMachine *dvm, Function **function_p,
                         DVM_Byte **code_p, int *code_size_p, int *pc_p, 
                         int *base_p, ExecutableEntry **executable_entry_p, 
                         DVM_Executable **executable_p, DVM_ObjectRef *exception) {
    DVM_Boolean in_try;
    dvm->current_exception = *exception;

    while (1) {
        in_try = throw_in_try(dvm, *executable_p, *executable_entry_p, *function_p,
                                pc_p, &dvm->stack.stack_pointer, *base_p);
        if (in_try) break;

        if (*function_p) {
            add_stack_trace(dvm, *executable_p, *function_p, *pc_p);
            if (do_return(dvm, function_p, code_p, code_size_p, pc_p, base_p,
                            executable_entry_p, executable_p)) {
                return DVM_TRUE;
            }
        } else {
            int function_index = dvm_search_function(dvm, DVM_DIKSAM_DEFAULT_PACKAGE, DIKSAM_PRINT_STACK_TRACE_FUNCTION);
            add_stack_trace(dvm, *executable_entry_p, *function_p, *pc_p);

            invoke_diksam_function_from_native(dvm, dvm->function[function_index],
                                                dvm->current_exception, NULL);
            exit(1);
        }
    }

    return DVM_FALSE;
}

static DVM_Boolean throw_null_pointer_exception(DVM_VirtualMachine *dvm, Function **function_p,
                                                DVM_Byte **code_p, int *code_size_p, int *pc_p,
                                                int *base_p, ExecutableEntry **executable_entry, DVM_Executable **executable_p) {
// TODO: coding
}

static void clear_stack_trace(DVM_VirtualMachine *dvm, DVM_ObjectRef *ex) {
    int stack_trace_index = DVM_get_field_index(dvm, *ex, "stack_trace");
    ex->data->u.class_object.field[stack_trace_index].object
        = dvm_create_array_double_i(dvm, 0);
}

DVM_Value dvm_execute_i(DVM_VirtualMachine *dvm, Function *function,
                         DVM_Byte *code, int code_size, int base) {

    DBG_debug_write((DBG_DEBUG_LEVEL_DEFAULT, "EXECUTE_START code_size: %d\n", code_size));
    
    DVM_Value ret;

    ExecutableEntry *executable_entry = dvm->current_executable;
    DVM_Executable *executable = dvm->current_executable->executable;

    int pc = dvm->pc;

    while (pc < code_size) {
        DBG_debug_write((DBG_DEBUG_LEVEL_DEFAULT, "execute pc: %d opcode: %s \n", pc, dvm_opcode_info[code[pc]].mnemonic));

        switch ((DVM_Opcode) code[pc]) {
            case DVM_PUSH_INT_1BYTE:
                STI_WRITE(dvm, 0, code[pc + 1]);
                dvm->stack.stack_pointer++;
                pc += 2;
                break;
            case DVM_PUSH_INT_2BYTE:
                STI_WRITE(dvm, 0, GET_2BYTE_INT(&code[pc + 1]));
                dvm->stack.stack_pointer++;
                pc += 3;
                break;
            case DVM_PUSH_INT:
                STI_WRITE(dvm, 0,
                          executable->constant_pool[GET_2BYTE_INT(&code[pc + 1])].u.c_int);
                dvm->stack.stack_pointer++;
                pc += 3;
                break;
            case DVM_PUSH_DOUBLE_0:
                STD_WRITE(dvm, 0, 0.0);
                dvm->stack.stack_pointer++;
                pc++;
                break;
            case DVM_PUSH_DOUBLE_1:
                STD_WRITE(dvm, 0, 1.0);
                dvm->stack.stack_pointer++;
                pc++;
                break;
            case DVM_PUSH_DOUBLE:
                STD_WRITE(dvm, 0,
                          executable->constant_pool[GET_2BYTE_INT(&code[pc + 1])].u.c_double);
                dvm->stack.stack_pointer++;
                pc += 3;
                break;
            case DVM_PUSH_STRING:
                STO_WRITE(dvm, 0,
                          dvm_literal_to_dvm_string_i(dvm,
                                                      executable->constant_pool[GET_2BYTE_INT(
                                                              &code[pc + 1])].u.c_string));
                dvm->stack.stack_pointer++;
                pc += 3;
                break;
            case DVM_PUSH_NULL:
                STO_WRITE(dvm, 0, dvm_null_object_ref);
                dvm->stack.stack_pointer++;
                pc++;
                break;
            case DVM_PUSH_STACK_INT:
                STI_WRITE(dvm, 0, STI_I(dvm, base + GET_2BYTE_INT(&code[pc + 1])));
                dvm->stack.stack_pointer++;
                pc += 3;
                break;
            case DVM_PUSH_STACK_DOUBLE:
                STD_WRITE(dvm, 0, STD_I(dvm, base + GET_2BYTE_INT(&code[pc + 1])));
                dvm->stack.stack_pointer++;
                pc += 3;
                break;
            case DVM_PUSH_STACK_OBJECT:
                STO_WRITE(dvm, 0, STO_I(dvm, base + GET_2BYTE_INT(&code[pc + 1])));
                dvm->stack.stack_pointer++;
                pc += 3;
                break;
            case DVM_POP_STACK_INT:
                STI_WRITE_I(dvm, base + GET_2BYTE_INT(&code[pc + 1]), STI(dvm, -1));
                dvm->stack.stack_pointer--;
                pc += 3;
                break;
            case DVM_POP_STACK_DOUBLE:
                STD_WRITE_I(dvm, base + GET_2BYTE_INT(&code[pc + 1]), STD(dvm, -1));
                dvm->stack.stack_pointer--;
                pc += 3;
                break;
            case DVM_POP_STACK_OBJECT:
                STO_WRITE_I(dvm, base + GET_2BYTE_INT(&code[pc + 1]), STO(dvm, -1));
                dvm->stack.stack_pointer--;
                pc += 3;
                break;
            case DVM_PUSH_STATIC_INT:
                STI_WRITE(dvm, 0,
                          executable_entry->static_v.variable[GET_2BYTE_INT(&code[pc + 1])].int_value);
                dvm->stack.stack_pointer++;
                pc += 3;
                break;
            case DVM_PUSH_STATIC_DOUBLE:
                STD_WRITE(dvm, 0,
                          executable_entry->static_v.variable[GET_2BYTE_INT(&code[pc + 1])].double_value);
                dvm->stack.stack_pointer++;
                pc += 3;
                break;
            case DVM_PUSH_STATIC_OBJECT:
                STO_WRITE(dvm, 0,
                          executable_entry->static_v.variable[GET_2BYTE_INT(&code[pc + 1])].object);
                dvm->stack.stack_pointer++;
                pc += 3;
                break;
            case DVM_POP_STATIC_INT:
                executable_entry->static_v.variable[GET_2BYTE_INT(&code[pc + 1])].int_value = STI(dvm, -1);
                dvm->stack.stack_pointer--;
                pc += 3;
                break;
            case DVM_POP_STATIC_DOUBLE:
                executable_entry->static_v.variable[GET_2BYTE_INT(&code[pc + 1])].double_value = STD(dvm, -1);
                dvm->stack.stack_pointer--;
                pc += 3;
                break;
            case DVM_POP_STATIC_OBJECT:
                executable_entry->static_v.variable[GET_2BYTE_INT(&code[pc + 1])].object = STO(dvm, -1);
                dvm->stack.stack_pointer--;
                pc += 3;
                break;
            case DVM_PUSH_ARRAY_INT: {
                DVM_ObjectRef array = STO(dvm, -2);
                int index = STI(dvm, -1);
                int int_value;
                restore_pc(dvm, executable_entry, function, pc);
                int_value = DVM_array_get_int(dvm, array, index);
                STI_WRITE(dvm, -2, int_value);
                dvm->stack.stack_pointer--;
                pc++;
                break;
            }
            case DVM_PUSH_ARRAY_DOUBLE: {
                DVM_ObjectRef array = STO(dvm, -2);
                int index = STI(dvm, -1);
                double double_value;
                restore_pc(dvm, executable_entry, function, pc);
                double_value = DVM_array_get_double(dvm, array, index);
                STI_WRITE(dvm, -2, double_value);
                dvm->stack.stack_pointer--;
                pc++;
                break;
            }
            case DVM_PUSH_ARRAY_OBJECT: {
                DVM_ObjectRef array = STO(dvm, -2);
                int index = STI(dvm, -1);
                DVM_ObjectRef object;
                restore_pc(dvm, executable_entry, function, pc);
                object = DVM_array_get_object(dvm, array, index);
                STO_WRITE(dvm, -2, object);
                dvm->stack.stack_pointer--;
                pc++;
                break;
            }
            case DVM_POP_ARRAY_INT: {
                int value = STI(dvm, -3);
                DVM_ObjectRef array = STO(dvm, -2);
                int index = STI(dvm, -1);
                restore_pc(dvm, executable_entry, function, pc);
                DVM_array_set_int(dvm, array, index, value);
                dvm->stack.stack_pointer -= 3;
                pc++;
                break;
            }
            case DVM_POP_ARRAY_DOUBLE: {
                double value = STD(dvm, -3);
                DVM_ObjectRef array = STO(dvm, -2);
                int index = STI(dvm, -1);
                restore_pc(dvm, executable_entry, function, pc);
                DVM_array_set_double(dvm, array, index, value);
                dvm->stack.stack_pointer -= 3;
                pc++;
                break;
            }
            case DVM_POP_ARRAY_OBJECT: {
                DVM_ObjectRef value = STO(dvm, -3);
                DVM_ObjectRef array = STO(dvm, -2);
                int index = STI(dvm, -1);
                restore_pc(dvm, executable_entry, function, pc);
                DVM_array_set_object(dvm, array, index, value);
                dvm->stack.stack_pointer -= 3;
                pc++;
                break;
            }
            case DVM_PUSH_FIELD_INT: {
                DVM_ObjectRef object = STO(dvm, -1);
                int index = GET_2BYTE_INT(&code[pc + 1]);
                check_null_pointer(executable, function, pc, &object);
                STI_WRITE(dvm, -1, object.data->u.class_object.field[index].int_value);
                pc += 3;
                break;
            }
            case DVM_PUSH_FIELD_DOUBLE: {
                DVM_ObjectRef object = STO(dvm, -1);
                int index = GET_2BYTE_INT(&code[pc + 1]);
                check_null_pointer(executable, function, pc, &object);
                STD_WRITE(dvm, -1, object.data->u.class_object.field[index].double_value);
                pc += 3;
                break;
            }
            case DVM_PUSH_FIELD_OBJECT: {
                DVM_ObjectRef object = STO(dvm, -1);
                int index = GET_2BYTE_INT(&code[pc + 1]);
                check_null_pointer(executable, function, pc, &object);
                STO_WRITE(dvm, -1, object.data->u.class_object.field[index].object);
                pc += 3;
                break;
            }
            case DVM_POP_FIELD_INT: {
                DVM_ObjectRef object = STO(dvm, -1);
                int index = GET_2BYTE_INT(&code[pc + 1]);
                check_null_pointer(executable, function, pc, &object);
                object.data->u.class_object.field[index].int_value = STI(dvm, -2);
                dvm->stack.stack_pointer -= 2;
                pc += 3;
                break;
            }
            case DVM_POP_FIELD_DOUBLE: {
                DVM_ObjectRef object = STO(dvm, -1);
                int index = GET_2BYTE_INT(&code[pc + 1]);
                if (is_null_pointer(&object)) {
                    // TODO
                    printf("is_null_pointer\n");
                } else {

                }
                check_null_pointer(executable, function, pc, &object);
                object.data->u.class_object.field[index].double_value = STD(dvm, -2);
                dvm->stack.stack_pointer -= 2;
                pc += 3;
                break;
            }
            case DVM_POP_FIELD_OBJECT: {
                DVM_ObjectRef object = STO(dvm, -1);
                int index = GET_2BYTE_INT(&code[pc + 1]);
                check_null_pointer(executable, function, pc, &object);
                object.data->u.class_object.field[index].object = STO(dvm, -2);
                dvm->stack.stack_pointer -= 2;
                pc += 3;
                break;
            }
            case DVM_ADD_INT:
                STI(dvm, -2) = STI(dvm, -2) + STI(dvm, -1);
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_ADD_DOUBLE:
                STD(dvm, -2) = STD(dvm, -2) + STD(dvm, -1);
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_ADD_STRING:
                STO(dvm, -2) = chain_string(dvm, STO(dvm, -2), STO(dvm, -1));
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_SUB_INT:
                STI(dvm, -2) = STI(dvm, -2) - STI(dvm, -1);
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_SUB_DOUBLE:
                STD(dvm, -2) = STD(dvm, -2) - STD(dvm, -1);
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_MUL_INT:
                STI(dvm, -2) = STI(dvm, -2) * STI(dvm, -1);
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_MUL_DOUBLE:
                STD(dvm, -2) = STD(dvm, -2) * STD(dvm, -1);
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_DIV_INT:
                STI(dvm, -2) = STI(dvm, -2) / STI(dvm, -1);
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_DIV_DOUBLE:
                STD(dvm, -2) = STD(dvm, -2) / STD(dvm, -1);
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_MOD_INT:
                STI(dvm, -2) = STI(dvm, -2) % STI(dvm, -1);
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_MOD_DOUBLE:
                STD(dvm, -2) = fmod(STD(dvm, -2), STD(dvm, -1));
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_MINUS_INT:
                STI(dvm, -1) = -STI(dvm, -1);
                pc++;
                break;
            case DVM_MINUS_DOUBLE:
                STD(dvm, -1) = -STD(dvm, -1);
                pc++;
                break;
            case DVM_INCREMENT:
                STI(dvm, -1)++;
                pc++;
                break;
            case DVM_DECREMENT:
                STI(dvm, -1)--;
                pc++;
                break;
            case DVM_CAST_INT_TO_DOUBLE:
                STD(dvm, -1) = (double) STI(dvm, -1);
                pc++;
                break;
            case DVM_CAST_DOUBLE_TO_INT:
                STI(dvm, -1) = (int) STD(dvm, -1);
                pc++;
                break;
            case DVM_CAST_BOOLEAN_TO_STRING:
                if (STI(dvm, -1)) {
                    STO_WRITE(dvm, -1,
                              dvm_literal_to_dvm_string_i(dvm, TRUE_STRING));
                } else {
                    STO_WRITE(dvm, -1,
                              dvm_literal_to_dvm_string_i(dvm, FALSE_STRING));
                }
                pc++;
                break;
            case DVM_CAST_INT_TO_STRING: {
                char buf[LINE_BUF_SIZE];
                DVM_Char *wc_str;
                sprintf(buf, "%d", STI(dvm, -1));
                wc_str = dvm_mbstowcs_alloc(dvm, buf);
                STO_WRITE(dvm, -1, dvm_create_dvm_string_i(dvm, wc_str));
                pc++;
                break;
            }
            case DVM_CAST_DOUBLE_TO_STRING: {
                char buf[LINE_BUF_SIZE];
                DVM_Char *wc_str;
                sprintf(buf, "%f", STD(dvm, -1));
                wc_str = dvm_mbstowcs_alloc(dvm, buf);
                STO_WRITE(dvm, -1, dvm_create_dvm_string_i(dvm, wc_str));
                pc++;
                break;
            }
            case DVM_UP_CAST: {
                // TODO
                DBG_debug_write((DBG_DEBUG_LEVEL_DEFAULT, "TODO: DVM_UP_CAST unsuppet"));
                break;
            }
            case DVM_DOWN_CAST: {
                // TODO
            }
            case DVM_EQ_INT:
                STI(dvm, -2) = (STI(dvm, -2) == STI(dvm, -1));
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_EQ_DOUBLE:
                STD(dvm, -2) = (STD(dvm, -2) == STD(dvm, -1));
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_EQ_STRING:
                STI_WRITE(dvm, -2, 0 == dvm_wcscmp(STO(dvm, -2).data->u.string.string,
                                                   STO(dvm, -1).data->u.string.string));
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_GT_INT:
                STI(dvm, -2) = (STI(dvm, -2) > STI(dvm, -1));
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_GT_DOUBLE:
                STD(dvm, -2) = (STD(dvm, -2) > STD(dvm, -1));
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_GT_STRING:
                STI_WRITE(dvm, -2, 0 < dvm_wcscmp(STO(dvm, -2).data->u.string.string,
                                                  STO(dvm, -1).data->u.string.string));
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_GE_INT:
                STI(dvm, -2) = (STI(dvm, -2) >= STI(dvm, -1));
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_GE_DOUBLE:
                STD(dvm, -2) = (STD(dvm, -2) >= STD(dvm, -1));
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_GE_STRING:
                STI_WRITE(dvm, -2, 0 <= dvm_wcscmp(STO(dvm, -2).data->u.string.string,
                                                   STO(dvm, -1).data->u.string.string));
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_LT_INT:
                STI(dvm, -2) = (STI(dvm, -2) < STI(dvm, -1));
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_LT_DOUBLE:
                STD(dvm, -2) = (STD(dvm, -2) < STD(dvm, -1));
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_LT_STRING:
                STI_WRITE(dvm, -2, 0 > dvm_wcscmp(STO(dvm, -2).data->u.string.string,
                                                  STO(dvm, -1).data->u.string.string));
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_LE_INT:
                STI(dvm, -2) = (STI(dvm, -2) <= STI(dvm, -1));
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_LE_DOUBLE:
                STD(dvm, -2) = (STD(dvm, -2) <= STD(dvm, -1));
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_LE_STRING:
                STI_WRITE(dvm, -2, 0 >= dvm_wcscmp(STO(dvm, -2).data->u.string.string,
                                                   STO(dvm, -1).data->u.string.string));
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_NE_INT:
                STI(dvm, -2) = (STI(dvm, -2) != STI(dvm, -1));
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_NE_DOUBLE:
                STD(dvm, -2) = (STD(dvm, -2) != STD(dvm, -1));
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_NE_STRING:
                STI_WRITE(dvm, -2, 0 != dvm_wcscmp(STO(dvm, -2).data->u.string.string,
                                                   STO(dvm, -1).data->u.string.string));
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_LOGICAL_AND:
                STI(dvm, -2) = (STI(dvm, -2) && STI(dvm, -1));
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_LOGICAL_OR:
                STI(dvm, -2) = (STI(dvm, -2) || STI(dvm, -1));
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_LOGICAL_NOT:
                STI(dvm, -1) = !STI(dvm, -1);
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_POP:
                dvm->stack.stack_pointer--;
                pc++;
                break;
            case DVM_DUPLICATE:
                dvm->stack.stack[dvm->stack.stack_pointer] = dvm->stack.stack[dvm->stack.stack_pointer - 1];
                dvm->stack.pointer_flags[dvm->stack.stack_pointer] = dvm->stack.pointer_flags[dvm->stack.stack_pointer - 1];
                dvm->stack.stack_pointer++;
                pc++;
                break;
            case DVM_DUPLICATE_OFFSET: {
                int offset = GET_2BYTE_INT(&code[pc + 1]);
                dvm->stack.stack[dvm->stack.stack_pointer] = dvm->stack.stack[dvm->stack.stack_pointer - 1 - offset];
                dvm->stack.pointer_flags[dvm->stack.stack_pointer] = dvm->stack.pointer_flags[dvm->stack.stack_pointer - 1 - offset];
                dvm->stack.stack_pointer++;
                pc += 3;
                break;
            }
            case DVM_JUMP:
                pc = GET_2BYTE_INT(&code[pc + 1]);
                break;
            case DVM_JUMP_IF_TRUE:
                if (STI(dvm, -1)) {
                    pc = GET_2BYTE_INT(&code[pc + 1]);
                } else {
                    pc += 3;
                }
                dvm->stack.stack_pointer--;
                break;
            case DVM_JUMP_IF_FALSE:
                if (STI(dvm, -1)) {
                    pc += 3;
                } else {
                    pc = GET_2BYTE_INT(&code[pc + 1]);
                }
                dvm->stack.stack_pointer--;
                break;
            case DVM_PUSH_FUNCTION:
                STI_WRITE(dvm, 0, GET_2BYTE_INT(&code[pc + 1]));
                dvm->stack.stack_pointer++;
                pc += 3;
                break;
            case DVM_PUSH_METHOD: {
                DVM_ObjectRef object = STO(dvm, -1);
                int index = GET_2BYTE_INT(&code[pc + 1]);
                DBG_debug_write((DBG_DEBUG_LEVEL_DEFAULT, "TODO: DVM_PUSH_METHOD\n", object));

                if (is_null_pointer(&object)) {
                    DBG_debug_write((DBG_DEBUG_LEVEL_DEFAULT, "TODO: DVM_PUSH_METHOD is_null_pointer\n"));

                    if (throw_null_pointer_exception(dvm, &function, &code, &code_size, 
                                                    &pc, &base, &executable_entry, &executable)) {
                        goto EXECUTE_END;
                    }
                } else {
                    DBG_debug_write((DBG_DEBUG_LEVEL_DEFAULT, "TODO: DVM_PUSH_METHOD !is_null_pointer table_size: %d\n", object.v_table->table_size));
                    STI_WRITE(dvm, 0, object.v_table->table[index].index);
                    dvm->stack.stack_pointer++;
                    pc += 3;
                }
                DBG_debug_write((DBG_DEBUG_LEVEL_DEFAULT, "TODO: DVM_PUSH_METHOD over\n"));

                break;
            }
            case DVM_INVOKE: {
                int function_index = STI(dvm, -1);
                
                if (dvm->function[function_index]->kind == NATIVE_FUNCTION) {
                    restore_pc(dvm, executable_entry, function, pc);
	                invoke_native_function(dvm, function, dvm->function[function_index], pc, &dvm->stack.stack_pointer,
	                                       base);
                    pc++;
                } else {
                    invoke_diksam_function(dvm, &function, dvm->function[function_index],
                                           &code, &code_size, &pc, &dvm->stack.stack_pointer,
                                           &base, &executable_entry, &executable);
                }
                break;
            }
            case DVM_RETURN:
	            if (return_function(dvm, &function, &code, &code_size, &pc, &base, &executable_entry, &executable)) {
		            ret = dvm->stack.stack[dvm->stack.stack_pointer - 1];
		            goto EXECUTE_END;
	            }
                break;
            case DVM_NEW: {
                int class_index = GET_2BYTE_INT(&code[pc + 1]);
                STO_WRITE(dvm, 0, dvm_create_class_object_i(dvm, class_index));
                dvm->stack.stack_pointer++;
                pc += 3;
                break;
            }
            case DVM_NEW_ARRAY: {
                int dim = code[pc + 1];
                DVM_TypeSpecifier *type = &executable->type_specifier[GET_2BYTE_INT(&code[pc + 2])];
                DVM_ObjectRef array;
                restore_pc(dvm, executable_entry, function, pc);
                array = create_array(dvm, dim, type);
                dvm->stack.stack_pointer -= dim;
                STO_WRITE(dvm, 0, array);
                dvm->stack.stack_pointer++;
                pc += 4;
                break;
            }
            case DVM_NEW_ARRAY_LITERAL_INT: {
                int size = GET_2BYTE_INT(&code[pc + 1]);
                DVM_ObjectRef array;
                restore_pc(dvm, executable_entry, function, pc);
                array = create_array_literal_int(dvm, size);
                dvm->stack.stack_pointer -= size;
                STO_WRITE(dvm, 0, array);
                dvm->stack.stack_pointer++;
                pc += 3;
                break;
            }
            case DVM_NEW_ARRAY_LITERAL_DOUBLE: {
                int size = GET_2BYTE_INT(&code[pc + 1]);
                DVM_ObjectRef array;
                restore_pc(dvm, executable_entry, function, pc);
                array = create_array_literal_double(dvm, size);
                dvm->stack.stack_pointer -= size;
                STO_WRITE(dvm, 0, array);
                dvm->stack.stack_pointer++;
                pc += 3;
                break;
            }
            case DVM_NEW_ARRAY_LITERAL_OBJECT: {
                int size = GET_2BYTE_INT(&code[pc + 1]);
                DVM_ObjectRef array;
                restore_pc(dvm, executable_entry, function, pc);
                array = create_array_literal_object(dvm, size);
                dvm->stack.stack_pointer -= size;
                STO_WRITE(dvm, 0, array);
                dvm->stack.stack_pointer++;
                pc += 3;
                break;
            }
            case DVM_SUPER: {
                DVM_ObjectRef *object = &STO(dvm, -1);
                ExecutableClass *this_class = object->v_table->executable_class;

                int target_index = GET_2BYTE_INT(&code[pc + 1]);

                object->v_table = this_class->super_class->class_table;

                pc++;
                break;
            }
            case DVM_INSTANCEOF: {
                DVM_ObjectRef *object = &STO(dvm, -1);
                int target_index = GET_2BYTE_INT(&code[pc + 1]);
                if (object->v_table->executable_class->class_index == target_index) {
                    STI_WRITE(dvm, -1, DVM_TRUE);
                } else {
                    STI_WRITE(dvm, -1, check_instanceof(dvm, object, target_index));
                }
                pc += 3;
                break;
            }
            case DVM_THROW: {
                DVM_ObjectRef* exception = &STO(dvm, -1);
                clear_stack_trace(dvm, exception);
                if (do_throw(dvm, &function, &code, &code_size, &pc,
                            &base, &executable_entry, &executable, 
                            exception)) {
                    goto EXECUTE_END;
                }
                break;
            }
            case DVM_RETHROW : {
                DVM_ObjectRef* exception = &STO(dvm, -1);
                if (do_throw(dvm, &function, &code, &code_size, &pc,
                            &base, &executable_entry, &executable, 
                            exception)) {
                    goto EXECUTE_END;
                }
                break;
            }
            case DVM_GO_FINALLY : {
                STI_WRITE(dvm, 0, pc);
                dvm->stack.stack_pointer++;
                pc = GET_2BYTE_INT(&code[pc + 1]);
                break;
            }
            case DVM_FINALLY_END : {
                if (!is_object_null(dvm->current_exception)) {
                    if (do_throw(dvm, &function, &code, &code_size, &pc,
                            &base, &executable_entry, &executable, 
                            &dvm->current_exception)) {
                        goto EXECUTE_END;
                    }
                } else {
                    pc = STI(dvm, -1) + 3;
                    dvm->stack.stack_pointer--;
                }
                break;
            }
            default:
                DBG_assert(0, ("code[pc]..%d\n", code[pc]));
        }
    }

    EXECUTE_END:;

    DBG_debug_write((DBG_DEBUG_LEVEL_DEFAULT, "EXECUTE_END\n"));
    return ret;
}

DVM_Value DVM_execute(DVM_VirtualMachine *dvm) {
    DVM_Value ret;
    dvm->current_executable = dvm->top_level;
    dvm->current_function = NULL;
    dvm->pc = 0;
    dvm_expand_stack(dvm, dvm->top_level->executable->top_level.need_stack_size);
    dvm_execute_i(dvm, NULL, dvm->top_level->executable->top_level.code, dvm->top_level->executable->top_level.code_size, 0);

    // ret
    return ret;
}

void dvm_push_object(DVM_VirtualMachine *dvm, DVM_Value value) {
    STO_WRITE(dvm, 0, value.object);
    dvm->stack.stack_pointer++;
}

DVM_Value dvm_pop_object(DVM_VirtualMachine *dvm) {
    DVM_Value result;
    result.object = STO(dvm, -1);
    dvm->stack.stack_pointer--;

    return result;
}

void DVM_dispose_executable_list(DVM_ExecutableList *executable_list) {
    DVM_ExecutableItem *executable_temp;
    while (executable_list->list) {
        executable_temp = executable_list->list;
        executable_list->list = executable_temp->next;
        dvm_dispose_executable(executable_temp->executable);
        MEM_free(executable_temp);
    }
    MEM_free(executable_list);
}

static void dispose_v_table(DVM_VTable *v_table) {
    for (int i = 0; i < v_table->table_size; i++) {
        MEM_free(v_table->table[i].name);
    }

    MEM_free(v_table->table);
    MEM_free(v_table);
}

void DVM_dispose_virtual_machine(DVM_VirtualMachine *dvm) {
    while (dvm->executable_entry) {
        ExecutableEntry *executable_entry_temp = dvm->executable_entry;
        dvm->executable_entry = executable_entry_temp->next;

        // MEM_free(executable_entry_temp->function_table);
        // MEM_free(executable_entry_temp->class_table);
        // MEM_free(executable_entry_temp->enum_table);
        // MEM_free(executable_entry_temp->constant_table);
        MEM_free(executable_entry_temp->static_v.variable);
        MEM_free(executable_entry_temp);
    }
    dvm_garbage_collect(dvm);

    MEM_free(dvm->stack.stack);
    MEM_free(dvm->stack.pointer_flags);

    for (int i = 0; i < dvm->function_count; i++) {
        MEM_free(dvm->function[i]->package_name);
        MEM_free(dvm->function[i]->name);
        MEM_free(dvm->function[i]);
    }
    MEM_free(dvm->function);

    for (int i = 0; i < dvm->class_count; i++) {
        MEM_free(dvm->_class[i]->package_name);
        MEM_free(dvm->_class[i]->name);
        dispose_v_table(dvm->_class[i]->class_table);
        for (int j = 0; j < dvm->_class[i]->interface_count; j++) {
            dispose_v_table(dvm->_class[i]->interface_v_table[j]);
        }
        MEM_free(dvm->_class[i]->interface_v_table);
        MEM_free(dvm->_class[i]->interface);
        MEM_free(dvm->_class[i]->field_type);
        MEM_free(dvm->_class[i]);
    }

    // for (int i = 0; i < dvm->constant_count; i++) {
    //     MEM_free(dvm->constant[i]->name);
    //     MEM_free(dvm->constant[i]->package_name);
    //     MEM_free(dvm->constant[i]);
    // }

    MEM_free(dvm->array_v_table->table);
    MEM_free(dvm->array_v_table);
    MEM_free(dvm->string_v_table->table);
    MEM_free(dvm->string_v_table);

    MEM_free(dvm->_class);

    MEM_free(dvm);
}
