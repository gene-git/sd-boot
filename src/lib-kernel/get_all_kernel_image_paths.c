// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Get list of every kernel image path:
 *
 * "/usr/lib/modules/<kern-vers>/vmlinuz"
 */

#include "sd-boot-utils.h"


int get_all_kernel_image_paths(Array_str *image_paths) {
    int ret = 0;
    const char *ki_image_pattern = "/usr/lib/modules/*/vmlinuz";

    ret = file_list_glob(ki_image_pattern, image_paths);
    if (ret != 0) {
        goto exit;
    }

exit:
    return ret;
}

