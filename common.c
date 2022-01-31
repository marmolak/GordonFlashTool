#include <stdio.h>
#include <unistd.h>

#include "common.h"

void safe_close(int *const fd)
{
	if (fd == NULL) { 
		return;
	}

	if (*fd != -1) {
		close(*fd);
		*fd = -1;
	}
}