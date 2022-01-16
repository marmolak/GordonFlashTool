#include <assert.h>
#include <arpa/inet.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "common.h"
#include "metadata.h"

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

void metadata_set_short_label(const char *const short_label, struct metadata *const meta_p)
{
    assert(short_label != NULL);
    assert(meta_p != NULL);
    /* Don't allow uninitialised structs. */
    assert(ntohl(meta_p->magic) == METADATA_MAGIC && "Metadata struct must be initialised by metadata_init().");

    const size_t len = strnlen(short_label, METADATA_SHORT_LABEL_SIZE);
    memcpy((void *) meta_p->short_label, (void *) short_label, len);
    meta_p->short_label[METADATA_SHORT_LABEL_SIZE - 1] = '\0';
}

void metadata_parse(const int fd)
{
    struct metadata meta;
    
    CHECK_ERROR_GENERIC(read(fd, &meta, sizeof(meta)), ssize_t, FAIL_READ);
    if (ntohl(meta.magic) != METADATA_MAGIC) {
        printf("No metadata found.");
        return;
    }

    printf("%.63s", meta.short_label);
}

void metadata_write(const int fd, const struct metadata *const meta_p, const unsigned int slot)
{
    assert(meta_p != NULL);

    const uint64_t offset = (MAGIC_OFFSET * slot) + IMAGE_SIZE;
    CHECK_ERROR_GENERIC(lseek(fd, offset, SEEK_SET), off_t, FAIL_LSEEK);
    CHECK_ERROR(write(fd, (void *) meta_p, sizeof(*meta_p)), FAIL_WRITE);
}

void metadata_write_short_label_only(const int fd, const char *const short_label, const unsigned int slot)
{
    assert(short_label != NULL);

    const uint64_t offset = (MAGIC_OFFSET * slot) + IMAGE_SIZE + offsetof(struct metadata, short_label);
    const size_t len = strnlen(short_label, METADATA_SHORT_LABEL_SIZE);
    char tmp_short_label[METADATA_SHORT_LABEL_SIZE] = { '\0' };

    memcpy(tmp_short_label, short_label, len);
    tmp_short_label[METADATA_SHORT_LABEL_SIZE - 1] = '\0';

    CHECK_ERROR_GENERIC(lseek(fd, offset, SEEK_SET), off_t, FAIL_LSEEK);
    CHECK_ERROR(write(fd, (void *) &tmp_short_label, METADATA_SHORT_LABEL_SIZE), FAIL_WRITE);
}