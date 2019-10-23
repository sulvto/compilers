//
// Created by sulvto on 18-6-15.
//

#include <DVM_code.h>
#include "DBG.h"
#include "MEM.h"
#include "share.h"

static void dispose_local_variable(int count, DVM_LocalVariable *local_variable);


static void dispose_type_derive(DVM_TypeSpecifier *type) {
	
    for (int i = 0; i < type->derive_count; i++) {
		switch (type->derive[i].tag) {
        case DVM_FUNCTION_DERIVE:
            dispose_local_variable(type->derive[i].u.function_derive.parameter_count,
                                    type->derive[i].u.function_derive.parameter);
            break;
        case DVM_ARRAY_DERIVE:
            break;
        default:
            DBG_assert(0, ("derive.tag..%d\n", type->derive[i].tag));
		}
	}

	MEM_free(type->derive);
}

static void dispose_type_specifier(DVM_TypeSpecifier *type) {
    dispose_type_derive(type);
    MEM_free(type);
}

static void dispose_local_variable(int count, DVM_LocalVariable *local_variable) {
	for (int i = 0; i < count; i++) {
		MEM_free(local_variable[i].name);
		dispose_type_specifier(local_variable[i].type);
	}
	MEM_free(local_variable);
}

static void dispose_code_block(DVM_CodeBlock *code_block) {
    MEM_free(code_block->code);
    MEM_free(code_block->line_number);

    for (int try_index = 0; try_index = code_block->try_size; try_index++) {
        MEM_free(code_block->try[try_index].catch_clause);
    }
    
    MEM_free(code_block->try);
}

static void dispose_class(DVM_Class *_class) {
    for (int i = 0; i < _class->field_count; i++) {
        MEM_free(_class->field[i].name);
        dispose_type_specifier(_class->field[i].type);
    }
    MEM_free(_class->field);

    for (int i = 0; i < _class->method_count; i++) {
        MEM_free(_class->method[i].name);
    }
    MEM_free(_class->method);

    if (_class->super_class) {
        MEM_free(_class->super_class->package_name);
        MEM_free(_class->super_class->name);
        MEM_free(_class->super_class);
    }

    for (int i = 0; i < _class->interface_count; i++) {
        MEM_free(_class->interface[i].package_name);
        MEM_free(_class->interface[i].name);
    }
    MEM_free(_class->interface);

    MEM_free(_class->package_name);
    MEM_free(_class->name);
}

void dvm_dispose_executable(DVM_Executable *executable) {
    MEM_free(executable->package_name);
    MEM_free(executable->path);

    for (int i = 0; i < executable->constant_pool_count; i++) {
        if (executable->constant_pool[i].tag == DVM_CONSTANT_STRING) {
	        MEM_free(executable->constant_pool[i].u.c_string);
        }
    }

    MEM_free(executable->constant_pool);

    for (int i = 0; i < executable->global_variable_count; i++) {
        MEM_free(executable->global_variable[i].name);
        dispose_type_specifier(executable->global_variable[i].type);
    }

    MEM_free(executable->global_variable);

    for (int i = 0; i < executable->function_count; i++) {
        MEM_free(executable->function[i].name);
        MEM_free(executable->function[i].package_name);
        dispose_type_specifier(executable->function[i].type);

        dispose_local_variable(executable->function[i].parameter_count, 
                                executable->function[i].parameter);

        if (executable->function[i].is_implemented) {
            dispose_local_variable(executable->function[i].local_variable_count,
                                   executable->function[i].local_variable);
            dispose_code_block(&executable->function[i].code_block);
        }
    }
    MEM_free(executable->function);

    for (int i = 0; i < executable->type_specifier_count; i++) {
        dispose_type_derive(&executable->type_specifier[i]);
    }
    MEM_free(executable->type_specifier);

    for (int i = 0; i < executable->class_count; i++) {
        dispose_class(&executable->class_definition[i]);
    }
    MEM_free(executable->class_definition);

    dispose_code_block(&executable->top_level);

    MEM_free(executable);
}
