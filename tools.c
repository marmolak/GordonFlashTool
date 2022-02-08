#include "tools.h"

#include <assert.h>
#include <unistd.h>

enum RET_CODES write_content_to(int fd, uint8_t *buff, ssize_t buff_size)
{
    uint8_t *bytes_p = buff;
    ssize_t bytes_send;
    ssize_t bytes_size = buff_size;

    assert(buff != NULL);

    do {
        /* Don't handle syscall restart for now. */
        bytes_send = CHECK_ERROR(write(fd, (const void *) bytes_p, bytes_size), FAIL_WRITE);
        bytes_size -= bytes_send;
        bytes_p += bytes_send;
    } while (bytes_size > 0);

    return FAIL_SUCC;
}