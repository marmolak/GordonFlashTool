#include "common.h"

#include <errno.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <inttypes.h>

#include "images.h"
#include "metadata.h"
#include "file_dev_ops.h"


static enum RET_CODES parse_fat(const int fd)
{
	char label[8];
	uint32_t magic_mark;

	CHECK_ERROR_GENERIC(read(fd, &magic_mark, LABEL_OFFSET), ssize_t, FAIL_READ);

    magic_mark = BSWAP32_ONLY_ON_LE(magic_mark);
	if (
		((magic_mark & 0xFFFFFF00u) != FAT_MAGIC_1)
		&& ((magic_mark & 0xFFFFFF00u) != FAT_MAGIC_2)
		&& ((magic_mark & 0xFFFFFF00u) != FAT_MAGIC_3)
	) {
		printf("'NO FAT MAGIC FOUND'");
		return FAIL_FAIL;
	}

	CHECK_ERROR_GENERIC(read(fd, &label, sizeof(label) / sizeof(label[0]) - 1), ssize_t, FAIL_READ);
	label[7] = '\0';

	printf("'%s'", label);
	return FAIL_SUCC;
}

enum RET_CODES parse_slot(const int fd, unsigned int slot)
{
	const uint64_t offset = slot * MAGIC_OFFSET;
	const uint64_t start_block = offset / 512;
	const uint64_t end_block = start_block + (IMAGE_SIZE / 512);
	printf("%3u - %8" PRIu64 ",%-7" PRIu64 " - 0x%.8" PRIX64 " ", slot, start_block, end_block, offset);

	CHECK_ERROR_GENERIC(lseek(fd, offset, SEEK_SET), off_t, FAIL_LSEEK);
	parse_fat(fd);

	printf(" ");

	/* There is small (98304 bytes) gap among images, so let's use it for metadata */
	CHECK_ERROR_GENERIC(lseek(fd, offset + IMAGE_SIZE, SEEK_SET), off_t, FAIL_LSEEK);
	metadata_parse(fd);
	printf("\n");

    return FAIL_SUCC;
}

static enum RET_CODES metadata_write_helper(const int fd, const unsigned int slot, const char *const metadata_short_label)
{
	struct metadata meta = metadata_init();
    enum RET_CODES rc;

	rc = metadata_set_short_label(metadata_short_label, &meta);
    if (rc != FAIL_SUCC) {
        return rc;
    }

	rc = metadata_write(fd, slot, &meta);
    if (rc != FAIL_SUCC) {
        return rc;
    }

    return FAIL_SUCC;
}

static void usage(void)
{
	fprintf(stderr, "Usage: gordon -d image_file|block_device [-s slot] [-w 'short label'] [-i 'input_file']\n");
}

void help(void)
{
	static const char help[] =
	"Output:\n"
	"\tslot - start sector,end sector - hex offset 'LABEL FROM IMAGE' Metadata short label of image\n"
	"\nMACOS (X) NOTES:\n"
	"\tOn macOS machines, start sector and end sector can be used to "
	"attach part of image file (only image file on filesystem :/) via hdiutil.\n"
	"\n\tExample:\n"
	"\t\t$ hdiutil attach -section 3072,5952 multiple_images_file.img\n"
	"\nLINUX NOTES\n"
	"On linux machine, loopback command can be used to mount single images"
	"\n";

	printf(help);
}

int main(int argc, char **argv)
{
	int opt;
	char *image_name_p = NULL;
	char *src_image_name_p = NULL;
	unsigned int slot = UINT_MAX;
	bool write_mode = false;
	bool write_meta = false;
	char *metadata_short_label = NULL;

	int fd __attribute__ ((__cleanup__(safe_close))) = -1;
	int open_flags;

	uint64_t fd_size = 0;
	unsigned int potential_num_of_drives;
	unsigned int num_of_fdds = GOTEK_MAX_FDDS;

    enum RET_CODES rc;

    if (argc < 2) {
        usage();
        printf("\n");
        help();
        return EXIT_SUCCESS;
    }

	while ((opt = getopt(argc, argv, "s:d:w:i:h")) != -1) {
		switch (opt) {
		case 'w':
			write_mode = true;
			write_meta = true;
			metadata_short_label = optarg;
			break;

		case 'd':
			image_name_p = optarg;
			break;

		case 's': {
			char *endptr = NULL;
			errno = 0;
			slot = (unsigned int) strtoul(optarg, &endptr, 10);
			if (errno != 0 || *endptr != '\0') {
				usage();
                return EXIT_SUCCESS;
			}
			break;
		}

		/* Input image */
		case 'i':
			write_mode = true;
			src_image_name_p = optarg;
			break;

		case 'h':
			help();
            return EXIT_SUCCESS;
			break;

		default: /* '?' */
			usage();
            return EXIT_SUCCESS;
			break;
		}
	}

	if (image_name_p == NULL)
	{
		fprintf(stderr, "You need to provide at least disk/image file with -d disk/image file.\n");
		usage();
        return EXIT_SUCCESS;
	}

	open_flags = write_mode ? O_RDWR | O_SYNC : O_RDONLY;
	open_flags |= ADDITIONAL_OPEN_FLAGS;
	fd = CHECK_ERROR(open(image_name_p, open_flags), FAIL_OPEN);

    rc = get_file_or_device_size(fd, &fd_size);
    if (rc != FAIL_SUCC) {
        return rc;
    }

	potential_num_of_drives = fd_size / MAGIC_OFFSET;
	if (potential_num_of_drives < GOTEK_MAX_FDDS) {
		num_of_fdds = potential_num_of_drives;
	}

	if (slot != UINT_MAX && slot >= num_of_fdds)
	{
		fprintf(stderr, "Maximum number of slots is 999. Allowed values 0 to 999.\n");
		return EXIT_FAILURE;
	}

	printf("Num of slots: 0 - %u\n", num_of_fdds - 1);

	if (slot != UINT_MAX && image_name_p != NULL && src_image_name_p != NULL)
	{
		if (write_meta) {
			rc = metadata_write_helper(fd, slot, metadata_short_label);
            if (rc != FAIL_SUCC) {
                return rc;
            }
		}

		rc = images_put_image_to(fd, slot, src_image_name_p);
        if (rc != FAIL_SUCC) {
            return rc;
        }

		return EXIT_SUCCESS;
	}

	if (slot != UINT_MAX && !write_mode) {
		rc = parse_slot(fd, slot);
        if (rc != FAIL_SUCC) {
            return rc;
        }
		return EXIT_SUCCESS;
	}

	if (write_mode && slot == UINT_MAX)
	{
		fprintf(stderr,"You need to specify short label.\n");
		return EXIT_FAILURE;
	}

	if (write_meta) {
		rc = metadata_write_helper(fd, slot, metadata_short_label);
        if (rc != FAIL_SUCC) {
            return rc;
        }
		return EXIT_SUCCESS;
	}

	for (slot = 0; slot < num_of_fdds; ++slot)
	{
		parse_slot(fd, slot);
	}

	return EXIT_SUCCESS;
}
