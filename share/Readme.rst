.. SPDX-License-Identifier: GPL-2.0-or-later

.. _read_me:

#######
sd-boot
#######

Overview
========

Provide tools to install linux kernels and efi programs using systemd's *kernel-install*.
This provides a systematic way to install bootable software. 

The boot process requires access to the ESP partition
and there are two distinct recommendations where the ESP should be mounted:

- Always mount ESP partition onto */efi* and XBOOTLDR partition, if used, onto */boot*
- Mount ESP partition onto */boot* or on */efi* if there is an XBOOTLDR partition 

The first is recommended by kernel-install and the second by the 
`UAPI Group Specifications: Mount Points <https://uapi-group.org/specifications/specs/boot_loader_specification>`_.

kernel-install happily works with either approach.

We designate ESP mount point location as *<EFI>*, which is either */efi* or */boot* as
appropriate. We also refer to the directory where kernels are put as *<BOOT>*.
If you mount the ESP onto */boot* then both *<EFI>* and *<BOOT>* refer to */boot*.
If the ESP is mounted on */efi* and you have an XBOOTLDR partition mounted on */boot*
then they refer to those (obviously).

sd-boot provides:

- pacman alpm hooks
- kernel-install plugins
- suite of bash scripts

kernels and bootable efi tools can be installed by sd-boot. A config files lists the package names
of those kernels sd-boot is permitted to install. By default the Arch kernel is excluded.

On initial install it creates */etc/kernel/cmdline* (if not present) and populates it with the 
current kernel cmdline options taken from */proc/cmdline*. It also installs 
*/etc/kernel/install.conf* which sets the layout to *bls* and the initrd generator to *dracut*.

The `RFC 66 Discussion <https://gitlab.archlinux.org/archlinux/rfcs/-/merge_requests/66#note_452083>`_
about changing to use *kernel-install* triggered me to migrate my own kernels and
see how it works in practice. I found it works really well and am sharing this 
with the community in case its helpful to others.

Getting Started
===============

The requirements on a kernel package are that it is installed, as usual, in:

.. code-block:: text

   /usr/lib/modules/<kernel-version>
     with kernel
   /usr/lib/modules/<kernel-version>/vmlinuz

Package must **NOT** install the kernel or any initrd into */efi* or */boot*.
It should not even create an initrd.

Once the package is installed, set the kernels to be managed by sd-boot by
editing and listing them in the file:

.. code-block:: text

   /etc/sd-boot/kernel.packages

On the next (re)install of a managed kernel sd-boot will install it. 
You can trigger a refresh by :

.. code-block:: bash

    touch /usr/lib/modules/<kernel-version>/vmlinuz
    pacman -Syu


sd-boot relies on each kernel package providing a file in the kernel module
directory that contains that kernel package name:

.. code-block:: text

    /usr/lib/modules/<kernel-version>/pkgbase

or

.. code-block:: text

    /usr/lib/modules/<kernel-version>/pkgbase-sdb

The alternative name is offered since the Arch kernel install tools may act on any
kernel using *pkgbase*. To avoid the Arch tools installing kernels sd-boot is already
handling *pkgbase-sdb* may be used.


Loader Entries
--------------

Since sd-boot uses *kernel-install* to do the real work, systemd-boot loader entries are
automatically created. It creates one new entry for each kernel version.

This means you should remove any old (fixed) loader entries from */boot/loader/entries* (or
*/efi/loader/entries* if that's where they are) for the same kernel package.

It also means that the systemd-boot *loader.conf* file located in the *EFI* should be changed
if the default kernel to boot is one of those managed by sd-boot.

The loader entries are located in:

.. code-block:: text

   <EFI>/loader/entries/<machine-id>-<kernel-version>.conf
    
    for example

   <EFI>/loader/entries/<machine-id>-7.0.0-custom-1.conf

We would match this entry in *loader.conf* as the default kernel using perhaps:

.. code-block:: text

    # loader.conf
    default *-custom-*
    timeout 5
    editor  yes


With this change its helpful to verify things are seen correctly by running:

.. code-block:: bash

   bootctl
   bootctl list

dracut options
--------------

sd-boot provides a default dracut config file:

.. code-block:: text

   /etc/dracut.conf.d/010-dracut.conf

These options may be changed to suit your preference. This file is marked as a *backup* in the 
pacman PKGBUILD.

Efi Filesystem Drivers
----------------------

While the *EFI* is always Fat-32, the */boot* partition may use other filesystems. 
In this case the boot loader requires efi filesystem drivers that provided by the *efifs* package.

When this package is installed the driver files are located in the directory */usr/lib/efifs-x86/*,
sd-boot detects this, via an alpm hook, and installs them into 

.. code-block:: text
   
    <EFI>/EFI/systemd/drivers/

where systemd-boot expects to find them.

Bootable efi tools
==================

sd-boot supports bootable efi tools such as *efi-shell* provided by the *edk2-shell* package.
The alpm hook *99-sd-boot-efi-tool-install.hook* provides the trigger based on package name
and the location of the efi file itself is found in 


.. code-block:: text

    /etc/sd-boot/edk2-shell.image

This file contains the efi file path. It may also contain comments (lines starting with #).

To add any efi tool and have it *just work*, 2 files are then needed. An alpm hook file install
file, an alpm remove file and file containing the path to the bootable efi file itself.

Simplest is to copy the provided files for efi shell as templates and modify appropriately.

For example, if the efi tool package is called XXX-efi
then the alpm hook install file, installed (as usual) in

.. code-block:: text

   /usr/share/libalpm/99-XXX-efi-install.hook

should contain:

.. code-block:: text

   [Trigger]
    Type = Package
    Operation = Install
    Operation = Upgrade
    Target = XXX-efi

    [Action]
    Description = sd-boot: installing an efi tool to EFI
    When = PostTransaction
    Exec = /usr/lib/sd-boot/sd-boot-efi-tool-update add
    NeedsTargets

The remove file:

.. code-block:: text

   /usr/share/libalpm/70-XXX-efi-remove.hook

should contain:

.. code-block:: text

    [Trigger]
    Type = Package
    Operation = Remove
    Target = XXX-efi

    [Action]
    Description = sd-boot: Removing efi tool from ESP
    When = PreTransaction
    Exec = /usr/lib/sd-boot/sd-boot-efi-tool-update remove
    NeedsTargets

The last file provides the location of the efi file itself. This should be located in

.. code-block:: text

    /etc/sd-boot/XXX-efi.image

and contain the path itself:

.. code-block:: text

   /usr/share/XXX-efi/x64/xxx-tool.efi


A note on memtest86+
^^^^^^^^^^^^^^^^^^^^

If sd-boot is to be responsible for installing this then please do not use the (current)
version of *memtest86+-efi* in Arch repo. As of this writing, this package installs 
files directly into */boot* which is not what is needed. I'm sure this will be updated
at some point. It is also somewhat out of date. 

I may provide a compatible version to the AUR at some point if its helpful.
Note that the latest open source version from memtest.org is 8.x. There is
also a commercial (non open source version) 11.x in the AUR which does install to /usr/share.
The latest open source version is v8.00 at this time.

