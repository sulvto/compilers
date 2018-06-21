//
// Created by sulvto on 18-6-9.
//

#include <string.h>
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

char *dkc_strdup(char *src) {
	DKC_Compiler *compiler = dkc_get_current_compiler();

	char *p = MEM_storage_malloc(compiler->compile_storage, strlen(src) + 1);

	strcpy(p, src);

	return p;
}

static FunctionDefinition *search_renamed_function(DKC_Compiler *compiler, RenameList *rename) {
	for (CompilerList *pos = compiler->required_list; pos; pos = pos->next) {
		if (!dkc_compare_package_name(rename->package_name, pos->compiler->package_name)) {
			continue;
		}

		for (FunctionDefinition *fun_def_pos = pos->compiler->function_list; fun_def_pos; fun_def_pos = fun_def_pos->next) {
			if (strcmp(fun_def_pos->name, rename->original_name)==0 && fun_def_pos->class_definition == NULL) {
				return fun_def_pos;
			}
		}
	}

	return NULL;
}

FunctionDefinition *dkc_search_function(char *name) {
	DKC_Compiler *compiler = dkc_get_current_compiler();

	for (FunctionDefinition *pos = compiler->function_list; pos; pos = pos->next) {
        if (strcmp(pos->name, name) == 0 && pos->class_definition == NULL) {
            return pos;
        }
    }

	for (RenameList *pos = compiler->rename_list; pos; pos = pos->next) {
		if (strcmp(pos->renamed_name, name) == 0) {
			FunctionDefinition *fun_def = search_renamed_function(compiler, pos);
			if (fun_def) {
				return fun_def;
			}
		}
	}

	for (CompilerList *pos = compiler->required_list; pos; pos = pos->next) {
		for (FunctionDefinition *fun_def_pos = pos->compiler->function_list; fun_def_pos; fun_def_pos = fun_def_pos->next) {
			if (strcmp(fun_def_pos->name, name) == 0 && fun_def_pos->class_definition == NULL) {
				return fun_def_pos;
			}
		}
	}

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
	// TODO
    return NULL;
}

void dkc_vstr_clear(VString *v) {
	v->string = NULL;
}

static int my_strlen(char *string) {
	if (string == NULL) {
		return 0;
	}

	return strlen(string);
}

void dkc_vstr_append_string(VString *v, char *string) {
	int old_len = my_strlen(v->string);
	int new_size = old_len + strlen(string) + 1;
	v->string = MEM_realloc(v->string, new_size);
	strcpy(&v->string[old_len], string);
}

void dkc_vstr_append_character(VString *v, int ch) {
	int current_len = my_strlen(v->string);
	v->string = MEM_realloc(v->string, current_len + 2);
	v->string[current_len] = ch;
	v->string[current_len + 1] = '\0';
}

void dkc_vwstr_clear(VWString *v) {
	v->string = NULL;
}

static int my_wcslen(DVM_Char *string) {
	if (string == NULL) {
		return 0;
	}

	return dvm_wcslen(string);
}

void dkc_vwstr_append_string(VWString *v, DVM_Char *string) {
	int old_len = my_wcslen(v->string);
	int new_size = sizeof(DVM_Char) * (old_len + dvm_wcslen(string) + 1);
	v->string = MEM_realloc(v->string, new_size);
	dvm_wcscpy(&v->string[old_len], string);
}

void dkc_vwstr_append_character(VWString *v, int ch) {
	int current_len = my_wcslen(v->string);
	v->string = MEM_realloc(v->string, sizeof(DVM_Char)
			*(current_len + 2));

	v->string[current_len] = ch;
	v->string[current_len + 1] = '\0';
}

char *dkc_get_type_name(TypeSpecifier *type) {
	printf("dkc_get_type_name\n");

}

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
            DBG_assert(0, ("bad case. type..%d\n", type));
    }

    return NULL;
}

DVM_Char *dkc_expression_to_string(Expression *expression) {
printf("dkc_expression_to_string\n");
}

char *dkc_package_name_to_string(PackageName *package_name) {
	if (package_name == NULL) {
		return NULL;
	}

	PackageName *pos;

	int len = 0;
	for (pos = package_name; pos; pos = pos->next) {
		len += strlen(pos->name) + 1;
	}

	char *dest = MEM_malloc(len);
	dest[0] = '\0';

	for (pos = package_name; pos; pos = pos->next) {
		strcat(dest, pos->name);
		if (pos->next) {
			strcat(dest, ".");
		}
	}

	return dest;
}
