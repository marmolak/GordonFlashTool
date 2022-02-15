#ifndef TOOLS_H
#define TOOLS_H

#include "common.h"

#include <inttypes.h>

enum RET_CODES tools_write_content_to(int fd, uint8_t *buff, ssize_t buff_size);
enum RET_CODES tools_read_from_write_to(int fd_src, int fd_dst, const ssize_t size);

#endif /* TOOLS_H */
