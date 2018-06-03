//
// Created by sulvto on 18-6-3.
//

#ifndef CROWBAR_MEMORY_H
#define CROWBAR_MEMORY_H

#include "MEM.h"

typedef union Header_tag Header;

struct MEM_Controller_tag {
    FILE *error_fp;
    MEM_ErrorHandler error_handler;
    MEM_FailMode     fail_mode;
    Header          *block_header;
};

#endif //CROWBAR_MEMORY_H
