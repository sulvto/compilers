//
// Created by sulvto on 18-6-19.
//
#include <wchar.h>
#include "DBG.h"
#include "MEM.h"
#include "dvm_pri.h"

wchar_t *dvm_mbstowcs_alloc(DVM_VirtualMachine *dvm, const char *src) {
	int len = dvm_mbstowcs_len(src);
	wchar_t *ret;
	if (len < 0) {
		if (dvm) {
			dvm_error_i(dvm->current_executable->executable,
			            dvm->current_function, dvm->pc,
			            BAD_MULTIBYTE_CHARACTER_ERR,
			            DVM_MESSAGE_ARGUMENT_END);
		} else {
			dvm_error_i(NULL, NULL, NO_LINE_NUMBER_PC,
			            BAD_MULTIBYTE_CHARACTER_ERR,
			            DVM_MESSAGE_ARGUMENT_END);
		}
		return NULL;
	}
	ret = MEM_malloc(sizeof(wchar_t) * (len + 1));

	dvm_mbstowcs(src, ret);

	return ret;
}