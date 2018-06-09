//
// Created by sulvto on 18-6-9.
//

#include "MEM.h"
#include "DBG.h"
#include "diksamc.h"

static DKC_Compiler *st_current_compiler;

DKC_Compiler *dkc_get_current_compiler(void) {
	return st_current_compiler;
}

void dkc_set_current_compiler(DKC_Compiler *compiler) {
	st_current_compiler = compiler;
}
