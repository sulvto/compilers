//
// Created by sulvto on 18-6-15.
//

#include "MEM.h"
#include "DVM.h"
#include "DVM_code.h"
#include "share.h"

void dvm_dispose_executable(DVM_Executable *executable) {
    MEM_free(executable->package_name);
    MEM_free(executable->path);
    for (int i = 0; i < executable->constant_pool_count; i++) {
        if (executable->constant_pool[i].tag == DVM_CONSTANT_STRING) {
            MEM_free(executable->constant_pool[i].u.c_string)
        }
    }

    MEM_free(executable->constant_pool);

    for (int i = 0; i < executable->global_variable_count; i++) {
        MEM_free(executable->global_variable[i].name);
        dispose_type_specifier(executable->global_variable[i].type);
    }

    MEM_free(executable->global_variable);


    // TODO


}
