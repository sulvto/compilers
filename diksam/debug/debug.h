//
// Created by sulvto on 18-6-13.
//

#ifndef DIKSAM_DEBUG_H
#define DIKSAM_DEBUG_H

#include <stdio.h>
#include "DBG.h"

struct DBG_Controller_tag {
    FILE    *debug_write_fp;
    int     current_debug_level;
};

#endif //DIKSAM_DEBUG_H
