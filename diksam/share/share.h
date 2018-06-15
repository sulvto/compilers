//
// Created by sulvto on 18-6-15.
//

#ifndef DIKSAM_SHARE_H
#define DIKSAM_SHARE_H

#include "DVM.h"
#include "DVM_code.h"

// dispose.c
void dvm_dispose_executable(DVM_Executable *executable);

// disassemble.c
int dvm_dump_instruction(FILE *fp, DVM_Byte *code, int index);

void dvm_disassemble(DVM_Executable *executable);



#endif //DIKSAM_SHARE_H
