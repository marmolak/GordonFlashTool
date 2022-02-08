#ifndef TOOLS_H
#define TOOLS_H

#include "common.h"

#include <inttypes.h>

enum RET_CODES write_content_to(int fd, uint8_t *buff, ssize_t buff_size);

#endif /* TOOLS_H */