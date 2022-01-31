#include "images.h"

#include <sys/mman.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "file_dev_ops.h"
#include "crypt_md5.h"
#include "metadata.h"

struct mem {
    void *m;
    uint64_t len;
};

static void safe_unmmap(struct mem *m_a)
{
    if (m_a == NULL || m_a->m == MAP_FAILED || m_a->m == NULL) {
        return;
    }

    munmap(m_a->m, m_a->len);
}

enum RET_CODES images_put_image_to(int fd_dst, const unsigned int slot, const char *const source)
{
    enum RET_CODES rc;
    int fd_src __attribute__ ((__cleanup__(safe_close))) = -1;
    const uint64_t dst_offset = slot * MAGIC_OFFSET;
    ssize_t size_bytes;
    ssize_t n;
    char *m_p;

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

    rc = get_file_or_device_size(fd_src, &src_m.len);
    if (rc != FAIL_SUCC) {
        return rc;
    }

    if (src_m.len > IMAGE_SIZE) {
        fprintf(stderr, "Source image size: %llu is bigger than %llu. Result on destination will be truncated.",
                src_m.len, IMAGE_SIZE);
        src_m.len = IMAGE_SIZE;
    }

    src_m.m = CHECK_ERROR_MMAP(mmap(NULL, src_m.len, PROT_READ, MAP_SHARED, fd_src, 0), FAIL_MMAP);
    safe_close(&fd_src);

    /* compute md5 sum */
    unsigned char checksum[METADATA_CHECKSUM_SIZE];
    md5sum(src_m.m, src_m.len, checksum);

/* MacOS 12 on /dev/rdisk returns: Operation not supported by device... so just use write. */
#if defined(__APPLE__) && defined(__MACH__)
    (void) dst_m;
    n = src_m.len;
    m_p = (char *) src_m.m;

    CHECK_ERROR(lseek(fd_dst, dst_offset, SEEK_SET), FAIL_LSEEK);

    do {
        /* Don't handle syscall restart for now. */
        size_bytes = CHECK_ERROR(write(fd_dst, m_p, n), FAIL_WRITE);
        n -= size_bytes;
        m_p += n;
    } while (n > 0);

#else
    (void) n;
    (void) m_p;
    (void) size_bytes;

    /* set and map destionation */
    dst_m.m = CHECK_ERROR_MMAP(mmap(NULL, dst_m.len, PROT_WRITE, MAP_SHARED, fd_dst, dst_offset), FAIL_MMAP);
    memcpy(dst_m.m, src_m.m, src_m.len);
#endif

    rc = metadata_write_checksum(fd_dst, slot, checksum, src_m.len);
    if (rc != FAIL_SUCC) {
        return rc;
    }

    return FAIL_SUCC;
}