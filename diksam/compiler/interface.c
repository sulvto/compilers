//
// Created by sulvto on 18-6-6.
//
#include <string.h>
#include <stdlib.h>
#include "DBG.h"
#include "MEM.h"
#include "DKC.h"
#include "diksamc.h"

static CompilerList *st_compiler_list = NULL;

typedef struct {
    char            *name;
    DVM_BasicType   type;
} BuiltInMethodParameter;

typedef struct {
    char            *name;
    DVM_BasicType   return_type;
    BuiltInMethodParameter *parameter;
    int             parameter_count;
} BuiltInMethod;

static BuiltInMethodParameter st_array_resize_arg[] = {
        {"new_size", DVM_INT_TYPE}
};

static BuiltInMethodParameter st_array_insert_arg[] = {
        {"index",    DVM_INT_TYPE},
        {"new_item", DVM_BASE_TYPE}
};

static BuiltInMethodParameter st_array_remove_arg[] = {
        {"index", DVM_INT_TYPE}
};

static BuiltInMethodParameter st_array_add_arg[] = {
        {"new_item", DVM_BASE_TYPE}
};

static BuiltInMethod st_string_method[] = {
        {ARRAY_METHOD_SIZE,   DVM_INT_TYPE,  NULL,                0},
        {ARRAY_METHOD_RESIZE, DVM_VOID_TYPE, st_array_resize_arg, ARRAY_SIZE(st_array_resize_arg)},
        {ARRAY_METHOD_INSERT, DVM_VOID_TYPE, st_array_insert_arg, ARRAY_SIZE(st_array_insert_arg)},
        {ARRAY_METHOD_REMOVE, DVM_VOID_TYPE, st_array_remove_arg, ARRAY_SIZE(st_array_remove_arg)},
        {ARRAY_METHOD_ADD,    DVM_VOID_TYPE, st_array_add_arg,    ARRAY_SIZE(st_array_add_arg)},
};

static BuiltInMethodParameter st_string_substr_arg[] = {
        {"start",  DVM_INT_TYPE},
        {"length", DVM_INT_TYPE}
};

static BuiltInMethod st_array_method[] = {
        {"length", DVM_INT_TYPE,    NULL,                 0},
        {"substr", DVM_STRING_TYPE, st_string_substr_arg, ARRAY_SIZE(st_string_substr_arg)}
};


static CompilerList * traversal_compiler(CompilerList *list, DKC_Compiler *compiler);

static FunctionDefinition *create_built_in_method(BuiltInMethod *src, int method_count) {
    FunctionDefinition *function_definition_array = dkc_malloc(sizeof(FunctionDefinition) * method_count);

    for (int i = 0; i < method_count; i++) {
        function_definition_array[i].name = src[i].name;
        function_definition_array[i].type = dkc_alloc_type_specifier(src[i].return_type);
        ParameterList *parameter_list = NULL;
        for (int param_index = 0; param_index < src[i].parameter_count; param_index++) {
            TypeSpecifier *type = dkc_alloc_type_specifier(src[i].parameter[param_index].type);
            if (parameter_list) {
                parameter_list = dkc_chain_parameter(parameter_list, type, src[i].parameter[param_index].name);
            } else {
                parameter_list = dkc_create_parameter(type, src[i].parameter[param_index].name);
            }
        }
        function_definition_array[i].parameter = parameter_list;
    }

    return function_definition_array;
}

DKC_Compiler *DKC_create_compiler(void) {
    DKC_Compiler *compiler_backup = dkc_get_current_compiler();
	MEM_Storage storage = MEM_open_storage(0);
	DKC_Compiler *compiler = MEM_storage_malloc(storage,
                                                sizeof(struct DKC_Compiler_tag));
	dkc_set_current_compiler(compiler);

	compiler->compile_storage = storage;
    compiler->package_name = NULL;
    compiler->source_suffix = DKM_SOURCE;
    compiler->require_list = NULL;
    compiler->rename_list = NULL;
    compiler->function_list = NULL;
    compiler->dvm_function_count = 0;
    compiler->dvm_function = NULL;
    compiler->dvm_class_count = 0;
    compiler->dvm_class = NULL;
    compiler->declaration_list = NULL;
    compiler->statement_list = NULL;
    compiler->class_definition_list = NULL;
    compiler->current_block = NULL;
    compiler->current_line_number = 1;
    compiler->current_class_definition = NULL;
    compiler->input_mode = DKC_FILE_INPUT_MODE;
    compiler->required_list = NULL;
    compiler->array_method_count = ARRAY_SIZE(st_array_method);
    compiler->array_method = create_built_in_method(st_array_method, ARRAY_SIZE(st_array_method));
    compiler->string_method_count = ARRAY_SIZE(st_string_method);
    compiler->string_method = create_built_in_method(st_string_method, ARRAY_SIZE(st_string_method));


    compiler->source_encoding = UTF_8_ENCODING;

    dkc_set_current_compiler(compiler_backup);

    return compiler;
}

