#define _LARGEFILE64_SOURCE
#define _DARWIN_USE_64_BIT_INODE

#include <errno.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <stdbool.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

#include "common.h"
#include "metadata.h"

/* Compatibility layer */
#if defined(__APPLE__) && defined(__MACH__)
#include <sys/disk.h>

#define ADDITIONAL_OPEN_FLAGS 0

void blkgetsize(int fd, uint64_t *psize)
{
	unsigned long blocksize = 0;
	unsigned long nblocks;

	CHECK_ERROR(ioctl(fd, DKIOCGETBLOCKSIZE, &blocksize), FAIL_IOCTL);
	CHECK_ERROR(ioctl(fd, DKIOCGETBLOCKCOUNT, &nblocks), FAIL_IOCTL);
	*psize = (uint64_t) nblocks * blocksize;
}

#elif defined(__linux__)
#include <linux/fs.h>

#define ADDITIONAL_OPEN_FLAGS O_LARGEFILE

void blkgetsize(int fd, uint64_t *psize)
{
#ifdef BLKGETSIZE64
	CHECK_ERROR(ioctl(fd, BLKGETSIZE64, psize), FAIL_IOCTL);
#elif BLKGETSIZE
	unsigned long sectors = 0;
	CHECK_ERROR(ioctl(fd, BLKGETSIZE, &sectors), FAIL_IOCTL);
	*psize = sectors * 512ULL;
#else
# error "Linux configuration error (blkgetsize)"
#endif
}

#else
#error "Unsupported platform."
#endif

static void safe_close(int *const fd)
{
	if (fd == NULL) { 
		return;
	}

	if (*fd != -1) {
		CHECK_ERROR(close(*fd), FAIL_CLOSE);
		*fd = -1;
	}
}

static bool parse_fat(const int fd)
{
	char label[8];
	uint32_t magic_mark;

	CHECK_ERROR_GENERIC(read(fd, &magic_mark, LABEL_OFFSET), ssize_t, FAIL_READ);

	if (bswap_32(magic_mark & 0x00FFFFFF) != FAT_MAGIC_1 &&
		bswap_32(magic_mark & 0x00FFFFFF) != FAT_MAGIC_2
	) {
		printf("'NO FAT MAGIC FOUND'");
		return false;
	}

	CHECK_ERROR_GENERIC(read(fd, &label, sizeof(label) / sizeof(label[0]) - 1), ssize_t, FAIL_READ);
	label[7] = '\0';

	printf("'%s'", label);
	return true;
}

static void parse_slot(const int fd, unsigned int slot)
{
	const uint64_t offset = slot * MAGIC_OFFSET;
	printf("%3u - 0x" UINT64_PRINTF_FORMAT " ", slot, offset);

	CHECK_ERROR_GENERIC(lseek(fd, offset, SEEK_SET), off_t, FAIL_LSEEK);
	parse_fat(fd);

	printf(" ");

	/* There is small (98304 bytes) gap among images, so let's use it for metadata */
	CHECK_ERROR_GENERIC(lseek(fd, offset + IMAGE_SIZE, SEEK_SET), off_t, FAIL_LSEEK);
	metadata_parse(fd);
	printf("\n");
}

static __attribute__((noreturn)) void usage_exit(void) 
{
	fprintf(stderr, "Usage: gordon [-d image_file|drive] [-s slot] [-l]\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	int opt;
	char *image_name_a = NULL;
	unsigned int slot = UINT_MAX;
	bool write_mode = false;
	char *metadata_short_label = NULL;

	int fd __attribute__ ((__cleanup__(safe_close))) = -1;

	int open_flags;
	uint64_t fd_size = 0;
	struct stat fstat_buf;
	unsigned int potential_num_of_drives;
	unsigned int num_of_fdds = GOTEK_MAX_FDDS;

	/* -d - drive
	   -s - slot
	*/

	while ((opt = getopt(argc, argv, "s:d:w:")) != -1) {
		switch (opt) {
		case 'w':
			write_mode = true;
			metadata_short_label = optarg;
			break;
		case 'd':
			image_name_a = optarg;
			break;
		case 's': {
			char *endptr = NULL;
			errno = 0;
			slot = (unsigned int) strtoul(optarg, &endptr, 10);
			if (errno != 0 || *endptr != '\0') {
				usage_exit();
			}
			break;
		}
		default: /* '?' */
			usage_exit();
		}
	}

	if (image_name_a == NULL)
	{
		fprintf(stderr, "You needd to provide at least disk/image file with -d disk|image file.\n");
		usage_exit();
	}

	open_flags = write_mode ? O_RDWR | O_SYNC : O_RDONLY;
	open_flags |= ADDITIONAL_OPEN_FLAGS;
	fd = CHECK_ERROR(open(image_name_a, open_flags), FAIL_OPEN);

	CHECK_ERROR(fstat(fd, &fstat_buf), 3);

	if ((fstat_buf.st_mode & S_IFBLK) == S_IFBLK)
	{
		blkgetsize(fd, &fd_size);
	} else {
		fd_size = fstat_buf.st_size;
	}

	potential_num_of_drives = fd_size / MAGIC_OFFSET;
	if (potential_num_of_drives < GOTEK_MAX_FDDS) {
		num_of_fdds = potential_num_of_drives;
	}

	if (slot != UINT_MAX && slot >= num_of_fdds)
	{
		fprintf(stderr, "Maximum number of slots is 999. Allowed values 0 to 999.\n");
		exit(EXIT_FAILURE);
	}

	printf("Num of slots: 0 - %u\n", num_of_fdds - 1);

	if (slot != UINT_MAX && !write_mode) {
		parse_slot(fd, slot);
		exit(EXIT_SUCCESS);
	}

	if (write_mode && slot == UINT_MAX)
	{
		fprintf(stderr,"You need to specify short label.\n");
		exit(EXIT_FAILURE);
	}

	if (write_mode) {
		struct metadata meta = metadata_init();

		metadata_set_short_label(metadata_short_label, &meta);
		metadata_write(fd, &meta, slot);
		exit(EXIT_SUCCESS);
	}

	for (slot = 0; slot < num_of_fdds; ++slot)
	{
		parse_slot(fd, slot);
	}

	return EXIT_SUCCESS;
}
