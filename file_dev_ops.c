#include <sys/stat.h>
#include <stdio.h>
#include <stdint.h>

#include "file_dev_ops.h"
#include "common.h"

/* Compatibility layer */
#if defined(__APPLE__) && defined(__MACH__)
#include <sys/disk.h>

enum RET_CODES blkgetsize(int fd, uint64_t *psize)
{
    uint32_t blocksize = 0;
    uint64_t nblocks;

	CHECK_ERROR(ioctl(fd, DKIOCGETBLOCKSIZE, &blocksize), FAIL_IOCTL);
	CHECK_ERROR(ioctl(fd, DKIOCGETBLOCKCOUNT, &nblocks), FAIL_IOCTL);
	*psize = (uint64_t) nblocks * blocksize;

    return FAIL_SUCC;
}

#elif defined(__linux__)
#include <linux/fs.h>

enum RET_CODES  blkgetsize(int fd, uint64_t *psize)
{
#ifdef BLKGETSIZE64
	CHECK_ERROR(ioctl(fd, BLKGETSIZE64, psize), FAIL_IOCTL);
#elif BLKGETSIZE
	unsigned long sectors = 0;
	CHECK_ERROR(ioctl(fd, BLKGETSIZE, &sectors), FAIL_IOCTL);
	*psize = sectors * 512ULL;

    return FAIL_SUCC;
#else
# error "Linux configuration error (blkgetsize)"
#endif
}

#else
#error "Unsupported platform."
#endif

enum RET_CODES get_file_or_device_size(int fd, uint64_t *const fd_size)
{
    struct stat fstat_buf;
    enum RET_CODES rc;

    CHECK_ERROR(fstat(fd, &fstat_buf), FAIL_FSTAT);

	if (S_ISCHR(fstat_buf.st_mode))
	{
		fprintf(stderr, "Special character device is not supported. Probably you want to use /dev/rdisk device on MacOS?\n");
		return FAIL_CHRNOTSUPP;
	}

	if (S_ISBLK(fstat_buf.st_mode))
	{
		rc = blkgetsize(fd, fd_size);
        return rc;
	} else {
		*fd_size = fstat_buf.st_size;
    }

    return FAIL_SUCC;
}