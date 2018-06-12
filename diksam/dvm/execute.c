//
// Created by sulvto on 18-6-11.
//
#include "MEM.h"
#include "DBG.h"
#include "dvm_pri.h"

extern OpcodeInfo dvm_opcode_info[];

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
	dvm->executable = NULL;

	dvm_add_netive_functions(dvm);

	return dvm;
}

DVM_Value DVM_add_native_function(DVM_VirtualMachine *dvm, char *name, 
				DVM_NativeFunctionProc *proc, int arg_count) {
	dvm->function = MEM_realloc(dvm->function, sizeof(Function) * (dvm->function_count + 1));
	dvm->function[dvm->function_count].name = MEM_strdup(name);
	dvm->function[dvm->function_count].kind = NATIVE_FUNCTION;
	dvm->function[dvm->function_count].u.native_function.proc = proc;
	dvm->function[dvm->function_count].u.native_function.argument_count= arg_count;

	dvm->function_count++;
}

static void add_functions(DVM_VirtualMachine *dvm, DVM_Executable *executable) {
	int src_index;
	int dest_index;
	int function_count;

	for (src_index = 0; src_index < executable->function_count; src_index++) {
		if (executable->function[src_index].is_implemented) {
			function_count++;
			for (dest_index = 0; dest_index < dvm->function_count; dest_index++) {
				if (strcmp(dvm->function[dest_index].name,
						   executable->function[src_index].name) == 0) {
					dvm_error(NULL, NULL, NO_LINE_NUMBER_PC,
							  FUNCTION_MULTIPLE_DEFINE_ERR,
							  STRING_MESSAGE_ARGUMENT,
							  "name", dvm->function[dest_index].name,
							  MESSAGE_ARGUMENT_END);
				}
			}
		}
	}

	dvm->function = MEM_realloc(sizeof(Function) * (dvm->function_count + function_count));
	
	for (src_index = 0, dest_index = function_count;
		 src_index < executable->function_count;
		 src_index++) {
		if (!executable->function[src_index].is_implemented) {
			continue;
		}
		dvm->function[dest_index].name = MEM_strdup(executable->function[src_index].name);
		dvm->function[dest_index].u.diksam_function.executable = executable;
		dvm->function[dest_index].u.diksam_function.index = src_index;
		dest_index++;
	}
	dvm->function_count += function_count;
}

static int search_function(DVM_VirtualMachine *dvm, char *name) {
	for (int i = 0; i < dvm->function_count; i++) {
		if (strcmp(dvm->function[i].name, name) == 0) {
			return i;
		}
	}

	dvm_error(NULL, NULL, NO_LINE_NUMBER_PC, FUNCTION_NOT_FOUND_ERR, STRING_MESSAGE_ARGUMENT, "name", name, MESSAGE_ARGUMENT_END);

	return 0;
}

