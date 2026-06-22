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

Displays the list of all ESP and XBOOTLDR partitions. Each is also marked
indicating any that are *current* and the *active* ESP used to boot the machine.

Each may have
a mark indicating whether that partition is the current one. The ESP
used by the motherboard to boot the machine is also saved to nvram. 
This partition, the one active at boot time, is also marked.

When machine boots the boot loader on the motherboard boots from an ESP and it saves
that *active* ESP partition into nvram. The *current* or expected ESP
is the one that is designated by the boot order. 

Under normal circumstances and almost always,  *current* and *active* are the same.
See 'Quirky Esp' section that discusses the situation when these differ and
a possible resolution.


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

  and the same when run as non-root user::

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

QUIRKY ESP IN NVRAM
===================

It is possible that the motherboard firmware chooses a different ESP than
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

Also useful to see boot order::

    efibootmgr

and the UUID and PartUUID::

    lsblk -o NAME,FSTYPE,MOUNTPOINTS,PARTUUID,UUID

* Example 4

  Two ESP with mismatched active and current ::

              Device Mount                          (c a) type
      /dev/nvme0n1p1 /efi                           (✔  ) EFI
      /dev/nvme1n1p1 /man/disk2/efi                 (  ✔) EFI


After eveything was done to ensure everything being correct and this *quirk* did not go away
we found that removing the offending nvram variable resolved the issue. After removing
and rebooting the nvram was then updated to the correct value by the motherboard.

To remove the nvram holding the wrong value::

    cd /sys/firmware/efi/efivars/

    echo "Should show the wrong value"
    cat LoaderDevicePartUUID-4a67b082-0a4c-41cf-b6c7-440b29bb8c4f
    
    chattr -i LoaderDevicePartUUID-4a67b082-0a4c-41cf-b6c7-440b29bb8c4f
    rm LoaderDevicePartUUID-4a67b082-0a4c-41cf-b6c7-440b29bb8c4f

Do this at your own risk!

SEE ALSO
========

**sd-boot-efi-tool-update**, **sd-boot-kernel-update**, **sd-boot-find-boot-mounts**,
**kernel-install**

