// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Run executable and display it's stdout on current process stdout.
 * Allow output to be shifted right to faciliate formattig.
 * Ouptut is only shown at verb level 2 or higher.
 * Variation of run_cmd(), run_cms_output()
 */
#include "sd-boot-cmd.h"
#include "sd-boot-msg.h"
#include <stdint.h>
#include <stdlib.h>


int run_cmd_output_show(char **argv, char **envp, int *child_ret_p, uint16_t margin, int line_len) {
    int ret = 0;
    char *output = nullptr;

    ret = run_cmd_output(argv, envp, &output, child_ret_p);
    if (ret != 0) {
        goto exit;
    }

    if (!output || output[0] == 0) {
        goto exit;
    }

    msg_buf(MSG_NORMAL, margin, line_len, output);

exit:
    if (output) {
        free((void *)output);
    }
    return ret;
}
