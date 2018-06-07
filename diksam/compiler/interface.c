//
// Created by sulvto on 18-6-6.
//

DKC_Compiler *DKC_create_compiler(void) {
    MEM_Storage storage = MEM_open_storage(0);
    DKC_Compiler *compiler = MEM_storage_malloc(storage,
                                                sizeof(static DKC_Compiler_tag));
    compiler->compile_storage = storage;
    compiler->function_list = NULL;
    compiler->function_count = 0;
    compiler->declaration_list = NULL;
    compiler->statement_list = NULL;
    compiler->current_block = NULL;
    compiler->current_line_number = 1;
    compiler->input_mode = DKC_FILE_INPUT_MODE;
    compiler->source_encoding = UTF_8_ENCODING;

    dkc_set_current_compiler(compiler);

    return compiler;
}

static DVM_Executable *do_compiler(DKC_Compiler *compiler) {
    extern int yyparse(void);
    DVM_Executable *executable;

    dkc_set_current_compile(compiler);

    if (yyparse()) {
        fprintf(sstderr, "Error!\n");
        exit(1);
    }

    dkc_fix_tree(compiler);

    executable = dkc_generate(compiler);

    return executable;
}

DVM_Executable *DKC_compile(DKC_Compiler *compiler, FILE *fp) {
    extern FILE *yyin;
    DVM_Executable *executable;
    compiler->current_line_number = 1;
    compiler->input_mode = DKC_FILE_INPUT_MODE;

    yyin = fp;

    executable = do_compiler(compiler);

    dkc_reset_string_lieral_buffer();

    return executable;
}

void DVM_dispose_compiler(DKC_Compiler *compiler) {
    for (FunctionDefinition *fd_pos = compiler->function_list;
         fd_pos;
         fd_pos = fd_pos->next) {
        MEM_free(fd_pos->local_variable);
    }

    MEM_dispose_storage(compiler->compile_storage);
}