#ifndef METADATA_H
#define METADATA_H

#include <stdint.h>

#include "common.h"
#include "cassert.h"

#define METADATA_MAGIC 0xB000B1E5u
#define METADATA_VERSION 0x0002u
#define METADATA_SHORT_LABEL_SIZE 64u
#define METADATA_NOTE_SIZE 8192u
#define METADATA_CHECKSUM_SIZE 64u

struct metadata
{
    uint32_t magic;
    uint16_t version;

    char short_label[METADATA_SHORT_LABEL_SIZE];
    uint32_t img_size;
    unsigned char checksum[METADATA_CHECKSUM_SIZE];
} __attribute__((packed));

STATIC_ASSERT(sizeof(struct metadata) <= IMAGES_GAP, Size_of_struct_metadata_must_be_able_to_fit_into_gap_of_IMAGES_GAP_bytes);


struct metadata metadata_init(void);

enum RET_CODES metadata_set_short_label(const char *const short_label, struct metadata *const meta_p);

void metadata_set_img_size(const uint32_t img_size, struct metadata *const meta_p);
uint32_t metadata_get_img_size(const struct metadata *const meta_p);

enum RET_CODES metadata_parse_slot(const int fd, const unsigned int slot, struct metadata *const meta_p);

enum RET_CODES metadata_write(const int fd, const unsigned int slot, const struct metadata *const meta_p);
enum RET_CODES metadata_write_checksum(const int fd, const unsigned int slot, const unsigned char *checksum, const uint32_t img_size);
enum RET_CODES metadata_write_short_label(const int fd, const unsigned int slot, const char *const short_label);

#endif /* METADATA_H */
