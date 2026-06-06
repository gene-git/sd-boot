.. SPDX-License-Identifier: GPL-2.0-or-later
.. SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>

=====================
sd-boot-kernel-update 
=====================

--------------------------------------------------------
Install (or remove) a kernel to (or from) boot partition
--------------------------------------------------------

:Author: Gene C <arch@sapience.com>
:Manual section: 8 
:Manual group: Linux Tools
:Date: @DATE@
:Version: @VERSION@

SYNOPSIS
========

``sd-boot-kernel-update``  ``add`` | ``remove`` ``<pacmage_name>``

DESCRIPTION
===========

Install (or remove) a linux kernel to (or from) the $BOOT partition using kernel-install.
The $BOOT partition, following kernel-install, is usually one of */boot* or */efi*.

Takes two arguments: *add* | *remove* and the package name providing the kernel.

The kernel package name must be one that is listed to be managed by sd-boot and
the kernel package must provide a package basename file that has the package name.

When a kernel package is installed using pacman, it  must be installed to::

    /usr/lib/modules/<kernel_version>

It must never instll any files to /boot (or /efi). kernel-install (and hence sd-boot)
are responsible for using vmlinux from */usr/lib/modules/<vers>*, generating the initrd
and installing them into $BOOT.

The layout, specified in */etc/kernel/install.conf* may be set to *bls* or *uki*.
In bls layout, the kernel and the initrd are installed separately. In addition
type #1 boot loader entry files are generated.

In *uki* layout the kernel and initrd are combined into a single unified kernel image (uki) 
and installed into the *efi*. These use type #2 loader entries that are taken directly from the
uki install and do not use a separate loader entry file, 

*uki* layout is preferable. It is simpler, does not require separate loader entry
files, and both kernel and initrd are signed since they are in one file. The *uki* file
itself is an *efi* file and can therefore be directly booted without the need of a boot manager
should that ever be needed.

sd-boot will only act if 2 conditions are met.

* the kernel package name is listed to be managed by sd-boot
* there must be a special file installed by the package in the 
  */usr/lib/modules/<kernel-version>* directory called *pkgbase-sdb* or *pkgbase*.
  This file should contain the package name.

There is an related tool (*sd-boot-kernel-update-triggers*) used by pacman that is 
called via an ALPM hook.

It is similar but takes a single argument only (add or remove) and reads the *triggers*
from stdin as provided by pacman.

ARGUMENTS
=========

* add <kernel_package_name> 

    Generate initrd and install the kernel provided in the package <kernel_package_name>
    along with it's initrd to $BOOT

* remove

    Remove the kernel from $BOOT. 
    
Note that this updates files in $BOOT. pacman is responsible for installing or removing the 
package itself.


FILES
=====

``/etc/kerne/install.conf``

    The kernel-install configuration file. The default version provided by sd-boot uses 
    uki layout and dracut for the initrd generator:

.. code-block:: text

    layout=uki
    initrd_generator=dracut
    uki_generator=ukify

``/etc/sd-boot/kernel.packages``

    The kernel package name must be listed here to permit sd-boot to manage installing
    or removing the kernel 

``/usr/lib/modules/<kernel-version>/vmlinuz``

    This is the kernel, provided by the package, that will be installed to $BOOT.

``/usr/lib/modules/<kernel-version>/pkgbase-sdb``

    This file contains the name of the kernel package.
    Arch kernels provide the file *pkgbase*. 

    While *sd-boot* uses either of these files, *pkgbase-sdb* is preferred
    since the Arch kernel install tools act on any kernel using *pkgbase*.
    Using a different filename prevents the Arch tools from installing kernels
    sd-boot is already handling.

``$BOOT/<machine-id>/<package_name>-<package_version>``

    When layout is *bls* only, 
    Kernels and initrd fles are installed to this directory.

``$BOOT/loader/entries/<machine-id>-<package_name>-<package_version>.conf``

    When layout is *bls* only, 
    This is the type #1 boot loader entry file. It has the title that is used in boot menu
    along with the path to the program to be booted.

``$BOOT/EFI/Linux/<machine-id>-<kernel_version>.efi``

    When layout is *uki* only,
    the unified kernel image is saved here.

EXAMPLES
========

To install the kernel provided by the package *linux-custom*::

    # sd-boot-kernel-update add linux-custom

where linux-custom must be listed in the */etc/sd-boot/kernel.packages*.

In addition the linux-custom package must provide the file::

    /usr/lib/modules/<kernel-version>/pkgbase

or::

    /usr/lib/modules/<kernel-version>/pkgbase-sdb

that contains one line::

    linux-custom


