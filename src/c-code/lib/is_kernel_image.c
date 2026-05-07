// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Check if an image path is for any kernel.
 *
 * If image ~ "/usr/lib/modules/<kern-vers>/vmlinux
 */
#include <regex.h>
#include <stdbool.h>

int is_kernel_image_path(char *image, bool *is_kernel) {
    int ret = 0;
    regex_t regex = {};
    char *pat = "^.*usr/lib/modules/[^/]+/vmlinuz$";

    *is_kernel = false;
    if (image == nullptr || image[0] == '\0') {
        goto exit;
    }

    ret = regcomp(&regex, pat, REG_EXTENDED);
    if (ret != 0) {
        ret = -1;
        goto exit;
    }
    int check = regexec(&regex, image, 0, nullptr, 0);
    if (check == 0) {
        *is_kernel = true;
    }

exit:
    regfree(&regex);
    return ret;
}
