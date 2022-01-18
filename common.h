#ifndef COMMON_H
#define COMMON_H

#define _LARGEFILE64_SOURCE
#define _DARWIN_USE_64_BIT_INODE

#include <stdlib.h>
#include <fcntl.h>

#define MAGIC_OFFSET 1572864u
#define IMAGE_SIZE 1474560u
#define LABEL_OFFSET 3u
#define IMAGES_GAP 98304

#define FAT_MAGIC_1 0xEB3C9000u
#define FAT_MAGIC_2 0xEB3E9000u
#define GOTEK_MAX_FDDS 1000u

#define NONNULLARGS __attribute__((nonnull)) 

enum RET_CODES
{
	FAIL_ARGS = 1,
	FAIL_OPEN = 2,
	FAIL_LSEEK = 10,
	FAIL_READ = 11,
	FAIL_WRITE = 12,
	FAIL_CLOSE = 13,
	FAIL_IOCTL = 20,
};

#define CHECK_ERROR_GENERIC(f, t, e) ({ \
    __typeof__(t) rc = (f);             \
    if (rc == -1) {                     \
        perror(__func__);               \
        exit((e));                      \
    }                                   \
    rc;                                 \
})

#define CHECK_ERROR(f, e) CHECK_ERROR_GENERIC(f, int, e)

#define CHECK_ERROR_P(f, e) ({  \
    void *rc_p = (f);           \
    if (rc_p == NULL) {         \
        perror(__func__);       \
        exit((e));              \
    }                           \
    rc_p;                       \
})

/* Functions */
void safe_close(int *const fd);

/* Compatibility */
#if defined(__APPLE__) && defined(__MACH__)

#define UINT64_PRINTF_FORMAT "%.8llx"

#define ADDITIONAL_OPEN_FLAGS 0

#if defined(__BIG_ENDIAN__)

#define bswap_16_depends(value) (value)
#define bswap_32_depends(value) (value)

#else

#define bswap_16_depends(value) \
((((value) & 0xff) << 8) | ((value) >> 8))

#define bswap_32_depends(value) \
(((uint32_t)bswap_16_depends((uint16_t)((value) & 0xffff)) << 16) | \
(uint32_t)bswap_16_depends((uint16_t)((value) >> 16)))

#endif

#elif defined(__linux__)

#include <endian.h>

#if BYTE_ORDER == LITTLE_ENDIAN

#define bswap_16_depends(value) (htobe16(value))
#define bswap_32_depends(value) (htobe32(value))

#else

#define bswap_16_depends(value) (value)
#define bswap_32_depends(value) (value)

#endif

#define UINT64_PRINTF_FORMAT "%.8lx"

#define ADDITIONAL_OPEN_FLAGS O_LARGEFILE

#endif /* MACOS and Linux checks */

#endif /* COMMON_H */
