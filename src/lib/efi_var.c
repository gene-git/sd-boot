// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Read EFI variable from firmware and convert the UTF-16 data into 
 * ascii characters
 *
 * efi paths are the ones provided by kernel:
 *  "/sys/firmware/efi/efivars/<variable-name>"
 */
/*
 * convert the utf16 buffer to ascii
 * - keep printable  (32 to 128)
 * - drops newlines as well (ascii 10)
 * - buffer end is signalled by utf16 null 0x0000
 */
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "sd-boot.h"

#define IS_PRINT(c) ((c) >= 32 && (c) <= 128)

enum Constants {
    HDR_LEN = 4,
    VAR_MAX = (64 * 1024),
    VAR_LEN_MAX = (8 * 1024),
};


/* 
 * Convert utf16 to ascii
 * - eof is utf16 null (0x0000)
 */
static void utf16_to_ascii(const uint16_t *buf, size_t buflen, char *text) {

    size_t cnt = 0;
    for (size_t i = 0; i < buflen; i++) {
        if (buf[i] == 0) {
            break;
        }
        if (IS_PRINT(buf[i])) {
            const char clower = (char)tolower((int)(unsigned char)buf[i]);
            text[cnt++] = clower;
        }
    }
    text[cnt] = '\0';
}


/*
 * Return allocated string that caller responsible for freeing.
 * Returns nullptr if 
 */
char *read_efi_var_string(const char *efi_path) {
    int ret = 0;
    char *efi_var = nullptr;
    uint16_t *buf = nullptr;
    size_t num_read = 0;
    FILE *fptr = nullptr;

    if (!efi_path) {
        return nullptr;
    }

    /*
     * file size
     */
    struct stat st_buf = {};
    if (stat(efi_path, &st_buf) != 0) {
        return nullptr;
    }

    size_t f_size = st_buf.st_size;
    if (f_size <= 4) {
        return nullptr;
    }

    fptr = fopen(efi_path, "rb");
    if (!fptr) {
        return nullptr;
    }

    /*
     * Skip 4 byte UEFI header
     * - NON_VOLATILE
     * - BOOTSERVICE_ACCESS
     * - RUNTIME_ACCESS
     * - HARDWARE_ERROR_RECORD
     */
    unsigned char header[4] = {};
    if (fread(header, 1, HDR_LEN, fptr) != 4) {
        goto exit;
    }
    f_size -= 4;

    /*
     * read data
     * - bound check since efi_path is provided by caller
     */
    if (f_size > VAR_MAX) {
        msg(MSG_ERR, "  sd-boot: EFI variable data too big.\n");
        goto exit;
    }

    size_t num_elems = (f_size + sizeof(uint16_t) - 1) / sizeof(uint16_t);

    buf = (uint16_t *)malloc(num_elems * sizeof(uint16_t));
    if (!buf) {
        goto exit;
    }

    num_read = fread((void *)buf, sizeof(uint16_t), num_elems, fptr);

    ret = fclose(fptr);
    fptr = nullptr;
    if (ret != 0) {
        goto exit;
    }

    if (num_read > VAR_LEN_MAX) {
        msg(MSG_ERR, "  sd-boot: EFI variable data too big.\n");
        goto exit;
    }

    efi_var = (char *)malloc((num_read + 1) * sizeof(char));
    if (!efi_var) {
        goto exit;
    }
    utf16_to_ascii((const uint16_t *)buf, num_read, efi_var);

exit:
    if (fptr) {
        (void)fclose(fptr);
    }
    if (buf) {
        free((void *)buf);
    }
    return efi_var;
}
