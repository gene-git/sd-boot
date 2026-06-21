// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * sd-boot.h
 */
#ifndef SD_BOOT_KERNEL_H
#define SD_BOOT_KERNEL_H

#include <stdbool.h>
#include <stddef.h>
//#include <sys/stat.h>

#include "sd-boot-config.h"
#include "sd-boot-utils.h"
#include "sd-boot.h"

/*
 *
 * Kernel Module info
 * Derived from kernel module directory
 * mod_dir = /usr/lib/modules/<kern-vers>
 * Files in mod_dir : sdb-pkgbase, pkgbase
 * package name is read from sdb-pkgbase or pkgbase
 *
 * kmod_dir =>
 * - package (from sdb-packbase, pkgbase)
 * - kern_vers (from mod_dir)
 * Note: plugins use KIplgiin which gets kern_vers/kern_image from by kernel-install.
 */
typedef struct {
    char *image;
    char *mod_dir;
    char *package;
    char *vers;

} KernelInfo; 

/*
 *
 * Kernel Trigger Info
 *
 * triggers are pathname or package names passed in
 * from ALPM hooks - each is provided as one line
 * on stdin.
 */
typedef struct {
    size_t num_info;
    size_t num_info_alloc;
    KernelInfo *info;

    size_t num_other;

} Triggers;


/*
 * Function declarations
 */
int is_kernel_image_path(char *image, bool *is_kernel);
int kernel_add_remove(SdBoot *conf, Array_str *pkgs_arr, KernelInfo *info);
int kernel_info_all(size_t *num_info_p, KernelInfo **info_p);
int kernel_image_path_to_info(KernelInfo *info);
void kernel_info_free(KernelInfo *info);
int kernel_inspect(SdBoot *conf, Array_str *pkgs_arr, KernelInfo *info);
int kernel_pkg_to_info(const char *pkg, KernelInfo *info);
int get_kernel_triggers(Triggers *trigs);
void free_triggers(Triggers *trigs);
int loaderentry_modify_kernel(SdBoot *conf, KIplugin *plugin);
bool is_kernel_sd_boot_managed(Array_str *pkgs_arr, KernelInfo *info);
int load_managed_kernel_packages(SdBoot *conf, Array_str *arr);
int parse_kernel_triggers(Array_str *trigs_all, Triggers *trigs);

#endif
