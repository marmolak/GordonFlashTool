#include "images.h"
#include "common.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

struct mem {
    void *m;
    size_t len;
};

static void safe_unmmap(struct mem *m_a)
{
    if (m_a == NULL) {
        return;
    }

    CHECK_ERROR(munmap(m_a->m, m_a->len), 26);
}

void put_image_to(const int fd_dst, const unsigned int slot, const char *const source)
{
    int fd_src __attribute__ ((__cleanup__(safe_close))) = -1;
    const uint64_t dst_offset = slot * MAGIC_OFFSET;

    struct mem __attribute__ ((__cleanup__(safe_unmmap))) src_m = {
        .m = NULL,
        .len = IMAGE_SIZE
    };

    struct mem __attribute__ ((__cleanup__(safe_unmmap))) dst_m = {
        .m = NULL,
        .len = IMAGE_SIZE
    };

    /* map source */
    fd_src = CHECK_ERROR(open(source, O_RDONLY | ADDITIONAL_OPEN_FLAGS), FAIL_OPEN);
    src_m.m = CHECK_ERROR_P(mmap(NULL, src_m.len, PROT_READ, MAP_SHARED, fd_src, 0), 66);
    safe_close(&fd_src);

    /* set and map destionation */
    CHECK_ERROR_GENERIC(lseek(fd_dst, dst_offset, SEEK_SET), off_t, FAIL_LSEEK);
    dst_m.m = CHECK_ERROR_P(mmap(NULL, src_m.len, PROT_WRITE, MAP_SHARED, fd_dst, 0), 66);

    memcpy(dst_m.m, src_m.m, IMAGE_SIZE);
}