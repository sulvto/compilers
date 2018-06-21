//
// Created by sulvto on 18-6-19.
//

#include <stdio.h>
#include <wchar.h>
#include <limits.h>
#include "DBG.h"
#include "MEM.h"

size_t dvm_wcslen(wchar_t *string) {
	return wcslen(string);
}

wchar_t *dvm_wcscpy(wchar_t *dest, wchar_t *src) {
	return wcscpy(dest, src);
}

wchar_t *dvm_wcsncpy(wchar_t *dest, wchar_t *src, size_t n) {
	return wcsncpy(dest, src, n);
}

int dvm_wcscmp(wchar_t *s1, wchar_t *s2) {
	return wcscmp(s1, s2);
}

int dvm_mbstowcs_len(const char *src) {
	printf("dvm_mbstowcs_len\t");
	int src_index, dest_index;
	int status;
	mbstate_t ps;
	memset(&ps, 0, sizeof(mbstate_t));
	for (src_index = dest_index = 0; src[src_index] != '\0';) {
		status = mbrtowc(NULL, &src[src_index], MB_LEN_MAX, &ps);
		if (status < 0) {
			return status;
		}
		dest_index++;
		src_index += status;
	}

	printf("dvm_mbstowcs_len end\n");
	return dest_index;
}

void dvm_mbstowcs(const char *src, wchar_t *dest) {
	int src_index, dest_index;
	int status;
	mbstate_t ps;

	memset(&ps, 0, sizeof(mbstate_t));
	for (src_index = dest_index = 0; src[src_index] != '\0';) {
		status = mbrtowc(&dest[dest_index], &src[src_index], MB_LEN_MAX, &ps);
		dest_index++;
		src_index += status;
	}

	dest[dest_index] = L'\0';
}

wchar_t dvm_wcscat(wchar_t *s1, wchar_t *s2) {
	return wcscat(s1, s2);
}

int dvm_wcstombs_len(const wchar_t *src) {
	int src_index, dest_index;
	int status;
	char dummy[MB_LEN_MAX];
	mbstate_t ps;
	memset(&ps, 0, sizeof(mbstate_t));
	for (src_index = dest_index = 0; src[src_index] != L'\0';) {
		status = wcrtomb(dummy, src[src_index], &ps);
		src_index++;
		dest_index += status;
	}

	return dest_index;
}

void dvm_wcstombs_i(const wchar_t *src, char *dest) {
	int src_index,dest_index;
	int status;
	mbstate_t ps;

	memset(&ps, 0, sizeof(mbstate_t));
	for (src_index = dest_index = 0; src[src_index] != '\0';) {
		status = wcrtomb(&dest[dest_index], src[src_index], &ps);
		src_index++;
		dest_index += status;
	}

	dest[dest_index] = '\0';
}

char *dvm_wcstombs_alloc(const wchar_t *src) {
	int len = dvm_wcstombs_len(src);
	char *ret = MEM_malloc(len + 1);
	dvm_wcstombs_i(src, ret);

	return ret;
}

char dvm_wctochar(wchar_t src) {
	mbstate_t ps;
	int status;
	char dest;

	memset(&ps, 0, sizeof(mbstate_t));
	status = wcrtomb(&dest, src, &ps);
	DBG_assert(status == 1, ("wcrtomb status..%d\n", status));

	return dest;
}

int dvm_print_wcs(FILE *fp, wchar_t *string) {
	int result;
	char *tmp = dvm_wcstombs_alloc(string);
	result = fprintf(fp, "%s", tmp);

	MEM_free(tmp);

	return result;
}

int dvm_print_wcs_ln(FILE *fp, wchar_t *string) {
	printf("dvm_print_wcs_ln \t");
	int result = dvm_print_wcs(fp, string);
	fprintf(fp, "\n");

	printf("dvm_print_wcs_ln end\n");
	return result;
}