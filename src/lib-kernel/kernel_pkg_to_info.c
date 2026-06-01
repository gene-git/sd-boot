// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Given package name find the kernel info.
 * Return allocated KernelInfo if found, else nullptr.
 *
 * NB. the package must be installed by pacman is the usual way.
 */
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"

static void clean_up(size_t num, KernelInfo **infos_p) {
    if (infos_p == nullptr || num == 0) {
        return;
    }

    for (size_t i = 0; i < num; i++) {
        kernel_info_free(&(*infos_p)[i]);
    }
    free((void *)*infos_p);
}

/*
 * info is empty. All fields are filled here. 
 */
int kernel_pkg_to_info(const char *pkg, KernelInfo *info) {
    int ret = 0;
    size_t num_infos = 0;
    KernelInfo *infos = nullptr;

    info->image = nullptr;
    info->mod_dir = nullptr;
    info->package = nullptr;
    info->vers = nullptr;

    if (pkg == nullptr) {
        return 0;
    }

    /*
     * all installed kernels
     */
    ret = kernel_info_all(&num_infos, &infos);
    if (ret != 0) {
        goto exit;
    }
    for (size_t i = 0; i < num_infos; i++) {
        if (infos[i].package != nullptr) {
            if (strcmp(infos[i].package, pkg) == 0) {

                info->image = infos[i].image;
                infos[i].image = nullptr;

                info->mod_dir = infos[i].mod_dir;
                infos[i].mod_dir = nullptr;

                info->package = infos[i].package;
                infos[i].package = nullptr;

                info->vers = infos[i].vers;
                infos[i].vers = nullptr;
                break;
            }
        }
    }

exit:
    clean_up(num_infos, &infos);
    return ret;
}

