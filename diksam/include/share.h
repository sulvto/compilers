//
// Created by sulvto on 18-6-15.
//

#ifndef DIKSAM_SHARE_H
#define DIKSAM_SHARE_H

#include "DVM_code.h"
#include "DKC.h"

typedef struct {
	char    *mnemonic;
	char    *parameter;
	int    stack_increment;
} OpcodeInfo;

typedef enum {
	SEARCH_FILE_SUCCESS,
	SEARCH_FILE_NOT_FOUND,
	SEARCH_FILE_PATH_TOO_LONG
} SearchFileStatus;

#define ARRAY_METHOD_SIZE   "size"
#define ARRAY_METHOD_RESIZE "resize"
#define ARRAY_METHOD_INSERT "insert"
#define ARRAY_METHOD_REMOVE "remove"
#define ARRAY_METHOD_ADD    "add"
#define ARRAY_PREFIX   "array#"

#define STRING_METHOD_LENGTH    "length"
#define STRING_METHOD_SUBSTR    "SUBSTR"
#define STRING_PREFIX   "string#"

#define ARRAY_SIZE(array)   (sizeof(array) / sizeof((array)[0]))


// dispose.c
void dvm_dispose_executable(DVM_Executable *executable);

// disassemble.c
int dvm_dump_instruction(FILE *fp, DVM_Byte *code, int index);

void dvm_disassemble(DVM_Executable *executable);

// wchar.c

wchar_t dvm_mbstowcs_alloc(DVM_VirtualMachine *dvm, const char *src);

#endif //DIKSAM_SHARE_H