static void convert_code(DVM_VirtualMachine *dvm, 
				DVM_Executable *executable, 
				DVM_Byte *code, int code_size,
				DVM_Function *function) {
	for (int i = 0; i < code_size; i++) {
		if (code[i] == DVM_PUSH_STACK_INT
			||code[i] == DVM_PUSH_STACK_DOUBLE
			||code[i] == DVM_PUSH_STACK_STRING
			||code[i] == DVM_POP_STACK_INT
			||code[i] == DVM_POP_STACK_DOUBLE
			||code[i] == DVM_POP_STACK_STRING) {
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
			unsigned int func_inx = search_function(dvm, executable->function[inx_in_exe].name);
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

static void initialize_value(DVM_VirtualMachine *dvm, DVM_BasicType type, DVM_Value *value) {
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

static void add_static_variable(DVM_VirtualMachine *dvm, DVM_Executable *executable) {
	dvm->static_v.variable_count = executable->global_variable_count;
	dvm->static_v.variable = MEM_malloc(sizeof(DVM_Value) * executable->global_variable_count);

	for (int i = 0; i < executable->global_variable_count; i++) {
		if (executable->global_variable[i].type->basic_type == DVM_STRING_TYPE) {
			dvm->static_v.variable[i].object = NULL;
		}
	}
	
	for (int i = 0; i < executable->global_variable_count; i++) {
		initialize_value(dvm, executable->global_variable[i].type->basic_type,
						 &dvm->static_v.variable[i]);
    }
}

void DVM_add_executable(DVM_VirtualMachine *dvm, 
				DVM_Executable *executable) {
	dvm->executable = executable;
	add_functions(dvm, executable);
	convert_code(dvm, executable, executable->code, 
					executable->code_size, NULL);
	for (int i = 0; i < executable->function_count; i++) {
		convert_code(dvm, executable, executable->function[i].code,
						executable->function[i]->code_size,
						executable->function[i]);
	}

	add_static_variable(dvm, executable);
}

static void expand_stack(DVM_VirtualMachine *dvm, int need_stack_size) {
	int rest = dvm->stack.alloc_size - dvm->stack.stack_pointer;
	if (rest <= need_stack_size) {
		int revalue_up = ((rest / STACK_ALLOC_SIZE) + 1) * STACK_ALLOC_SIZE;
		dvm->stack.alloc_size += revalue_up;
		dvm->stack.stack = MEM_realloc(dvm->stack.stack
						, dvm->stack.alloc_size * sizeof(DVM_Value));
		dvm->stack.pointer_flags =
				MEM_realloc(dvm->stack.pointer_flags,
							dvm->stack.alloc_size * sizeof(DVM_Boolean));
	}
}

static void invoke_native_function(DVM_VirtualMachine *dvm, Function *function, int *sp_p) {
	DVM_Value *stack = dvm->stack.stack;
	int sp = *sp_p;
	DBG_assert(function->kind == NATIVE_FUNCTION, ("function->kind..%d", function->kind));

	DVM_Value ret = function->u.native_function.proc(dvm, function->u.native_function.argument_count, &stack[sp - function->u.native_function.argument_count - 1]);

	stack[sp - function->u.native_function.argument_count - 1] = ret;

	*sp_p = sp - (function->u.native_function.argument_count);
}

static void initialize_local_variable(DVM_VirtualMachine *dvm, 
				Function *function, int from_sp) {
	for (int i = 0, sp_index = from_sp; 
					i < function->local_variable_count; 
					i++, sp_index++) {
		dvm->stack.pointer_flags[i] = DVM_FALSE;
	}

	for (int i = 0, sp_index = from_sp; 
    				i < function->local_variable_count; 
    				i++, sp_index++) {
    	dvm->stack.pointer_flags[i] = DVM_FALSE;
		initialize_value(dvm, function->local_variable[i].type->basic_type,
						&dvm->stack.stack[sp_index]);

		if (function->local_variable[i].type->basic_type == DVM_STRING_TYPE) {
			dvm->stack.pointer_flags[i] = DVM_FALSE;
		}
    }
}

static void invoke_diksam_function(DVM_VirtualMachine *dvm, Function **caller_p, 
				Function *callee, DVM_Byte **code_p, int *code_size_p, int *pc_p, 
				int *sp_p, int *base_p, DVM_Executable **executable_p) {
	*executable_p = callee->u.diksam_function.executable;
	DVM_Function *callee_p = &(*executable_p)->function[callee->u.diksam_function.index];
	expand_stack(dvm, CALL_INFO_ALIGN_SIZE 
						+ callee_p->local_variable_count 
						+ (*executable_p)->function[callee->u.diksam_function.index].need_stack_size);
	callinfo = (CallInfo *)&dvm->stack.stack[*sp_p - 1];
	callinfo->caller = *caller_p;
	callinfo->caller_address = *pc_p;
	callinfo->base = *base_p;
	for (int i = 0; i < CALL_INFO_ALIGN_SIZE; i++) {
		dvm->stack.pointer_flags[*sp_p - 1 + i] = DVM_FALSE;
	}

	*base_p = *sp_p - callee_p->parameter_count - 1;
	*caller_p = callee;

	initialize_local_variable(dvm, callee_p, *sp_p + CALL_INFO_ALIGN_SIZE - 1);

	*sp_p += CALL_INFO_ALIGN_SIZE + callee_p->local_variable_count - 1;
	*pc_p = 0;

	*code_p = (*executable_p)->function_p[callee->u.diksam_function.index].code;
	*code_size_p = (*executable_p)->function_p[callee->u.diksam_function.index].code_size;
}

static void return_function(DVM_VirtualMachine *dvm, Function **function_p, 
				DVM_Byte **code_p, int *code_size_p, int *pc_p, int *sp_p, 
				int *base_p, DVM_Executable **executable_p) {
	DVM_Value return_value = dvm->stack.stack[(*sp_p) - 1];
	DVM_Function *caller_p;
	DVM_Function *callee_p = &(*executable_p)->function[(*function_p)->u.diksam_function.index];
	CallInfo *callinfo = (CallInfo*) &dvm->stack.stack[*sp_p - 1 
														- callee_p->local_variable_count 
														- CALL_INFO_ALIGN_SIZE];

	if (callinfo->caller) {
		*executable_p = callinfo->caller->u.diksam_function.execute;
		caller_p = &(*executable_p)->function[callinfo->caller_p->u.diksam_function.index];
		*code_p = caller_p->code;
		*code_size_p = caller_p->code_size;
	} else {
		*executable_p = dvm->execute;
		*code_p = dvm->execute->code;
		*code_size_p = dvm->execute->code_size;
	}
	*function_p = callinfo->caller;

	*pc_p = callinfo->caller_address + 1;
	*base_p = callinfo->base;
	*sp_p -=  callee_p->local_variable_count + CALL_INFO_ALIGN_SIZE + callee_p->parameter_count;

	dvm->stack.stack[*sp_p - 1] = return_value;
}

#define STI(dvm, sp) ((dvm)->stack.stack[((dvm)->stack.stack_pointer + (sp))].int_value)
#define STD(dvm, sp) ((dvm)->stack.stack[((dvm)->stack.stack_pointer + (sp))].double_value)
#define STO(dvm, sp) ((dvm)->stack.stack[((dvm)->stack.stack_pointer + (sp))].object)

#define STI_I(dvm, sp) ((dvm)->stack.stack[(sp)].int_value)
#define STD_I(dvm, sp) ((dvm)->stack.stack[(sp)].double_value)
#define STO_I(dvm, sp) ((dvm)->stack.stack[(sp)].object)

#define STI_WRITE(dvm, sp, r)	\
		((dvm)->stack.stack[(dvm)->stack.stack_pointer + (sp)].int_value = r, \
		 (dvm)->stack.pointer_flags[(dvm)->stack.stack_pointer + (sp)] = DVM_FALSE)
#define STD_WRITE(dvm, sp, r)	\
		((dvm)->stack.stack[(dvm)->stack.stack_pointer + (sp)].double_value = r, \
		 (dvm)->stack.pointer_flags[(dvm)->stack.stack_pointer + (sp)] = DVM_FALSE)
#define STO_WRITE(dvm, sp, r)	\
		((dvm)->stack.stack[(dvm)->stack.stack_pointer + (sp)].object = r, \
		 (dvm)->stack.pointer_flags[(dvm)->stack.stack_pointer + (sp)] = DVM_TRUE)

#define STI_WRITE_I(dvm, sp, r)	\
		((dvm)->stack.stack[(sp)].int_value = r, (dvm)->stack.pointer_flags[(sp)] = DVM_FALSE)
#define STD_WRITE_I(dvm, sp, r)	\
		((dvm)->stack.stack[(sp)].double_value = r, (dvm)->stack.pointer_flags[(sp)] = DVM_FALSE)
#define STO_WRITE_I(dvm, sp, r)	\
		((dvm)->stack.stack[(sp)].object = r, (dvm)->stack.pointer_flags[(sp)] = DVM_TRUE)

static DVM_Value execute(DVM_VirtualMachine *dvm, Function *function,
						 DVM_Byte *code, int code_size) {
	DVM_Value ret;
	DVM_Value *stack = dvm->stack.stack;
	DVM_Executable *executable = dvm->executable;

	int pc = dvm->pc;

	while (pc < code_size) {
		switch (code[pc]) {
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
                		executable->constant_pool[GET_2BYTE_INT(&code[pc + 1])].u.c_string));
                dvm->stack.stack_pointer++;                                              	
                pc += 3;                                                                 	
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
			case DVM_PUSH_STACK_STRING:
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
			case DVM_POP_STACK_STRING:
				STO_WRITE_I(dvm, base + GET_2BYTE_INT(&code[pc + 1]), STO(dvm, -1));
                dvm->stack.stack_pointer--;
                pc += 3;
				break;
			case DVM_PUSH_STATIC_INT:
				STI_WRITE(dvm, 0, 
						dvm->static_v.variable[GET_2BYTE_INT(&code[pc + 1])].int_value);
                dvm->stack.stack_pointer++;
                pc += 3;
				break;
			case DVM_PUSH_STATIC_DOUBLE:
				STD_WRITE(dvm, 0, 
                		dvm->static_v.variable[GET_2BYTE_INT(&code[pc + 1])].double_value);
                dvm->stack.stack_pointer++;
				pc += 3;
				break;
			case DVM_PUSH_STATIC_STRING:
				STO_WRITE(dvm, 0, 
                		dvm->static_v.variable[GET_2BYTE_INT(&code[pc + 1])].object);
                dvm->stack.stack_pointer++;
                pc += 3;
				break;
			case DVM_POP_STATIC_INT:
				dvm->static_v.variable[GET_2BYTE_INT(&code[pc + 1])].int_value = STI(dvm, -1);
				dvm->stack_pointer--;
				pc += 3;
				break;
			case DVM_POP_STATIC_DOUBLE:
				dvm->static_v.variable[GET_2BYTE_INT(&code[pc + 1])].double_value = STD(dvm, -1);
                dvm->stack_pointer--;
                pc += 3;
				break;
			case DVM_POP_STATIC_STRING:
				dvm->static_v.variable[GET_2BYTE_INT(&code[pc + 1])].object = STO(dvm, -1);	
                dvm->stack_pointer--;
                pc += 3;
				break;
			case DVM_ADD_INT:
				STI(dvm, -2) = STI(dvm, -2) + STI(dvm, -1);
				svm->stack_pointer--;
				pc++;
				break;
			case DVM_ADD_DOUBLE:
				STD(dvm, -2) = STD(dvm, -2) + STD(dvm, -1);
                svm->stack_pointer--;
                pc++;
				break;
			case DVM_ADD_STRING:
				STO(dvm, -2) = chain_string(dvm, STO(dvm, -2), STO(dvm, -1));	
                svm->stack_pointer--;
                pc++;
				break;
			case DVM_SUB_INT:
				STI(dvm, -2) = STI(dvm, -2) - STI(dvm, -1);
                svm->stack_pointer--;
                pc++;
				break;
			case DVM_SUB_DOUBLE:
				STD(dvm, -2) = STD(dvm, -2) - STD(dvm, -1);	
				svm->stack_pointer--;                     	
				pc++;                                     	
				break;
			case DVM_MUL_INT:
				STI(dvm, -2) = STI(dvm, -2) * STI(dvm, -1);	
				svm->stack_pointer--;                     	
				pc++;                                     	
				break;
			case DVM_MUL_DOUBLE:
				STD(dvm, -2) = STD(dvm, -2) * STD(dvm, -1);	
				svm->stack_pointer--;                     	
				pc++;                                     	
				break;
			case DVM_DIV_INT:
				STI(dvm, -2) = STI(dvm, -2) / STI(dvm, -1);	
				svm->stack_pointer--;                     	
				pc++;                                     	
				break;
			case DVM_DIV_DOUBLE:
				STD(dvm, -2) = STD(dvm, -2) / STD(dvm, -1);	
                svm->stack_pointer--;
                pc++;
				break;
			case DVM_MOD_INT:
				STI(dvm, -2) = STI(dvm, -2) % STI(dvm, -1);	
				svm->stack_pointer--;                     	
				pc++;                                     	
				break;
			case DVM_MOD_DOUBLE:
				STD(dvm, -2) = fmod(STD(dvm, -2), STD(dvm, -1));					
                svm->stack_pointer--;
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
					STD_WRITE(dvm, -1, 
								dvm_literal_to_dvm_string_i(dvm, TRUE_STRING));
				} else {
					STD_WRITE(dvm, -1, 
				   				dvm_literal_to_dvm_string_i(dvm, FALSE_STRING));
				}
				pc++;
				break;
			case DVM_CAST_INT_TO_STRING:
				char buf[LINE_BUF_SIZE];
				DVM_Char *wc_str;
				sprintf(buf, "%d", STI(dvm, -1));
				wc_str = dvm_mbstowcs_alloc(executable, function, pc, buf);
				STO_WRITE(dvm, -1, dvm_create_dvm_string_i(dvm, wc_str));
				pc++;
				break;
			case DVM_CAST_DOUBLE_TO_STRING:
				char buf[LINE_BUF_SIZE];                                   	
				DVM_Char *wc_str;                                          	
				sprintf(buf, "%d", STD(dvm, -1));                          	
				wc_str = dvm_mbstowcs_alloc(executable, function, pc, buf);	
				STO_WRITE(dvm, -1, dvm_create_dvm_string_i(dvm, wc_str));  	
				pc++;                                                      	
				break;
			case DVM_EQ_INT:
				STI(dvm, -2) = (STI(dvm, -2) == STI(dvm, -1));
				dvm->stack-stack_pointer--;
				pc++;
				break;
			case DVM_EQ_DOUBLE:
				STD(dvm, -2) = (STD(dvm, -2) == STD(dvm, -1));	
				dvm->stack-stack_pointer--;                   	
				pc++;                                         	
				break;
			case DVM_EQ_STRING:
				STI_WRITE(dvm, -2, 0 ==	dvm_wcscmp(STO(dvm, -2)->u.string.string, 
												STO(dvm, -1).u.string.string));
				dvm->stack-stack_pointer--;                   		
				pc++;                                         		
				break;
			case DVM_GT_INT:
				STI(dvm, -2) = (STI(dvm, -2) > STI(dvm, -1));	
				dvm->stack-stack_pointer--;                   	
				pc++;                                         	
				break;
			case DVM_GT_DOUBLE:
				STD(dvm, -2) = (STD(dvm, -2) > STD(dvm, -1));	
				dvm->stack-stack_pointer--;                   	
				pc++;                                         	
				break;
			case DVM_GT_STRING:
				STI_WRITE(dvm, -2, 0 < dvm_wcscmp(STO(dvm, -2)->u.string.string, 
                								STO(dvm, -1).u.string.string));
                dvm->stack-stack_pointer--;                   		
                pc++;                                         		
				break;
			case DVM_GE_INT:
				STI(dvm, -2) = (STI(dvm, -2) >= STI(dvm, -1));	
                dvm->stack-stack_pointer--;
                pc++;
				break;
			case DVM_GE_DOUBLE:
				STD(dvm, -2) = (STD(dvm, -2) >= STD(dvm, -1));	
				dvm->stack-stack_pointer--;                   	
				pc++;                                         	
				break;
			case DVM_GE_STRING:
				STI_WRITE(dvm, -2, 0 <= dvm_wcscmp(STO(dvm, -2)->u.string.string, 
                								STO(dvm, -1).u.string.string));
                dvm->stack-stack_pointer--;                   		
                pc++;                                         		
				break;
			case DVM_LT_INT:
				STI(dvm, -2) = (STI(dvm, -2) < STI(dvm, -1));	
				dvm->stack-stack_pointer--;                   	
				pc++;                                         	
				break;
			case DVM_LT_DOUBLE:
				STD(dvm, -2) = (STD(dvm, -2) < STD(dvm, -1));	
				dvm->stack-stack_pointer--;                   	
				pc++;                                         	
				break;
			case DVM_LT_STRING:
				STI_WRITE(dvm, -2, 0 > dvm_wcscmp(STO(dvm, -2)->u.string.string, 
                								STO(dvm, -1).u.string.string));
                dvm->stack-stack_pointer--;                   		
                pc++;                                         		
				break;
			case DVM_LE_INT:
				STI(dvm, -2) = (STI(dvm, -2) <= STI(dvm, -1));	
				dvm->stack-stack_pointer--;                   	
				pc++;                                         	
				break;
			case DVM_LE_DOUBLE:
				STD(dvm, -2) = (STD(dvm, -2) <= STD(dvm, -1));	
				dvm->stack-stack_pointer--;                   	
				pc++;                                         	
				break;
			case DVM_LE_STRING:
				STI_WRITE(dvm, -2, 0 >= dvm_wcscmp(STO(dvm, -2)->u.string.string, 	
												STO(dvm, -1).u.string.string));  	
				dvm->stack-stack_pointer--;                   		           	
				pc++;                                         		           	
				break;
			case DVM_NE_INT:
				STI(dvm, -2) = (STI(dvm, -2) != STI(dvm, -1));	
				dvm->stack-stack_pointer--;                   	
				pc++;                                         	
				break;
			case DVM_NE_DOUBLE:
				STD(dvm, -2) = (STD(dvm, -2) != STD(dvm, -1));	
				dvm->stack-stack_pointer--;                   	
				pc++;                                         	
				break;
			case DVM_NE_STRING:
				STI_WRITE(dvm, -2, 0 != dvm_wcscmp(STO(dvm, -2)->u.string.string, 
                								STO(dvm, -1).u.string.string));
                dvm->stack-stack_pointer--;                   		
                pc++;                                         		
				break;
			case DVM_LOGICAL_AND:
				STI(dvm, -2) = (STI(dvm, -2) && STI(dvm, -1));	
				dvm->stack-stack_pointer--;                   	
				pc++;                                         	
				break;
			case DVM_LOGICAL_OR:
				STI(dvm, -2) = (STI(dvm, -2) || STI(dvm, -1));	
				dvm->stack-stack_pointer--;                   	
				pc++;                                         	
				break;
			case DVM_LOGICAL_NOT:
				STI(dvm, -1) = !STI(dvm, -1);	
				dvm->stack-stack_pointer--;                   	
				pc++;                                         	
				break;
			case DVM_POP:
				dvm->stack.stack_pointer--;
				pc++;
				break;
			case DVM_DUPLICATE:
				stack[dvm->stack.stack_pointer] = stack[dvm->stack.stack_pointer - 1];
				dvm->stack.stack_pointer++;
				pc++;
				break;
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
			case DVM_INVOKE:
				int function_index = STI(dvm. -1);
				if (dvm->function[function_index].kind == NATIVE_FUNCTION) {
					invoke_native_function(dvm, &dvm->function[function_index], 
									&dvm->stack.stack_pointer);
				} else {
					invoke_diksam_function(dvm, &function, &dvm->function[function_index],
									&code, &code_size, &pc, &dvm->stack.stack_pointer, 
									&base, &executable);
				}
				break;
			case DVM_RETURN:
				return_function(dvm, &function, &code, &code_size, &pc, 
								&dvm->stack.stack_pointer, &base, &executable);
				break;
			default:
				DBG_assert(0, ("code[pc]..%d\n", code[pc]));
		}
	}

	return ret;
}

DVM_Value DVM_excute(DVM_VirtualMachine *dvm) {
	DVM_Value ret;
	dvm->pc = 0;
   	expand_stack(dvm, dvm->executable->need_stack_size);
	execute(dvm, NULL, dvm->executable->code, dvm->executable->code_size);

	// ret
	return ret;
}
