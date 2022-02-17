#include "metadata.h"

#include <assert.h>
#include <arpa/inet.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>


struct metadata metadata_init(void)
{
    struct metadata meta = {
        .magic          = htonl(METADATA_MAGIC),
        .version        = htons(METADATA_VERSION),
        .short_label    = { '\0' },
        .img_size       = 0,
        .checksum       = { '\0' },
    };

    return meta;
}

    enum RET_CODES metadata_set_short_label(const char *const short_label, struct metadata *const meta_p)
{
    assert(short_label != NULL);
    assert(meta_p != NULL);
    /* Don't allow uninitialised structs. */
    assert(ntohl(meta_p->magic) == METADATA_MAGIC && "Metadata struct must be initialised by metadata_init().");

    const size_t len = ({ 
        size_t _len = strlen(short_label);
        _len > METADATA_SHORT_LABEL_SIZE ? METADATA_SHORT_LABEL_SIZE : _len;
    });
    memcpy((void *) meta_p->short_label, short_label, len);
    meta_p->short_label[METADATA_SHORT_LABEL_SIZE - 1] = '\0';

    return FAIL_SUCC;
}


void metadata_set_img_size(const uint32_t img_size, struct metadata *const meta_p)
{
    assert(meta_p != NULL);

    meta_p->img_size = (img_size);
}

uint32_t metadata_get_img_size(const struct metadata *const meta_p)
{
    uint32_t img_size;

    assert(meta_p != NULL);
    
    img_size = ntohl(meta_p->img_size);

    if (img_size > IMAGE_SIZE) {
        img_size = IMAGE_SIZE;
    }

    return img_size;
}

static enum RET_CODES metadata_parse(const int fd, struct metadata *const meta_p)
{
    struct metadata meta;
    
    CHECK_ERROR_GENERIC(read(fd, &meta, sizeof(meta)), ssize_t, FAIL_READ);
    if (ntohl(meta.magic) != METADATA_MAGIC) {
        return FAIL_NOMETA;
    }

    memcpy(meta_p, &meta, sizeof(struct metadata));
    return FAIL_SUCC;
}


enum RET_CODES metadata_parse_slot(const int fd, const unsigned int slot, struct metadata *const meta_p)
{
    const uint64_t offset = slot * MAGIC_OFFSET + IMAGE_SIZE;
    enum RET_CODES rc;

    CHECK_ERROR_GENERIC(lseek(fd, offset, SEEK_SET), off_t, FAIL_LSEEK);
    rc = metadata_parse(fd, meta_p);

    return rc;
}


enum RET_CODES metadata_write(const int fd, const unsigned int slot, const struct metadata *const meta_p)
{
    const uint64_t offset = (MAGIC_OFFSET * slot) + IMAGE_SIZE;

    assert(meta_p != NULL);

    CHECK_ERROR_GENERIC(lseek(fd, offset, SEEK_SET), off_t, FAIL_LSEEK);
    CHECK_ERROR_GENERIC(write(fd, meta_p, sizeof(*meta_p)), ssize_t, FAIL_WRITE);
    return FAIL_SUCC;
}

enum RET_CODES metadata_write_checksum(const int fd, const unsigned int slot, const unsigned char *checksum, const uint32_t img_size)
{
    const uint64_t offset_checksum = (MAGIC_OFFSET * slot) + IMAGE_SIZE + offsetof(struct metadata, checksum);
    const uint64_t offset_img_size = (MAGIC_OFFSET * slot) + IMAGE_SIZE + offsetof(struct metadata, img_size);

    uint32_t img_size_right_endian = htonl(img_size);

    assert(checksum != NULL);

    CHECK_ERROR_GENERIC(lseek(fd, offset_checksum, SEEK_SET), off_t, FAIL_LSEEK);
    CHECK_ERROR_GENERIC(write(fd, (const void *) checksum, METADATA_CHECKSUM_SIZE), ssize_t, FAIL_WRITE);

    CHECK_ERROR_GENERIC(lseek(fd, offset_img_size, SEEK_SET), off_t, FAIL_LSEEK);
    CHECK_ERROR_GENERIC(write(fd, (void *) &img_size_right_endian, sizeof(img_size)), ssize_t, FAIL_WRITE);
    return FAIL_SUCC;
}

enum RET_CODES metadata_write_short_label(const int fd, const unsigned int slot, const char *const short_label)
{
    const uint64_t offset = (MAGIC_OFFSET * slot) + IMAGE_SIZE + offsetof(struct metadata, short_label);
    const size_t len = ({
        size_t _len = strlen(short_label);
        _len > METADATA_SHORT_LABEL_SIZE ? METADATA_SHORT_LABEL_SIZE : _len;
    });
    char tmp_short_label[METADATA_SHORT_LABEL_SIZE] = { '\0' };

    assert(short_label != NULL);

    memcpy(tmp_short_label, short_label, len);
    tmp_short_label[METADATA_SHORT_LABEL_SIZE - 1] = '\0';

    CHECK_ERROR_GENERIC(lseek(fd, offset, SEEK_SET), off_t, FAIL_LSEEK);
    CHECK_ERROR_GENERIC(write(fd, (void *) &tmp_short_label, METADATA_SHORT_LABEL_SIZE), ssize_t, FAIL_WRITE);

    return FAIL_SUCC;
}
