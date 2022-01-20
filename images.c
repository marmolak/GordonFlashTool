#define _LARGEFILE64_SOURCE
#define _DARWIN_FEATURE_64_BIT_INODE

#include "common.h"
#include "images.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

struct mem {
    void *m;
    size_t len;
};

static void safe_unmmap(struct mem *m_a)
{
    if (m_a == NULL || m_a == MAP_FAILED) {
        return;
    }

    CHECK_ERROR(munmap(m_a->m, m_a->len), 26);
}

void put_image_to(int fd_dst, const unsigned int slot, const char *const source)
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
    src_m.m = CHECK_ERROR_MMAP(mmap(NULL, src_m.len, PROT_READ, MAP_SHARED, fd_src, 0), FAIL_MMAP);
    safe_close(&fd_src);

    /* set and map destionation */
    dst_m.m = CHECK_ERROR_MMAP(mmap(NULL, dst_m.len, PROT_WRITE, MAP_SHARED, fd_dst, dst_offset), FAIL_MMAP);

    memcpy(dst_m.m, src_m.m, IMAGE_SIZE);
}