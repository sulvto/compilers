//
// Created by sulvto on 18-6-6.
//

typedef struct {
    int             size;
    int             alloc_size;
    DVM_Byte        *code;
    int             label_table_size;
    int             label_table_alloc_size;
    LabelTable      *label_table;
    int             line_number_size;
    DVM_LineNumber  *line_number;
} OpcodeBuf;

static void add_global_variable(DKC_Compiler *compiler, DVM_Executable *executable) {

}

static void copy_function(FunctionDefinition *src, DVM_Function *dest) {
    dest->type = copy_type_specifier(src->type);
    dest->name = MEM_strdup(src->name);
    dest->parameter = copy_paramter_list(src->parameter,
                                         &dest->parameter_count);

    if (src->block) {
        dest->local_variable = copy_local_varialbes(src,
                                                    dest->parameter_count);
        dest->local_variable_count = src->local_variable_count - dest->parameter_count;
    } else {
        dest->local_variable = NULL;
        dest->local_variable_count = 0;
    }
}

static void generate_statement_list(DVM_Executable *executable,
                                    Block *current_block,
                                    StatementList *statement_list,
                                    OpcodeBuf *opcode_buf) {

    for (StatementList *pos = statement_list; pos; pos = pos->next) {
        switch (pos->statement->type) {
            case EXPRESSION_STATEMENT :
            case IF_STATEMENT:
            case SWITCH_STATEMENT:
            case WHILE_STATEMENT:
            case DO_WHILE_STATEMENT:
            case FOR_STATEMENT:
            case FOREACH_STATEMENT:
            case RETURN_STATEMENT:
            case BREAK_STATEMENT:
            case CONTINUE_STATEMENT:
            case TRY_STATEMENT:
            case THROW_STATEMENT:
            case DECLARATION_STATEMENT:
            case STATEMENT_TYPE_COUNT_PLUS_1:
            default:
                DBG_assert(0, ("pos->statement->type..", pos->statement->type));
        }
    }


}

static void init_opcode_buf(OpcodeBuf *opcode_buf) {
    opcode_buf->size = 0;
    opcode_buf->alloc_size = 0;
    opcode_buf->code = 0;
    opcode_buf->label_table = NULL;
    opcode_buf->label_table_size = 0;
    opcode_buf->label_table_alloc_size = 0;
    opcode_buf->line_number = NULL;
    opcode_buf->line_number_size = 0;
}

static void add_functions(DKC_Compiler *compiler, DVM_Executable *executable) {
    FunctionDefinition *function_def;
    int function_count = 0;

    for (FunctionDefinition *function_def = compiler->function_list;
         function_def;
         function_def = function_def.next) {
        function_count++;
    }

    executable->function_count = function_count;
    executable->function = MEM_malloc(sizeof(DVM_Function) * function_count);

    int i;
    OpcodeBuf opcode_buf;

    for (FunctionDefinition *function_def = compiler->function_list, i = 0;
         function_def;
         function_def = function_def->next, i++) {
        copy_function(function_def, &executable->function[i]);

        if (function_def->block) {
            init_opcode_buf(&opcode_buf);
            generate_statement_list(executable,
                                    function_def->block,
                                    function_def->block->statement_list,
                                    &opcode_buf);
            executable->function[i].is_implemented = DVM_TURE;
            executable->function[i].code_size = opcode_buf.size;
            executable->function[i].code = fix_opcode_buf(&opcode_buf);
            executable->function[i].line_number_size = opcode_buf.line_number_size;
            executable->function[i].line_number = opcode_buf.line_number;
            executable->function[i].need_stack_size = calc_need_stack_size(executable->function[i].code,
                                                                           executable->function[i].coed_size);
        } else {
            executable->function[i].is_implemented = DVM_FALSE;
        }
    }
}

static void add_top_level(DKC_Compiler *compiler, DVM_Executable *executable) {
    OpcodeBuf opcode_buf;
    init_opcode_buf(&opcode_buf);
    generate_statement_list(executable, NULL, compiler->statement_list, &opcode_buf);

    executable->code_size = opcode_buf.size;
    executable->code = fix_opcode_buf(&opcode_buf);
    executable->line_number_size = opcode_buf.line_number_size;
    executable->line_number = opcode_buf.line_number;
    executable->need_stack_size = calc_need_stack_size(executable->code, executable->code_size);
}

DVM_Executable *dkc_generate(DKC_Compiler *compiler) {
    DVM_Executable *executable = alloc_executable();

    add_global_variable(compiler, executable);
    add_functions(compiler, executable);
    add_top_level(compiler, executable);

    return executable;
}