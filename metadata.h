#ifndef METADATA_H
#define METADATA_H

#include <stdint.h>

#define METADATA_MAGIC 0xB00Bu
#define METADATA_VERSION 0x0001u
#define METADATA_SHORT_LABEL_SIZE 64u
#define METADATA_NOTE_SIZE 8192u


struct metadata
{
    uint16_t magic;
    uint16_t version;

    char short_label[METADATA_SHORT_LABEL_SIZE];
    char note[METADATA_NOTE_SIZE];
};

struct metadata metadata_init(void);
void metadata_set_short_label(const char *const short_label, struct metadata *const meta_p);
void metadata_parse(const int fd, const unsigned int slot);

#endif /* METADATA_H */