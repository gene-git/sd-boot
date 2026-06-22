.. SPDX-License-Identifier: GPL-2.0-or-later
.. SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>

=======================
sd-boot-efi-tool-update
=======================

--------------------------------------
Install or remove an efi tool to $BOOT
--------------------------------------

:Author: Gene C <arch@sapience.com>
:Manual section: 8 
:Manual group: Linux Tools
:Date: @DATE@
:Version: @VERSION@

SYNOPSIS
========

``sd-boot-efi-tool-update`` ``add`` | ``remove`` ``<package_name>``

DESCRIPTION
===========

Install (or remove) a bootable efi tool to (or from) the $BOOT partition using kernel-install.
The $BOOT partition, following kernel-install, is usually one of */boot* or */efi*.

Takes two arguments: *add* | *remove* and the package name providing the tool.

The package must be one that is listed as managed by sd-boot and
the path to the efi file to be installed must be provided.

Since these tools are not kernels, these are installed using *BLS* layout, even 
when layout is set to *UKI* in */etc/kernel/install.conf*. This ensures that
the tools are installed to::

    /boot/<machine-id>/<package_name>/

and that boot loader entry files are appropriately created in::

    /boot/loader/entries/<machine_name-pacmage_name>.conf

ARGUMENTS
=========

* add 

    Install the drivers to $BOOT partition

* remove

    Remove the drivers from $BOOT partition.

FILES
=====

``/etc/sd-boot/<package-name>.packages``

    The package name must be listed here to permit sd-boot to manage the tool. 

``/etc/sd-boot/<package-name>.image``

    This file contains the full path of the efi tool provided by the package. This file
    will be copied to $BOOT.

``$BOOT/<machine-id>/<package_name>-<package_version>``

    This is where the bootable efi tool is installed.

``$BOOT/loader/entries/<machine-id>-<package_name>-<package_version>.conf``

    This is the type #1 boot loader entry file. It has the title that is used in boot menu
    along with the path to the program to be booted.

EXAMPLES
========

To install the efi shell from *edk2-shell* package::

    # sd-boot-efi-tool-update add edk2-shell

where edk2-shell is listed in */etc/sd-boot/edk2-shell.packages* and the pathname 
is provided in the file */etc/sd-boot/edk2-shell.image* which contains::

    /usr/share/edk2-shell/x64/Shell_Full.efi

The file is provided by the *edk2-shell* package.
