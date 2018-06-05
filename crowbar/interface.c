//
// Created by sulvto on 18-6-5.
//

#include "MEM.h"
#include "crowbar.h"

static void add_native_function(CRB_Interpreter *interpreter) {
    CRB_add_native_function(interpreter, "print", crb_native_print_proc);
    CRB_add_native_function(interpreter, "fopen", crb_native_fopen_proc);
    CRB_add_native_function(interpreter, "fclose", crb_native_fclose_proc);
    CRB_add_native_function(interpreter, "fgets", crb_native_fgets_proc);
    CRB_add_native_function(interpreter, "fputs", crb_native_fputs_proc);
    CRB_add_native_function(interpreter, "new_array", crb_native_new_array_proc);
}


CRB_Interpreter *CRB_create_interpreter(void) {
    MEM_Storage storage = MEM_open_storage(0);
    CRB_Interpreter *interpreter = MEM_storage_malloc(storage, sizeof(struct CRB_Interpreter_tag));
    interpreter->interpreter_storage = storage;
    interpreter->execute_storage = NULL;
    interpreter->variable = NULL;
    interpreter->function_list = NULL;
    interpreter->statement_list = NULL;
    interpreter->current_line_number = 0;
    interpreter->stack.stack_alloc_size = 0;
    interpreter->stack.stack_pointer = 0;
    interpreter->stack.stack = MEM_malloc(sizeof(CRB_Value) * STACK_ALLOC_SIZE);
    interpreter->heap.current_heap_size = 0;
    interpreter->heap.current_threshold = 0;
    interpreter->heap.header = NULL;
    interpreter->top_environment = NULL;

    crb_set_current_interpreter(interpreter);
    add_native_function(interpreter);

    return interpreter;
}

void CRB_compile(CRB_Interpreter *interpreter, FILE *fp) {
    extern int yyparse(void);
    extern FILE *yyin;
    crb_set_current_interpreter(interpreter);

    yyin = fp;

    if (yyparse()) {
        fprintf(stderr, "Error !\n");
        exit(1);
    }

    crb_reset_string_literal_buffer();
}

void CRB_interpret(CRB_Interpreter *interpreter) {
    interpreter->execute_storage = MEM_open_storage(0);
    crb_add_std_fp(interpreter);
    crb_execute_statement_list(interpreter, NULL, interpreter->statement_list);
    crb_garbage_collect(interpreter);
}

static void release_global_strings(CRB_Interpreter *interpreter) {
    // ???
    while (interpreter->vaiable) {
        Vaiable *temp = interpreter->vaiable;
        interpreter->vaiable = temp->next;
    }
}

void CRB_dispose_interpret(CRB_Interpreter *interpreter) {
    release_global_strings(interpreter);

    if (interpreter->execute_storage) {
        MEM_dispose_storage(interpreter->execute_storage);
    }
    interpreter->variable = NULL;
    crb_garbage_collect(interpreter);

    DBG_assert(interpreter->heap.current_heap_size == 0,
               ("%d bytes leaked.\n", interpreter->heap.current_heap_size));

    MEM_free(interpreter->stack.stack);
    MEM_dispose_storage(interpreter->interpreter_storage);
}

void CRB_add_native_function(CRB_Interpreter *interpreter, char *name, CRB_NativeFunctionProc *proc) {
    FunctionDefinition *fd = crb_malloc(sizeof(FunctionDefinition));
    fd->name = name;
    fd->type = NATIVE_FUNCTION_DEFINITION;
    fd->u.native_f.proc = proc;
    fd->next = interpreter->function_list;

    interpreter->function_list = fd;
}
