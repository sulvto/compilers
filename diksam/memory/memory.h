//
// Created by sulvto on 18-6-12.
//

#ifndef DIKSAM_MEM_H
#define DIKSAM_MEM_H

#include "MEM.h"

typedef union Header_tag Header;

struct MEM_Controller_tag {
	FILE 				*error_fp;
	MEM_ErrorHandler	error_handler;
	MEM_FailMode		fail_mode;
	Header				*block_header;
}

#endif //DIKSAM_MEM_H
