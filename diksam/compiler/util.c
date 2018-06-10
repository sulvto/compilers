//
// Created by sulvto on 18-6-9.
//

#include "MEM.h"
#include "DBG.h"
#include "diksamc.h"

static DKC_Compiler *st_current_compiler;

DKC_Compiler *dkc_get_current_compiler(void) {
	return st_current_compiler;
}

void dkc_set_current_compiler(DKC_Compiler *compiler) {
	st_current_compiler = compiler;
}

void *dkc_malloc(size_t size) {
	DKC_Compiler *compiler = dkc_get_current_compiler();
	void *p = MEM_storage_malloc(compiler->compile_storage, size);
	return p;
}

FunctionDefinition *dkc_search_function(char *name) {
	DKC_Compiler *compiler = dkc_get_current_compiler();
	FunctionDefinition *pos;
	for (pos = compiler->function_list; pos->next; pos = pos->next) {
		if (strcmp(pos->name, name) == 0) {
			break;
		}
	}
	return pos;
}

Declaration *dkc_search_declaration(char *name, Block *block) {
	Block *block_pos;
	Declaration *decl_pos;

	for (block_pos = block; block_pos; block_pos = block_pos->next) {
		for (decl_pos = block_pos->declaration_list; decl_pos; decl_pos = decl_pos->next) {
			if (strcmp(decl_pos->declaration->name, name) == 0) {
				return decl_pos->declaration;
			}
		}
	}

	DKC_Compiler *compiler = dkc_get_current_compiler();
	for (decl_pos = compiler->declaration_list; decl_pos; decl_pos = decl_pos->next) {
		if (strcmp(decl_pos->declaration->name, name) == 0) {
			return decl_pos->declaration;
		}
	}

	return NULL;
}

TypeSpecifier *dkc_alloc_type_specifier(DVM_BasicType type) {
	TypeSpecifier *type_specifier =  dkc_malloc(sizeof(TypeSpecifier));
	type_specifier->type = type;
	type_specifier->derive = NULL;

	return type;
}