static CompilerList *add_compiler_to_list(CompilerList *list, DKC_Compiler *compiler) {
    CompilerList *new_item = MEM_malloc(sizeof(CompilerList));
    new_item->compiler = compiler;
    new_item->next = NULL;

    if (list == NULL) {
        return new_item;
    }

    CompilerList *pos;
    for (pos = list; pos->next; pos = pos->next);

    pos->next = new_item;

    return list;
}

static void set_path_to_compiler(DKC_Compiler *compiler, char *path) {
    compiler->path = MEM_storage_malloc(compiler->compile_storage, strlen(path) + 1);

    strcpy(compiler->path, path);
}

static DKC_Compiler *search_compiler(CompilerList *list, PackageName *package_name) {
	for (CompilerList *pos = list; pos; pos = pos->next) {
		if (dkc_compare_package_name(pos->compiler->package_name, package_name)) {
			return pos->compiler;
		}
	}

	return NULL;
}

static void make_search_path(int line_number, PackageName *package_name, char *buf) {
	PackageName *pos;
	int len = 0;
	int prev_len = 0;
	int suffix_len = strlen(DIKSAM_REQUIRE_SUFFIX);
	buf[0] = '\0';
	for (pos = package_name; pos; pos = pos->next) {
		prev_len = len;
		len += strlen(pos->name);
		if (len > FILENAME_MAX - (2 + suffix_len)) {
			dkc_compile_error(line_number, PACKAGE_NAME_TOO_LONG_ERR, MESSAGE_ARGUMENT_END);
		}
		strcpy(&buf[prev_len], pos->name);
		if (pos->next) {
			buf[strlen(buf)] = FILE_SEPARATOR;
			len++;
		}
	}

	strcpy(&buf[len], DIKSAM_REQUIRE_SUFFIX);
}

static void get_require_input(RequireList *require, char *path, SourceInput *source_input) {
    char *search_path = getenv("DKM_REQUIRE_SEARCH_PATH");
    if (search_path == NULL) {
        search_path = ".";
    }

    char search_file[FILENAME_MAX];

    make_search_path(require->line_number, require->package_name, search_file);

    SearchFileStatus status = dvm_search_file(search_path, search_file, path);
    if (status != SEARCH_FILE_SUCCESS) {
        if (status == SEARCH_FILE_NOT_FOUND) {
            dkc_compile_error(require->line_number, REQUIRE_FILE_NOT_FOUND_ERR,
                               STRING_MESSAGE_ARGUMENT, "file", search_file,
                               MESSAGE_ARGUMENT_END);
        } else {
            dkc_compile_error(require->line_number, REQUIRE_FILE_NOT_FOUND_ERR,
                               STRING_MESSAGE_ARGUMENT, "status", (int) status,
                               MESSAGE_ARGUMENT_END);
        }
    }

    source_input->input_mode = DKC_FILE_INPUT_MODE;
    source_input->u.file.fp = fopen(path, "r");
}

static DVM_Boolean add_executable_to_list(DVM_Executable *executable, DVM_ExecutableList *list) {
    DVM_ExecutableItem *tail;

    for (DVM_ExecutableItem *pos = list->list; pos; pos = pos->next) {
        if (dvm_compare_package_name(pos->executable->package_name, executable->package_name)
                && pos->executable->is_required == executable->is_required) {
            return DVM_FALSE;
        }
        tail = pos;
    }

    DVM_ExecutableItem *item = MEM_malloc(sizeof(DVM_ExecutableItem));
    item->executable = executable;
    item->next = NULL;

    if (list->list == NULL) {
        list->list = item;
    } else {
        tail->next = item;
    }

    return DVM_TRUE;
}

