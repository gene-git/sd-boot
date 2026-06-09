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

When machine boots the motherboard boot loader boots from an ESP and marks
that *active* ESP partition in nvram. There is also the intended or expected ESP
as marked in the boot order in nvram. Most of the time these are the same 
but it is possible that the motherboard firmware chooses a different ESP than
that defined by nvram boot order. When this happens these 2 ESP partition
do not match. 

Running *bootctl* when they are mismatched will display::

    WARNING: The boot loader reports a different partition UUID than the detected ESP.

We designate partitions determinted by boot nvram as *active* and expected partitions as 
as *current*.

Please note that root privilages are required to check for a current XBOOTLDR partitions.
No elevated priveleges are otherwise needed.

To manually check for any current ESP and XBOOTLDDR partitions::

    bootctl | grep -E 'ESP:|XBOOTLDR:'

And to list all active ESP and XBOOTLDR partitions::

    fdisk -l | grep -E 'EFI System|Linux extended boot'

ARGUMENTS
=========

``none``

EXAMPLES
========

* Example 1

  One ESP::

                                                    (c)urrent (a)ctive
              Device Mount                          (c a) type
      /dev/nvme0n1p1 /boot                          (✔ ✔) EFI

*  Example 2:

   One ESP and one XBOOTLDR::


              Device Mount                          (c a) type
           /dev/sda1 /efi                           (✔ ✔) EFI
           /dev/sda5 /boot                          (✔ ✔) XBOOTLDR

  Same when run as non-root user::

              Device Mount                          (c a) type
           /dev/sda1 /efi                           (✔ ✔) EFI
           /dev/sda5 /boot                          (  ✔) XBOOTLDR

* Example 3

  Two ESP and two XBOOTLDR::

              Device Mount                          (c a) type
      /dev/nvme0n1p1 /efi                           (✔ ✔) EFI
           /dev/sdg1 /mnt/root1/efi                 (   ) EFI
      /dev/nvme0n1p2 /boot                          (✔ ✔) XBOOTLDR
           /dev/sdg5 /mnt/root1/boot                (   ) XBOOTLDR

When run as non-root user, any current XBOOTLDR remains unknown/unmarked.

SEE ALSO
========

**sd-boot-efi-tool-update**, **sd-boot-kernel-update**, **sd-boot-find-boot-mounts**,
**kernel-install**

