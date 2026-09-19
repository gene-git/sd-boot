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

``sd-boot-efi-tool-update`` ``add`` | ``remove`` | ``inspect`` ``<package_name>``

DESCRIPTION
===========

Install, remove, or inspect a bootable efi tool in the $BOOT partition using kernel-install.
The $BOOT partition, following the notation used by kernel-install, is usually one of */boot* or */efi*.

The package must be managed by sd-boot with the package name one of those listed in::

    /etc/sd-boot/efi-tool.packages

It takes two arguments. The first argument is the operation::

   *add* | *remove* | *inspect* 

followed by the package name that provides the tool.

For installs using *add*, the path of the source **efi** image must be provided in::

    /etc/sd-boot/<package-name>.image

This file may contain shell style comments as well as the image path.

Since these tools are not kernels, they are installed using *BLS* layout, even 
when the kernel layout is set to *UKI* in */etc/kernel/install.conf*. This ensures that
the efi tools are installed to::

    /boot/<machine-id>/<package_name>/

and that boot loader entry files are appropriately created in::

    /boot/loader/entries/<machine_name-pacmage_name>.conf

ARGUMENTS
=========

**Operation**:

* add 

    Install the efi tool to $BOOT partition

* remove

    Remove the drivers from $BOOT partition.

* inspect

    Disply information about an installed package.

**Package Name**

Can be name of a package or the string "--all--" which means all efi tools managed by sd-boot.

FILES
=====

``/etc/sd-boot/<package-name>.packages``

    The package name must be listed here to permit sd-boot to manage the tool. 

``/etc/sd-boot/<package-name>.image``

    This file contains the full path of the efi tool provided by the package. This efi image
    file will be copied to $BOOT.

``$BOOT/<machine-id>/<package_name>-<package_version>``

    This is the path where the bootable efi tool is installed.

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

The image itself is provided by the *edk2-shell* package.

To update all efi tools currently managed by sd-boot::

    sd-boot-efi-tool-update add "--all--"

