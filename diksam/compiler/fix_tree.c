//
// Created by sulvto on 18-6-15.
//

#include "MEM.h"
#include "diksamc.h"

static int reserve_function_index(DKC_Compiler *compiler, FunctionDefinition *src) {
	if (src->class_definition && src->block == NULL_EXPRESSION) {
		return ABSTRACT_METHOD_INDEX;
	}

	char *src_package_name = dkc_package_name_to_string(src->package_name);
	for (int i = 0; i < compiler->dvm_function_count; i++) {
		if (dvm_compare_package_name(src_package_name, compiler->dvm_function[i].package_name)
		    && 0 == strcmp(src->name, compiler->dvm_function[i])) {
			MEM_free(src_package_name);
			return i;
		}
	}

	compiler->dvm_function = MEM_realloc(compiler->dvm_function,
	                                     sizeof(DVM_Function) * (compiler->dvm_function_count + 1));
	DVM_Function *dest = &compiler->dvm_function[compiler->dvm_function_count];
	compiler->dvm_function_count++;
	dest->package_name = src_package_name;
	if (src->class_definition) {
		dest->name = dvm_create_method_function_name(src->class_definition->name, src->name);
	} else {
		dest->name = MEM_strdup(src->name);
	}

	return compiler->dvm_function_count - 1;
}

static void fix_class_list(DKC_Compiler *compiler) {
	// TODO
}

static void fix_function(DKC_Compiler *compiler, FunctionDefinition *function) {
	// TODO
}

static void fix_statement_list(Block *current_block, StatementList *list, FunctionDefinition *function_definition){
	// TODO
}

void dkc_fix_tree(DKC_Compiler *compiler) {
    fix_class_list(compiler);
    int variable_count = 0;

    for (FunctionDefinition *pos = compiler->function_list; pos; pos = pos->next) {
        reserve_function_index(compiler, pos);
    }

    fix_statement_list(NULL, compiler->statement_list, NULL);

    for (FunctionDefinition *pos = compiler->function_list; pos; pos = pos->next) {
        if (pos->class_definition == NULL) {
            fix_function(compiler, pos);
        }
    }

    for (DeclarationList *decl = compiler->declaration_list; decl; decl = decl->next) {
        decl->declaration->variable_index = variable_count;
        variable_count++;
    }
}
