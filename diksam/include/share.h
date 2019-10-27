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

#define FILE_SEPARATOR  ('/')
#define FILE_PATH_SEPARATOR (':')

#define DIKSAM_REQUIRE_SUFFIX   (".dkh")
#define DIKSAM_IMPLEMENTATION_SUFFIX (".dkm")
#define DIKSAM_STACK_TRACE_CLASS ("StackTrace")
#define DIKSAM_PRINT_STACK_TRACE_FUNCTION ("Exception#print_stack_tracr")

#define EXCEPTION_CLASS_NAME    ("Exception")
#define BUG_EXCEPTION_CLASS_NAME    ("BugException")
#define RUNTIME_EXCEPTION_CLASS_NAME    ("RuntimeException")

#define ARRAY_METHOD_SIZE   "size"
#define ARRAY_METHOD_RESIZE "resize"
#define ARRAY_METHOD_INSERT "insert"
#define ARRAY_METHOD_REMOVE "remove"
#define ARRAY_METHOD_ADD    "add"
#define ARRAY_PREFIX   "array#"

#define STRING_METHOD_LENGTH    "length"
#define STRING_METHOD_SUBSTR    "substr"
#define STRING_PREFIX   "string#"

#define ARRAY_SIZE(array)   (sizeof(array) / sizeof((array)[0]))


// dispose.c
void dvm_dispose_executable(DVM_Executable *executable);

// disassemble.c
int dvm_dump_instruction(FILE *fp, DVM_Byte *code, int index);

void dvm_disassemble(DVM_Executable *executable);

// wchar.c
size_t dvm_wcslen(wchar_t *string);

wchar_t *dvm_wcscpy(wchar_t *dest, wchar_t *src);

wchar_t *dvm_wcsncpy(wchar_t *dest, wchar_t *src, size_t n);

int dvm_wcscmp(wchar_t *s1, wchar_t *s2);

wchar_t dvm_wcscat(wchar_t *s1, wchar_t *s2);

int dvm_mbstowcs_len(const char *src);

void dvm_mbstowcs(const char *src, wchar_t *dest);

wchar_t *dvm_mbstowcs_alloc(DVM_VirtualMachine *dvm, const char *src);

int dvm_wcstombs_len(const wchar_t *src);

void dvm_wcstombs_i(const wchar_t *src, char *dest);

char *dvm_wcstombs_alloc(const wchar_t *src);

char dvm_wctochar(wchar_t src);

int dvm_print_wcs(FILE *fp, wchar_t *string);

int dvm_print_wcs_ln(FILE *fp, wchar_t *string);

// util.c
SearchFileStatus dvm_search_file(char *search_path, char *search_file, char *found_path);

DVM_Boolean dvm_compare_string(char *str1, char *str2);

DVM_Boolean dvm_compare_package_name(char *p1, char *p2);

char *dvm_create_method_function_name(char *class_name, char *method_name);

void dvm_strncpy(char *dest, char *src, int buf_size);


//
SearchFileStatus dkc_dynamic_compile(DKC_Compiler *compiler, char *package_name, DVM_ExecutableList *list,
                                     DVM_ExecutableItem **add_top, char *search_file);
#endif //DIKSAM_SHARE_H
