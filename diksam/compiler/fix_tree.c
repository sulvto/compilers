//
// Created by sulvto on 18-6-15.
//

#include "MEM.h"
#include "diksamc.h"

static void fix_function(DKC_Compiler *compiler, FunctionDefinition *function) {
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
