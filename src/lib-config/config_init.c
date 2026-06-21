// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Development Set Up.
 * - Supports non-root testing in directory (Testing/__root__/)
 * - Activated by non-root user and env variable set:
 *   SDB_DEV_TEST
 *
 */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sd-boot-config.h"
#include "sd-boot-msg.h"

/*
 * Initialize config and dev setup when SDB_DEV_TEST is set.
 * Note that dev test requires:
 * - non-root user.
 * - we use secure_getenv() 
 *
 * Normal mode:
 * - conf->root = "/"
 *
 * Dev test mode:
 * - conf->root = "Testing/__root__/"
 * - testing tree is initized.
 *
 * The working directory is always conf->root.
 */
int config_init(SdBoot *conf) {

    conf->euid = geteuid();

    char *dev_test_env = secure_getenv("SDB_DEV_TEST");
    if (dev_test_env) {

        conf->test = true;
        if (config_test_init(conf) != 0) {
            return -1;
        }

    } else {

        conf->test = false;
        conf->test_root = nullptr;
        conf->root = strdup("/");
        if (!conf->root) {
            msg(MSG_ERR, "  ! Error: mem allocation\n");
        }
    }

    return 0;
}
