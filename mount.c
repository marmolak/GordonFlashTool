#include "mount.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <inttypes.h>


#if defined(__APPLE__) && defined(__MACH__)

/* Just call hdiutil which handles heavy lifting. */
static enum RET_CODES mount_slot_os_x(int fd, const unsigned int slot, const char *const image_name)
{
    struct stat fstat_buf;
    int ret;
    char buffer[1024] = { '\0' };

    const uint64_t offset = slot * MAGIC_OFFSET;
    const uint64_t start_block = offset / 512;
    const uint64_t end_block = start_block + (IMAGE_SIZE / 512);

    CHECK_ERROR(fstat(fd, &fstat_buf), FAIL_FSTAT);

    if (!S_ISREG(fstat_buf.st_mode)) {
        fprintf(stderr, "Only image files are supported on macOS/Mac OS X.\n");
        return FAIL_NOTSUPP;
    }
    /* Don't allow fd leak. If execl fails, then descriptor will be closed. */
    close(fd);

    snprintf(buffer, sizeof(buffer), "%" PRIu64 ",%" PRIu64, start_block, end_block);

    ret = execl("/usr/bin/hdiutil", "/usr/bin/hdiutil", "attach", "-section", buffer, image_name,(char*) NULL);

    if (ret == -1) {
        fprintf(stderr, "Unable to execute hdiutil.\n");
        return FAIL_FAIL;
    }

    return FAIL_FAIL;
}
#endif

enum RET_CODES mount_slot(int fd, const unsigned int slot, const char *const image_name)
{
    enum RET_CODES rc;
#if defined(__APPLE__) && defined(__MACH__)
    rc = mount_slot_os_x(fd, slot, image_name);
    return rc;
#endif

    fprintf(stderr, "Not supported on your OS.\n");
    return FAIL_NOTSUPP;
}