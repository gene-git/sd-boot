.. SPDX-License-Identifier: GPL-2.0-or-later
.. SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>

========================
sd-boot-find-boot-mounts
========================

-----------------------------------------------
Display ESP and XBOOTLDR partition mount points
-----------------------------------------------

:Author: Gene C <arch@sapience.com>
:Manual section: 8 
:Manual group: Linux Tools
:Date: @DATE@
:Version: @VERSION@

SYNOPSIS
========

``sd-boot-find-boot-mounts``

DESCRIPTION
===========

Displays the list of all ESP and XBOOTLDR partitions.
Those used in the currently booted system are marked with an asterisk, which 
is helpful when there is more than one ESP or XBOOTLDR partition.

This must be run as root since partition probing requires elevated privileges.

ARGUMENTS
=========

``none``

EXAMPLES
========

Running *sd-boot-find-boot-mounts* on a system with 2 of each partition type displays::

    EFI:
	    * /efi
	    /mnt/root1/efi
    XBOOTLDR:
	    * /boot
	    /mnt/root1/boot

SEE ALSO
========

**sd-boot-efi-tool-update**, **sd-boot-kernel-update**, **sd-boot-find-boot-mounts**,
**kernel-install**

