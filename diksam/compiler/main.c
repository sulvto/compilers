//
// Created by sulvto on 18-6-6.
//

int main(int argc, chat *argv) {
    DKC_Compiler *compiler;
    FILE *fp;
    DVM_Executable *executable;
    DVM_VirtualMachine *dvm;

    if (argc < 2) {
        fprintf(stderr, "usage:%s filename arg1, arg2, ...\n", argv[0]);
        exit(1);
    }

    fp = fopen(argv[1], "r");

    if (fp == NULL) {
        fprintf(stderr, "%s no found.\n", argv[1]);
        exit(1);
    }

    setlocale(LC_CTYPE, "");
    compiler = DKC_create_compiler();
    executable = DKC_compile(compiler, fp);
    dvm = DVM_create_virtual_machine();
    DVM_add_excutable(dvm, executable);
    DVM_excute(dvm);
    DVM_dispose_compiler(compiler);
    DVM_dispose_virtual_machine(dvm);

    MEM_check_all_blocks();
    MEM_dump_blocks(stdout);

    return 0;
}