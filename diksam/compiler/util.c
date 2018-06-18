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

void dkc_strdup(char *src) {
    // TODO
}

FunctionDefinition *dkc_search_function(char *name) {
	DKC_Compiler *compiler = dkc_get_current_compiler();

	for (FunctionDefinition *pos = compiler->function_list; pos->next; pos = pos->next) {
        if (strcmp(pos->name, name) == 0 && pos->class_definition == NULL) {
            return pos;

        }
    }


    // TODO
	return NULL;
}

Declaration *dkc_search_declaration(char *name, Block *block) {
	Block *block_pos;
	DeclarationList *declaration_pos;

	for (block_pos = block; block_pos; block_pos = block_pos->outer_block) {
		for (declaration_pos = block_pos->declaration_list;
		     declaration_pos;
		     declaration_pos = declaration_pos->next) {
			if (strcmp(declaration_pos->declaration->name, name) == 0) {
				return declaration_pos->declaration;
			}
		}
	}

	DKC_Compiler *compiler = dkc_get_current_compiler();
	for (declaration_pos = compiler->declaration_list;
	     declaration_pos;
	     declaration_pos = declaration_pos->next) {
		if (strcmp(declaration_pos->declaration->name, name) == 0) {
			return declaration_pos->declaration;
		}
	}

	return NULL;
}

TypeSpecifier *dkc_alloc_type_specifier(DVM_BasicType type) {
	TypeSpecifier *type_specifier =  dkc_malloc(sizeof(TypeSpecifier));
	type_specifier->basic_type = type;
	type_specifier->derive = NULL;

	if (type == DVM_CLASS_TYPE) {
		type_specifier->class_ref.identifier = NULL;
		type_specifier->class_ref.class_definition = NULL;
	}

	return type_specifier;
}


TypeDerive *dkc_alloc_type_derive(DeriveTag derive_tag) {
    TypeDerive *derive = dkc_malloc(sizeof(TypeDerive));
    derive->tag = derive_tag;
    derive->next = NULL;

    return derive;
}

TypeSpecifier *dkc_alloc_type_specifier2(TypeSpecifier *src) {
    TypeSpecifier *result = dkc_malloc(sizeof(TypeSpecifier));
    *result = *src;

    return result;
}

DVM_Boolean dkc_compare_parameter(ParameterList *parameter1, ParameterList *parameter2) {
    ParameterList *pos1;
    ParameterList *pos2;

    for (pos1 = parameter1, pos2 = parameter2;
         pos1 && pos2;
         pos1 = pos1->next, pos2 = pos2->next) {
        if (strcmp(pos1->name, pos2->name) != 0) {
            return DVM_FALSE;
        }

        if (!dkc_compare_type(pos1->type, pos2->type)) {
            return DVM_FALSE;
        }
    }

    if (pos1 || pos2) {
        return DVM_FALSE;
    }

    return DVM_TRUE;
}

DVM_Boolean dkc_compare_type(TypeSpecifier *type1, TypeSpecifier *type2) {
    // TODO
    return DVM_FALSE;
}

DVM_Boolean dkc_compare_package_name(PackageName *package1, PackageName *package2) {

}

ClassDefinition *dkc_search_class(char *identifier) {

}

MemberDeclaration *dkc_search_member(ClassDefinition *class_definition, char *member_name) {
    DKC_Compiler *compiler = dkc_get_current_compiler();
    return NULL;
}

void dkc_vstr_clear(VString *v);

void dkc_vstr_append_string(VString *v, char *string);

void dkc_vstr_append_character(VString *v, int ch);

void dkc_vwstr_clear(VString *v);

void dkc_vwstr_append_string(VWString *v, char *string);

void dkc_vwstr_append_character(VWString *v, int ch);

char *dkc_get_type_name(TypeSpecifier *type);

char *dkc_get_basic_type_name(DVM_BasicType type) {
    switch (type) {
        case DVM_VOID_TYPE:     return "void";
        case DVM_BOOLEAN_TYPE:  return "boolean";
        case DVM_INT_TYPE:      return "int";
        case DVM_DOUBLE_TYPE:   return "double";
        case DVM_STRING_TYPE:   return "string";
        case DVM_CLASS_TYPE:    return "class";
        case DVM_NULL_TYPE:     return "null";
        case DVM_BASE_TYPE:
        default:
            DGB_assert(0, ("bad case. type..%d\n", type));
    }

    return NULL;
}

DVM_Char *dkc_expression_to_string(Expression *expression);

char *dkc_package_name_to_string(PackageName *package_name) {

}