static DVM_Executable *do_compile(DKC_Compiler *compiler, DVM_ExecutableList *executable_list, char *path, DVM_Boolean is_required) {
	printf("do_compile path %s\n", compiler->path);
	extern FILE *yyin;
	extern int yyparse(void);

	DVM_Executable *executable;
	char found_path[FILENAME_MAX];
	SourceInput source_input;

	DKC_Compiler *compiler_backup = dkc_get_current_compiler();

	dkc_set_current_compiler(compiler);

	if (yyparse()) {
		fprintf(stderr, "Error!\n");
		exit(1);
	}

	for (RequireList *req_pos = compiler->require_list; req_pos; req_pos = req_pos->next) {
		DKC_Compiler *req_compiler = search_compiler(st_compiler_list, req_pos->package_name);
		if (req_compiler) {
			compiler->required_list = add_compiler_to_list(compiler->required_list, req_compiler);
			continue;
		}

		req_compiler = DKC_create_compiler();
		req_compiler->package_name = req_pos->package_name;
		req_compiler->source_suffix = DKH_SOURCE;
		compiler->required_list = add_compiler_to_list(compiler->required_list, req_compiler);
		st_compiler_list = add_compiler_to_list(st_compiler_list, req_compiler);

		get_require_input(req_pos, found_path, &source_input);
		set_path_to_compiler(req_compiler, found_path);

		req_compiler->input_mode = source_input.input_mode;

		if (source_input.input_mode == DKC_FILE_INPUT_MODE) {
			yyin = source_input.u.file.fp;
		} else {
			dkc_set_source_string(source_input.u.string.lines);
		}

		do_compile(req_compiler, executable_list, path, DVM_TRUE);
	}

	printf("dkc_fix_tree\n");
	dkc_fix_tree(compiler);

	executable = dkc_generate(compiler);

	if (path) {
		executable->path = MEM_strdup(path);
	} else {
		executable->path = NULL;
	}

	printf("dvm_disassemble\n");

	dvm_disassemble(executable);

	executable->is_required = is_required;

	if (!add_executable_to_list(executable, executable_list)) {
		dvm_dispose_executable(executable);
	}

	dkc_set_current_compiler(compiler_backup);

	return executable;
}


static void dispose_compiler_list(void) {
    while (st_compiler_list) {
        CompilerList *temp = st_compiler_list;
        st_compiler_list = st_compiler_list->next;
        MEM_free(temp);
    }
}

static void make_search_path_impl(char *package_name, char *buf) {
	int suffix_len = strlen(DIKSAM_IMPLEMENTATION_SUFFIX);
	int package_len = strlen(package_name);

	DBG_assert(package_len <= FILENAME_MAX - (2 + suffix_len), ("package name is too long(%s)", package_name));

	int i;
	for (i = 0; package_name[i] != '\0'; i++) {
		if (package_name[i] == '.') {
			buf[i] = FILE_SEPARATOR;
		} else {
			buf[i] = package_name[i];
		}
	}

	buf[i] = '\0';

	strcat(buf, DIKSAM_IMPLEMENTATION_SUFFIX);
}

static SearchFileStatus get_dynamic_load_input(char *package_name, char *found_path, char *search_file, SourceInput *source_input) {
	char *search_path = getenv("DKC_LOAD_SEARCH_PATH");
	if (search_path == NULL) {
		search_path = ".";
	}

	make_search_path_impl(package_name, search_file);
	SearchFileStatus status = dvm_search_file(search_path, search_file, found_path);
	if (status != SEARCH_FILE_SUCCESS) {
		return status;
	}

	source_input->input_mode = DKC_FILE_INPUT_MODE;
	source_input->u.file.fp = fopen(found_path, "r");

	return SEARCH_FILE_SUCCESS;
}

static PackageName *create_ont_package_name(DKC_Compiler *compiler, char *string, int start_index, int to_index) {
	MEM_Storage storage = compiler->compile_storage;
	PackageName *package_name = MEM_storage_malloc(storage, sizeof(PackageName));
	package_name->name = MEM_storage_malloc(storage, to_index - start_index + 1);

	int i;
	for (i = 0; i < to_index - start_index; i++) {
		package_name->name[i] = string[start_index + i];
	}

	package_name->name[i] = '\0';
	package_name->next = NULL;

	return package_name;
}

