#include <stdio.h>

#include "common.h"

#include <unistd.h>

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