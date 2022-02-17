#ifndef FAT_DRIVER_H
#define FAT_DRIVER_H

/*
    One copy of FAT cluster map have 4608 bytes.
    We have two copies 4608 + 4608 = 9216
    512 (boot sector) + 9216  = 9728
    9728 + (512 * 2) (direntries - first is used as a volume label) = 10752
*/
#define FAT_ALL_METADATA_SIZE 10752u

typedef uint8_t fat_12_table_buff_t[FAT_ALL_METADATA_SIZE];
typedef uint8_t (*fat_12_table_buff_t_p)[FAT_ALL_METADATA_SIZE];

/* Functions */
void init_fat12_blank_floppy(fat_12_table_buff_t_p output);

#endif /* FAT_DRIVER_H */
