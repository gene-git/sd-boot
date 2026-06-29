// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Initialize kernel install's operation.
 *
 * Set tool->conf.oper from input string
 *
 * e.g.
 *  "add" -> KI_ADD
 *  "remove" -> KI_REMOVE
 * etc
 */
#include <string.h>

#include "sd-boot-msg.h"
#include "sd-boot-config.h"
#include "sd-boot.h"


int init_oper(char *oper_str, Tool *tool) {
    int ret = 0;
    
    tool->conf.oper = KI_BAD;
    tool->conf.oper_str = strdup(oper_str);
    if (!tool->conf.oper_str) {
        msg(MSG_ERR, "! sd-boot: mem allocation error %s\n", oper_str);
        ret = 1;
        goto exit;
    }   
    
    tool->conf.oper = ki_operation((const char *)tool->conf.oper_str);
    if (tool->conf.oper == KI_BAD) {
        msg(MSG_ERR, "! sd-boot: unknown operation:  %s\n", tool->conf.oper_str);
        ret = 1;
        goto exit;
    }
        
exit:   
    return ret;
}

