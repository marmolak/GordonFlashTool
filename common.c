#include "common.h"

#include <stdio.h>
#include <unistd.h>

/* */

void safe_close(int *const fd)
{
	if (fd == NULL) { 
		return;
	}

	if (*fd != -1) {
		CHECK_ERROR(close(*fd), FAIL_CLOSE);
		*fd = -1;
	}
}