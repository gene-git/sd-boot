// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * From buffer of text characters
 * return one "line" by over-writing newline in buffer with null
 */
#include <string.h>

char *get_one_line(char **ptr_p) {
    /**
     * Returns pointer to one line at a time from a mutable buffer.
     * Overwrites '\n' with '\0'. i.e. Destructive.
     * Args:
     *  ptr_p - address of pointer that tracks the char after the last newline (or null)
     * Returns:
     *  Pointer to start of current line, or nullptr if done.
     * NB
     *  the string pointed to by *state_p MNUST be null terminated 
     */
    if (*ptr_p == nullptr || (*ptr_p)[0] == '\0') {
        // End of string
        return nullptr; 
    }

    /*
     * find next newline
     */
    char *ptr_0 = *ptr_p;
    *ptr_p = strpbrk(ptr_0, "\n");

    /*
     * over-write newline with null and move ptr to next char.
     */
    if (*ptr_p != nullptr) {
        (*ptr_p)[0] = '\0';
        (*ptr_p)++;
    } 

    /*
     * return start of this line or nullptr
     */
    return ptr_0;
}

