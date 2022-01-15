#define _LARGEFILE64_SOURCE
#define _DARWIN_USE_64_BIT_INODE

#define MAGIC_OFFSET 1572864u
#define IMAGE_SIZE 1474560u
#define LABEL_OFFSET 3u
#define FAT_MAGIC_1 0xEB3C9000u
#define FAT_MAGIC_2 0xEB3E9000u
#define GOTEK_MAX_FDDS 1000u

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

static NONNULLARGS void safe_close(int *fd)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wnonnull-compare"
#pragma GCC diagnostic ignored "-Wtautological-pointer-compare"
	if (fd == NULL) { 
		return;
	}
#pragma GCC diagnostic pop

	if (*fd != -1) {
		close(*fd);
	}
}

static bool parse_fat(const int fd, const unsigned int slot)
{
	char label[8];
	uint32_t magic_mark;

	CHECK_ERROR_GENERIC(read(fd, &magic_mark, LABEL_OFFSET), ssize_t, FAIL_READ);

	if (bswap_32(magic_mark) != FAT_MAGIC_1 &&
		bswap_32(magic_mark) != FAT_MAGIC_2
	) {
		printf("'NO FAT MAGIC FOUND'");
		return false;
	}

	CHECK_ERROR_GENERIC(read(fd, &label, sizeof(label) / sizeof(label[0]) - 1), ssize_t, FAIL_READ);
	label[7] = '\0';

	printf("'%s'", label);
	return true;
}

int main(int argc, char **argv)
{
	int fd __attribute__ ((__cleanup__(safe_close))) = -1;
	uint64_t fd_size = 0;
	struct stat fstat_buf;
	unsigned int potential_num_of_drives;
	unsigned int num_of_fdds = GOTEK_MAX_FDDS;

	if (argc < 2 || argv[1] == NULL) {
		return FAIL_ARGS;	
	}

	fd = CHECK_ERROR(open(argv[1], O_RDONLY | O_CLOEXEC | ADDITIONAL_OPEN_FLAGS), FAIL_OPEN);

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

	printf("Num of fdds: %u\n", num_of_fdds);

	for (unsigned int slot = 0; slot < num_of_fdds; ++slot)
	{
		const uint64_t offset = slot * MAGIC_OFFSET;
		printf("0x" UINT64_PRINTF_FORMAT " ", offset);

		CHECK_ERROR_GENERIC(lseek(fd, offset, SEEK_SET), off_t, FAIL_LSEEK);
		parse_fat(fd, slot);

		printf(" ");

		/* There is small (98304 bytes) gap among images, so let's use it to metadata */
		CHECK_ERROR_GENERIC(lseek(fd, offset + IMAGE_SIZE, SEEK_SET), off_t, FAIL_LSEEK);
		metadata_parse(fd, slot);
		printf("\n");
	}

	return EXIT_SUCCESS;
}
