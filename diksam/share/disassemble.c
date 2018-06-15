//
// Created by sulvto on 18-6-15.
//
#include "share.h"

int dvm_dump_instruction(FILE *fp, DVM_Byte *code, int index) {
    Opcodeinfo *info = &dvm_opcode_info[code[index]];
    fprintf(fp, "%4d %s ", index, info->mnemonic);

    int value;

    for (int i = 0; info->parameter[i] != '\0'; i++) {
        switch (info->parameter[i]) {
            case 'b':
                value = code[index + 1];
                fprintf(fp, " %d", value);
                index++;
                break;
            case 's':
            case 'p':
                value = (code[index + 1] << 8) + code[index + 2];
                fprintf(fp, " %d", value);
                index += 2;
                break;
            default:
                DBG_assert(0, ("param..%s, i..%d", info->parmeter, i));
        }
    }

    index++;

    return index;
}

void dvm_disassemble(DVM_Executable *executable) {
    // TODO
}

