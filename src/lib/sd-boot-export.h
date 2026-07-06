// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 *  sd-boot-export.h
 *  Build with meson -Dhardened_export=true (-fvisibility=hidden)
 */
#ifndef SD_BOOT_EXPORT_H
#define SD_BOOT_EXPORT_H

#if defined(__GNUC__) || defined(__clang__)
#define SD_BOOT_EXPORT __attribute__((visibility("default")))
#else
#define SD_BOOT_EXPORT
#endif

#endif
