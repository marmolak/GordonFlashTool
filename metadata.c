#include <assert.h>
#include <arpa/inet.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "metadata.h"
#include "banned.h"

struct metadata metadata_init(void)
{
    struct metadata meta = {
        .magic          = htonl(METADATA_MAGIC),
        .version        = htons(METADATA_VERSION),
        .short_label    = { '\0' },
        .note           = { '\0' },
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
    
    CHECK_ERROR_GENERIC(read(fd, &meta, sizeof(meta)), ssize_t, FAIL_READ);
    if (ntohl(meta.magic) != METADATA_MAGIC) {
        printf("No metadata found.");
        return FAIL_SUCC;
    }

    printf("%.63s", meta.short_label);
    return FAIL_SUCC;
}

enum RET_CODES metadata_write(const int fd, const struct metadata *const meta_p, const unsigned int slot)
{
    assert(meta_p != NULL);

    const uint64_t offset = (MAGIC_OFFSET * slot) + IMAGE_SIZE;
    CHECK_ERROR_GENERIC(lseek(fd, offset, SEEK_SET), off_t, FAIL_LSEEK);
    CHECK_ERROR(write(fd, (void *) meta_p, sizeof(*meta_p)), FAIL_WRITE);
    return FAIL_SUCC;
}

enum RET_CODES metadata_write_short_label_only(const int fd, const char *const short_label, const unsigned int slot)
{
    assert(short_label != NULL);

    const uint64_t offset = (MAGIC_OFFSET * slot) + IMAGE_SIZE + offsetof(struct metadata, short_label);
    const size_t len = ({
        size_t _len = strlen(short_label);
        _len > METADATA_SHORT_LABEL_SIZE ? METADATA_SHORT_LABEL_SIZE : _len;
    });
    char tmp_short_label[METADATA_SHORT_LABEL_SIZE] = { '\0' };

    memcpy(tmp_short_label, short_label, len);
    tmp_short_label[METADATA_SHORT_LABEL_SIZE - 1] = '\0';

    CHECK_ERROR_GENERIC(lseek(fd, offset, SEEK_SET), off_t, FAIL_LSEEK);
    CHECK_ERROR(write(fd, (void *) &tmp_short_label, METADATA_SHORT_LABEL_SIZE), FAIL_WRITE);

    return FAIL_SUCC;
}