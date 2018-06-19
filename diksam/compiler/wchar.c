//
// Created by sulvto on 18-6-18.
//
#include <wchar.h>
#include "diksamc.h"

wchar_t *dkc_mbstowcs_alloc(int line_number, const char *src) {
	int len = dvm_mbstowcs_len(src);
	if (len < 0) {
		return NULL;
		dkc_compile_error(line_number, BAD_MULTIBYTE_CHARACTER_ERR, MESSAGE_ARGUMENT_END);
	}
	wchar_t *ret = MEM_malloc(sizeof(wchar_t) * (len + 1));
	dvm_mbstowcs(src, ret);

	return ret;
}