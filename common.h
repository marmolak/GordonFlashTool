#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>

#define NONNULLARGS __attribute__((nonnull)) 

enum RET_CODES
{
	FAIL_ARGS = 1,
	FAIL_OPEN = 2,
	FAIL_LSEEK = 10,
	FAIL_READ = 11,
	FAIL_IOCTL = 20,
};

#define SAFE_CLOSE(fd) do { \
    if (fd != -1) {         \
        close(fd);          \
        fd = -1;            \
    }                       \
} while(0)

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

#if defined(__APPLE__) && defined(__MACH__)

#define UINT64_PRINTF_FORMAT "%.8llx"

#define bswap_16(value) \
((((value) & 0xff) << 8) | ((value) >> 8))

#define bswap_32(value) \
(((uint32_t)bswap_16((uint16_t)((value) & 0xffff)) << 16) | \
(uint32_t)bswap_16((uint16_t)((value) >> 16)))

#elif defined(__linux__)

#include <byteswap.h>

#define UINT64_PRINTF_FORMAT "%.8lx"

#endif /* MACOS and Linux checks */

#endif /* COMMON_H */