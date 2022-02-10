#include "common.h"

#include <string.h>
#include <inttypes.h>

#include "fat/fat_driver.h"
#include "fat/fat.h"
#include "fat/bpb.h"
#include "fat/bootsect.h"
#include "fat/direntry.h"

/* Function declarations */
static struct bootsector33 init_blank_fat_33(void);


void init_fat12_blank_floppy(fat_12_table_buff_t_p output)
{
    uint8_t *p = (uint8_t *) output;

    struct bootsector33 boot_sector = init_blank_fat_33();
    const uint32_t clusters = BSWAP32_ONLY_ON_LE(0xF0FFFF00u);
    struct direntry volume_label;


    memset(p, '\0', FAT_ALL_METADATA_SIZE);
    memset(&volume_label, '\0', sizeof(volume_label));

    /* Boot sector with BPB */
    memcpy(p, (const void *) &boot_sector, sizeof(boot_sector));
    p += sizeof(boot_sector);

    /* Set cluster map */
    memcpy(p, &clusters, sizeof(uint32_t));
    p += 4608u;

    memcpy(p, &clusters, sizeof(uint32_t));
    p += 4608u;

    /* Dirent entry with volume label attribute. */
    memcpy(volume_label.deName, "GordonFT10", 11);
    volume_label.deAttributes = ATTR_VOLUME;

    memcpy(p, (const void *) &volume_label, sizeof(volume_label));
}

static struct bootsector33 init_blank_fat_33(void)
{
    struct bootsector33 boot_sector = {
        .bsJump         = { 0xEB, 0x3C, 0x90 }, /* Jump to nowhere for now */
        .bsOemName      = "GFT  1.0",
        .bsBPB          = { '\0' },
        .bsDriveNumber  = 0,
        .bsBootCode     = { '\0' },
        .bsBootSectSig0 = BOOTSIG0,
        .bsBootSectSig1 = BOOTSIG1,
    };

    const struct bpb33 bpd = {
        .bpbBytesPerSec = BSWAP16_ONLY_ON_BE(512),
        .bpbSecPerClust = 1,
        .bpbResSectors  = BSWAP16_ONLY_ON_BE(1),
        .bpbFATs        = 2,
        .bpbRootDirEnts = BSWAP16_ONLY_ON_BE(224),
        .bpbSectors     = BSWAP16_ONLY_ON_BE(2880),
        .bpbMedia       = 0xF0,
        .bpbFATsecs     = BSWAP16_ONLY_ON_BE(9),
        .bpbSecPerTrack = BSWAP16_ONLY_ON_BE(18),
        .bpbHeads       = BSWAP16_ONLY_ON_BE(2),
        .bpbHiddenSecs  = BSWAP16_ONLY_ON_BE(0),
    };

    memcpy((void *) boot_sector.bsBPB, (const void *) &bpd, sizeof(boot_sector.bsBPB));

    return boot_sector;
}