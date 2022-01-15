#include <assert.h>
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
        .magic          = METADATA_MAGIC,
        .version        = METADATA_VERSION,
        .short_label    = { '\0' },
        .note           = { '\0' },
    };

    return meta;
}

void metadata_set_short_label(const char *const short_label, struct metadata *const meta_p)
{
    assert(short_label != NULL);
    assert(meta_p != NULL);
    assert(meta_p->magic == METADATA_MAGIC);

    const size_t len = strnlen(short_label, METADATA_SHORT_LABEL_SIZE);
    memcpy((void *) meta_p->short_label, (void *) short_label, len);
    meta_p->short_label[METADATA_SHORT_LABEL_SIZE - 1] = '\0';
}

void metadata_parse(const int fd, const unsigned int slot)
{
    struct metadata meta;
    
    CHECK_ERROR_GENERIC(read(fd, &meta, sizeof(meta)), ssize_t, FAIL_READ);
    if (meta.magic != METADATA_MAGIC) {
        printf("No metadata found.");
        return;
    }

    printf("%.63s", meta.short_label);
}