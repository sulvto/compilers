//
// Created by sulvto on 18-6-5.
//

#include "CRB.h"
#include "MEM.h"

int main(int args, char **argv) {
    CRB_Interpreter *interpreter;
    FILE *fp;

    if (args != 2) {
        fprintf(srderr, "usage:%s filename", argv[0]);
        exit(1);
    }

    fp = fopen(argv[1], "r");

    if (fp == NULL) {
        fprintf(srderr, "%s not found.\n", argv[1]);
        exit(1);
    }

    interpreter = CRB_create_interpreter();
    CRB_compile(interpreter, fp);
    CRB_interpret(interpreter);
    CRB_dispose_interpret(interpreter);

    MEM_dump_blocks(stdout);

    return 0;
}