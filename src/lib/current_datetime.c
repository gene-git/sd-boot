// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Current Date Time String: 
 * Format: ""%Y-%m-%d %H:%M:%S" ~ "YYYY-MM-DD HH:MM:SS"
 * returns 0 on success.
 */
#include <stdio.h>
#include <time.h>

int current_datetime_str(size_t buflen, char *buf) {

    if (!buf || buflen == 0) {
        return -1;
    }

    time_t unix_secs = time(nullptr);
    if (unix_secs == (time_t)-1) {
        return -1;
    }

    struct tm tm_info;
    if (!localtime_r(&unix_secs, &tm_info)) {
        return -1;
    }

    const char format[] = "%Y-%m-%d %H:%M:%S";
    size_t bytes_written = strftime(buf, buflen, format, &tm_info);

    if (bytes_written == 0) {
        return -1;
    }

    return 0;
}
