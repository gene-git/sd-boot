.. SPDX-License-Identifier: GPL-2.0-or-later
.. SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>

====================
sd-boot-efifs-update 
====================

-----------------------------------------------
Install or remove efi filesystem drivers to ESP
-----------------------------------------------

:Author: Gene C <arch@sapience.com>
:Date: @DATE@
:Manual section: 8 
:Manual group: Linux Tools
:Version: @VERSION@

SYNOPSIS
========

``sd-boot-efifs-update`` ``add`` | ``remove``

DESCRIPTION
===========

Install or remove efi filesystem drivers provided by *efifs* package to the *EFI* partition. 
The driver files are copied from /usr/lib/efifs-x64.

ARGUMENTS
=========

``add``

    Copies the efi drivers provided by the package and installs them 
    to the EFI partition.

``remove``

    Remove the drivers from the EFI partition.

FILES
=====

``/usr/lib/sd-boot/sd-boot-efifs-update``

    Location of the executable.

``<EFI>/EFI/systemd/drivers/``

    The efi partition where the drivers are installed. *<EFI>* is the mount point of the
    ESP partition.


EXAMPLES
========

To install the drivers::

    # /usr/lib/sd-boot/sd-boot-efifs-update add

SEE ALSO
========

**sd-boot-efi-tool-update**, **sd-boot-kernel-update**, **sd-boot-find-boot-mounts**,
**kernel-install**

