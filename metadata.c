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
    memcpy((void *) meta_p->short_label, (void *) short_label, len);
    meta_p->short_label[METADATA_SHORT_LABEL_SIZE - 1] = '\0';

    return FAIL_SUCC;
}

enum RET_CODES metadata_parse(const int fd)
{
    struct metadata meta;
    unsigned int p;
    
    CHECK_ERROR_GENERIC(read(fd, &meta, sizeof(meta)), ssize_t, FAIL_READ);
    if (ntohl(meta.magic) != METADATA_MAGIC) {
        printf("No metadata found.");
        return FAIL_SUCC;
    }

    printf("%.63s - ", meta.short_label);
    for (p = 0; p < 16; ++p)
    {
        printf("%.2x", meta.checksum[p]);
    }

    return FAIL_SUCC;
}

enum RET_CODES metadata_write(const int fd, const unsigned int slot, const struct metadata *const meta_p)
{
    const uint64_t offset = (MAGIC_OFFSET * slot) + IMAGE_SIZE;

    assert(meta_p != NULL);

    CHECK_ERROR_GENERIC(lseek(fd, offset, SEEK_SET), off_t, FAIL_LSEEK);
    CHECK_ERROR(write(fd, (void *) meta_p, sizeof(*meta_p)), FAIL_WRITE);
    return FAIL_SUCC;
}

enum RET_CODES metadata_write_checksum(const int fd, const unsigned int slot, const unsigned char *const checksum, const uint32_t img_size)
{
    const uint64_t offset_checksum = (MAGIC_OFFSET * slot) + IMAGE_SIZE + offsetof(struct metadata, checksum);
    const uint64_t offset_img_size = (MAGIC_OFFSET * slot) + IMAGE_SIZE + offsetof(struct metadata, img_size);

    uint32_t img_size_right_endian = htonl(img_size);

    assert(checksum != NULL);

    CHECK_ERROR_GENERIC(lseek(fd, offset_checksum, SEEK_SET), off_t, FAIL_LSEEK);
    CHECK_ERROR(write(fd, (void *) checksum, METADATA_CHECKSUM_SIZE), FAIL_WRITE);

    CHECK_ERROR_GENERIC(lseek(fd, offset_img_size, SEEK_SET), off_t, FAIL_LSEEK);
    CHECK_ERROR(write(fd, (void *) &img_size_right_endian, sizeof(img_size)), FAIL_WRITE);
    return FAIL_SUCC;
}

enum RET_CODES metadata_write_short_label_only(const int fd, const unsigned int slot, const char *const short_label)
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
    CHECK_ERROR(write(fd, (void *) &tmp_short_label, METADATA_SHORT_LABEL_SIZE), FAIL_WRITE);

    return FAIL_SUCC;
}