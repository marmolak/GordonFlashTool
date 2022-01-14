#define _LARGEFILE64_SOURCE
#define _DARWIN_USE_64_BIT_INODE

#define MAGIC_OFFSET 1572864u
#define LABEL_OFFSET 3u
#define FAT_MAGIC_1 0xEB3C9000u
#define FAT_MAGIC_2 0xEB3E9000u
#define GOTEK_MAX_FDDS 1000u

#define NONNULLARGS __attribute__((nonnull)) 

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

enum RET_CODES
{
	FAIL_ARGS = 1,
	FAIL_OPEN = 2,
	FAIL_LSEEK = 10,
	FAIL_READ = 11,
};

/* Common parts */

#define SAFE_CLOSE(fd) do { 				\
	if (fd != -1) {							\
		close(fd);							\
		fd = -1;							\
	}										\
} while(0)

#define CHECK_ERROR_GENERIC(f, t, e) ({		\
	typeof(t) rc = (f); 					\
	if (rc == -1) {							\
		perror(__func__);					\
		exit((e));							\
	}										\
	rc;										\
})

#define CHECK_ERROR(f, e) CHECK_ERROR_GENERIC(f, int, e)

#define CHECK_ERROR_P(f, e) ({ 				\
	void *rc_p = (f); 						\
	if (rc_p == NULL) {						\
		perror(__func__);					\
		exit((e));							\
	}										\
	rc_p;									\
})	

/* Compatibility layer */
#if defined(__APPLE__) && defined(__MACH__)
#include <sys/disk.h>

#define ADDITIONAL_OPEN_FLAGS 0

#define UINT64_PRINTF_FORMAT "%.8llx"

#define bswap_16(value) \
((((value) & 0xff) << 8) | ((value) >> 8))

#define bswap_32(value) \
(((uint32_t)bswap_16((uint16_t)((value) & 0xffff)) << 16) | \
(uint32_t)bswap_16((uint16_t)((value) >> 16)))

void blkgetsize(int fd, uint64_t *psize)
{
	unsigned long blocksize = 0;
	unsigned long nblocks;

	CHECK_ERROR(ioctl(fd, DKIOCGETBLOCKSIZE, &blocksize), 7);
	CHECK_ERROR(ioctl(fd, DKIOCGETBLOCKCOUNT, &nblocks), 7);
	*psize = (uint64_t) nblocks * blocksize;
}

#elif defined(__linux__)
#include <linux/fs.h>
#include <byteswap.h>

#define ADDITIONAL_OPEN_FLAGS O_LARGEFILE

#define UINT64_PRINTF_FORMAT "%.8lx"

void blkgetsize(int fd, uint64_t *psize)
{
#ifdef BLKGETSIZE64
	CHECK_ERROR(ioctl(fd, BLKGETSIZE64, psize), 7);
#elif BLKGETSIZE
	unsigned long sectors = 0;
	CHECK_ERROR(ioctl(fd, BLKGETSIZE, &sectors), 7);
  	*psize = sectors * 512ULL;
#else
# error "Linux configuration error (blkgetsize)"
#endif
}

#else
#error "Unsupported platform"
#endif

static NONNULLARGS void safe_close(int *fd)
{
	if (fd == NULL) { 
		return;
	}

	if (*fd != -1) {
		close(*fd);
	}
}

static bool parse_fat(const int fd, const unsigned int slot)
{
	char label[8];
	uint32_t magic_mark;
	const uint64_t offset = slot * MAGIC_OFFSET;

	CHECK_ERROR_GENERIC(lseek(fd, offset, SEEK_SET), off_t, FAIL_LSEEK);
	CHECK_ERROR_GENERIC(read(fd, &magic_mark, LABEL_OFFSET), ssize_t, FAIL_READ);

	if (bswap_32(magic_mark) != FAT_MAGIC_1 &&
		bswap_32(magic_mark) != FAT_MAGIC_2
	) {
		printf("NO FAT MAGIC FOUND - 0x" UINT64_PRINTF_FORMAT "\n", offset);
		/* TODO: Add new parser */
		return false;
	}

	CHECK_ERROR_GENERIC(read(fd, &label, sizeof(label) / sizeof(label[0]) - 1), ssize_t, FAIL_READ);
	label[7] = '\0';

	printf("%s - 0x" UINT64_PRINTF_FORMAT "\n", label, offset);
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
		parse_fat(fd, slot);
	}

	return EXIT_SUCCESS;
}