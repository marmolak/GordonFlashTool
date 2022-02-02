#include "images.h"

#include <assert.h>
#include <sys/mman.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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

/* Put image to selected slot an initialize metadata. */
enum RET_CODES images_put_image_to(int fd_dst, const unsigned int slot, const char *const source)
{
    enum RET_CODES rc;
    int fd_src __attribute__ ((__cleanup__(safe_close))) = -1;
    const uint64_t dst_offset = slot * MAGIC_OFFSET;
    ssize_t size_bytes;
    ssize_t n;
    char *m_p;

    struct metadata meta = metadata_init();
    unsigned char checksum[METADATA_CHECKSUM_SIZE] = { '\0' };

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
        fprintf(stderr, "Source image size: %" PRIu64 " is bigger than %" PRIu64 ". Result on destination will be truncated.\n",
                src_m.len, IMAGE_SIZE);
        src_m.len = IMAGE_SIZE;
    }

    src_m.m = CHECK_ERROR_MMAP(mmap(NULL, src_m.len, PROT_READ, MAP_SHARED, fd_src, 0), FAIL_MMAP);
    safe_close(&fd_src);

    /* compute md5 sum */
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

    /* Should be wrapped into functions for metedata handling. */
    memcpy(meta.checksum, checksum, METADATA_CHECKSUM_SIZE);

    metadata_set_img_size(src_m.len, &meta);

    rc = metadata_write(fd_dst, slot, &meta);
    if (rc != FAIL_SUCC) {
        return rc;
    }

    return FAIL_SUCC;
}


enum RET_CODES images_export_image(int fd_src, const unsigned int slot, const char *const export_file_name)
{
    const uint64_t src_offset = slot * MAGIC_OFFSET;
    int fd_dst __attribute__ ((__cleanup__(safe_close))) = -1;
    struct metadata meta;
    ssize_t size_bytes;
    ssize_t n;
    char *m_p;
    enum RET_CODES rc;

    struct mem __attribute__ ((__cleanup__(safe_unmmap))) src_m = {
        .m = NULL,
        .len = IMAGE_SIZE
    };

    assert(export_file_name != NULL);

    rc = metadata_parse_slot(fd_src, slot, &meta);
    if (rc != FAIL_SUCC) {
        fprintf(stderr, "Unable to read/parse/find metadata.\n");
        return FAIL_FAIL;
    }


    src_m.len = metadata_get_img_size(&meta);

    if (export_file_name[0] == '-' && export_file_name[1] == '\0')
    {
        fd_dst = STDOUT_FILENO;
    } else {
        fd_dst = CHECK_ERROR(open(export_file_name, O_WRONLY | O_CREAT | O_SYNC | ADDITIONAL_OPEN_FLAGS, S_IRUSR | S_IWUSR), FAIL_OPEN);
    }

    CHECK_ERROR(lseek(fd_src, src_offset, SEEK_SET), FAIL_LSEEK);
    src_m.m = CHECK_ERROR_MMAP(mmap(NULL, src_m.len, PROT_READ, MAP_SHARED, fd_src, 0), FAIL_MMAP);

    n = src_m.len;
    m_p = (char *) src_m.m;
    do {
        /* Don't handle syscall restart for now. */
        size_bytes = CHECK_ERROR(write(fd_dst, m_p, n), FAIL_WRITE);
        n -= size_bytes;
        m_p += n;
    } while (n > 0);

    return FAIL_SUCC;
}