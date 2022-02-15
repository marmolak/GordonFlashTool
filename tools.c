#include "tools.h"

#include <assert.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

enum RET_CODES tools_write_content_to(int fd, uint8_t *buff, ssize_t buff_size)
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


enum RET_CODES tools_read_from_write_to(int fd_src, int fd_dst, const ssize_t size)
{
    ssize_t bytes_send;
    ssize_t bytes_recv;
    ssize_t bytes_to_send;
    ssize_t bytes_on_dst = 0;

    uint8_t buff[4096 * 10];
    const uint8_t *buff_p = buff;

    do {
        bytes_recv = CHECK_ERROR(read(fd_src, (void *) buff, sizeof(buff)), FAIL_READ);
        bytes_to_send = bytes_recv;

        do {
            bytes_send = CHECK_ERROR(write(fd_dst, (const void *) buff_p, bytes_to_send), FAIL_WRITE);
            bytes_to_send -= bytes_send;
            buff_p += bytes_send;
        } while (bytes_to_send > 0);

        bytes_on_dst += bytes_recv;
        buff_p = buff;

    } while (bytes_on_dst != size);

    return FAIL_SUCC;
}