// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
  * @brief Write non-null data to a file.
  *
  * If data is null, nothing is written and 0 is returned.
  *
  * @param data     The data to be written.
  * @param flags    Flags passed to open().
  * @param mode     Mode passed into open().
  *
  *
  * @return         Success will return 0
  * @retval         0 = success
  * @retval         -1 = error
  */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "sd-boot-msg.h"
#include "sd-boot-utils.h"

/*
 * Write can be partial.
 * Subsequent writes may (or may not) succed for remaining data.
 */
static int write_with_retry(int fdes, const char *data, size_t data_size) {
    size_t total = 0;

    while (total < data_size) {
        const char *current_data = data + total;
        size_t remaining = data_size - total;

        ssize_t written = write(fdes, current_data, remaining);

        if (written < 0) {
            if (errno == EINTR) {       // NOLINT(misc-include-cleaner)
                continue;
            }
            return -1;
        }

        total += (size_t)written;

    }
    return 0;
}

int write_file(const char *data, size_t data_size, int flags, mode_t mode, const char *dest_path) {

    if (!data) {
        return 0;
    }

    int fdes = open(dest_path, flags, mode);
    if (fdes < 0) {
        perror("file open failes");
        msg(MSG_ERR, "!  Error opening %s\n", dest_path);
        return -1;
    }

    int ret = write_with_retry(fdes, data, data_size);
    if (ret < 0) {
        perror("file write failed");
        msg(MSG_ERR, "!  Error writing %s\n", dest_path);
        goto exit;
    }

exit:
    if (fdes >= 0) {
        (void)close(fdes);
    }
    return ret;
}

