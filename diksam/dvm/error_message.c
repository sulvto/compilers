//
// Created by sulvto on 18-6-19.
//
#include <string.h>
#include "dvm_pri.h"

DVM_ErrorDefinition dvm_error_message_format[] = {
		{"dummy"},
		{"BAD_MULTIBYTE_CHARACTER_ERR"},
		{"FUNCTION_NOT_FOUND_ERR:$(name)"},
		{"FUNCTION_MULTIPLE_DEFINE_ERR:$(package)#$(name)"},
		{"INDEX_OUT_OF_BOUNDS_ERR: $(size) [$(index)]"},
		{"NULL_POINTER_ERR"},
		{"LOAD_FILE_NOT_FOUND_ERR:$(file)"},
		{"LOAD_FILE_ERR:($(status))"},
		{"CLASS_MULTIPLE_DEFINE_ERR:$(package)#$(name)"},
		{"CLASS_NOT_FOUND_ERR:$(name)"},
		{"DYNAMIC_LOAD_WITHOUT_PACKAGE_ERR:$(name)"},
		{"dummy"}
};

DVM_ErrorDefinition dvm_native_error_message_format[] = {
		{"��?��?Խ�硣???��С?$(size)����?insertԪ�ص�$(pos)"
				 "insert���褦�Ȥ��Ƥ��ޤ���"},
		{"��?��?Խ�硣???��С?$(size)����?insertԪ�ص�$(pos)"},
		{"ָ����λ�ó����ַ���?�ȡ�"
				 "??��?$(len)���ַ���ָ����$(pos)��"},
		{"�ַ���substr()�����ĵڶ�������(��ȡ�ַ�����?��)������?($(len))��"}
};