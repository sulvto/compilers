//
// Created by sulvto on 18-6-13.
//

#ifndef DIKSAM_DKC_H
#define DIKSAM_DKC_H

#include <stdio.h>
#include "DVM_code.h"

typedef struct DKC_Compiler_tag DKC_Compiler;

DKC_Compiler *DKC_create_compiler(void);

DVM_ExecutableList *DKC_compile(DKC_Compiler *compiler, FILE *fp, char *path);

DVM_ExecutableList *DKC_compile_string(DKC_Compiler *compiler, char **lines);

void DVM_dispose_compiler(DKC_Compiler *compiler);


#endif //DIKSAM_DKC_H
