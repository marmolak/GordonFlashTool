#ifndef METADATA_H
#define METADATA_H

#include <stdint.h>

#include "cassert.h"
#include "common.h"

#define METADATA_MAGIC 0xB000B1E5u
#define METADATA_VERSION 0x0001u
#define METADATA_SHORT_LABEL_SIZE 64u
#define METADATA_NOTE_SIZE 8192u

struct metadata
{
    uint32_t magic;
    uint16_t version;

    char short_label[METADATA_SHORT_LABEL_SIZE];
    char note[METADATA_NOTE_SIZE];
} __attribute__((packed));

STATIC_ASSERT(sizeof(struct metadata) <= IMAGES_GAP, Size_of_struct_metadata_must_be_able_to_fit_into_gap_of_IMAGES_GAP_bytes);


struct metadata metadata_init(void);

enum RET_CODES metadata_set_short_label(const char *const short_label, struct metadata *const meta_p);
enum RET_CODES metadata_parse(const int fd);

enum RET_CODES metadata_write(const int fd, const struct metadata *const meta_p, const unsigned int slot);
enum RET_CODES metadata_write_short_label_only(const int fd, const char *const short_label, const unsigned int slot);

#endif /* METADATA_H */
