#ifndef MOUNT_H
#define MOUNT_H

#include "common.h"

enum RET_CODES mount_slot(int fd, const unsigned int slot, const char *const image_name);

#endif /* end of MOUNT_H */