static PackageName *string_to_package_name(DKC_Compiler *compiler, char *string) {
	int start_index = 0;
	PackageName *package_name;
	PackageName *top;
	PackageName *tail = NULL;

	int i;
	for (i = 0; string[i] != '\0'; i++) {
		if (string[i] == '.') {
			package_name = create_ont_package_name(compiler, string, start_index, i);
			start_index = i + 1;
			if (tail) {
				tail->next = package_name;
			} else {
				top = package_name;
			}
			tail = package_name;
		}
	}

	package_name = create_ont_package_name(compiler, string, start_index, i);

	if (tail) {
		tail->next = package_name;
	} else {
		top = package_name;
	}

	return top;
}

SearchFileStatus dkc_dynamic_compile(DKC_Compiler *compiler, char *package_name, DVM_ExecutableList *list,
                                     DVM_ExecutableItem **add_top, char *search_file) {

	extern FILE *yyin;
	SourceInput source_input;
	char found_path[FILENAME_MAX];

	SearchFileStatus status = get_dynamic_load_input(package_name, found_path, search_file, &source_input);
	if (status != SEARCH_FILE_SUCCESS) {
		return status;
	}

	DBG_assert(st_compiler_list == NULL, ("st_compiler_list != NULL(%p)", st_compiler_list));


	DVM_ExecutableItem *tail;
	for (tail = list->list; tail->next; tail = tail->next);

	compiler->package_name = string_to_package_name(compiler, package_name);

	set_path_to_compiler(compiler, found_path);

	compiler->input_mode = source_input.input_mode;
	if (source_input.input_mode == DKC_FILE_INPUT_MODE) {
		yyin = source_input.u.file.fp;
	} else {
		dkc_set_source_string(source_input.u.string.lines);
	}

	do_compile(compiler, list, found_path, DVM_FALSE);

	dispose_compiler_list();
	dkc_reset_string_literal_buffer();

	*add_top = tail->next;


	return SEARCH_FILE_SUCCESS;
}

DVM_ExecutableList *DKC_compile(DKC_Compiler *compiler, FILE *fp, char *path) {
    extern FILE *yyin;
    DVM_ExecutableList *executable_list;

    DBG_assert(st_compiler_list == NULL, (" st_compiler_list != NULL(%p)", st_compiler_list));

    set_path_to_compiler(compiler, path);
    compiler->input_mode = DKC_FILE_INPUT_MODE;

    yyin = fp;

    executable_list = MEM_malloc(sizeof(DVM_ExecutableList));
    executable_list->list = NULL;

	DVM_Executable *executable = do_compile(compiler, executable_list, NULL, DVM_FALSE);
	executable->path = MEM_strdup(path);
	executable_list->top_level = executable;

    dispose_compiler_list();
	dkc_reset_string_literal_buffer();

    return executable_list;
}

DVM_ExecutableList *DKC_compile_string(DKC_Compiler *compiler, char **lines) {
//    extern int yyparse(void);
//    DVM_Executable *executable;
//
//    dkc_set_source_string(lines);
//    compiler->current_line_number = 1;
//    compiler->input_mode = DKC_STRING_INPUT_MODE;
//
//    executable = do_compile(compiler);
//
//    dkc_reset_string_literal_buffer();
//
//    return executable;
}

CompilerList static *traversal_compiler(CompilerList *list, DKC_Compiler *compiler) {
	CompilerList *list_pos;
	CompilerList *req_pos;

	for (list_pos = list; list_pos; list_pos = list_pos->next) {
		if (list_pos->compiler == compiler) {
			break;
		}
	}

	if (list_pos == NULL) {
		list = add_compiler_to_list(list, compiler);
	}

	for (req_pos = compiler->required_list; req_pos;
	     req_pos = req_pos->next) {
		list = traversal_compiler(list, req_pos->compiler);
	}

	return list;
}

void DKC_dispose_compiler(DKC_Compiler *compiler) {
	CompilerList *list = NULL;
	CompilerList *temp;

	list = traversal_compiler(list, compiler);
	for (CompilerList *pos = list; pos;) {
		for (FunctionDefinition *fun_def_pos = pos->compiler->function_list; fun_def_pos; fun_def_pos = fun_def_pos->next) {
			MEM_free(fun_def_pos->local_variable);
		}
		while (pos->compiler->required_list) {
			temp = pos->compiler->required_list;
			pos->compiler->required_list = temp->next;
			MEM_free(temp);
		}

		MEM_dispose_storage(pos->compiler->compile_storage);
		temp = pos->next;

		MEM_free(pos);

		pos = temp;
	}
}
