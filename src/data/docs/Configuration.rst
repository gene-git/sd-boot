=============
Configuration
=============

In addition to the files kernel-install itself uses, such as
those in the */etc/kernel/* directory, the *sd-boot* configuration files
reside in */etc/sd-boot/*.

The file */etc/sd-boot/kernel.packages* lists the kernel package names
sd-boot is permitted to install.

By default, the Arch kernel is not included in the list, but of course sd-boot can manage that too.
The section :ref:`arch_kernels` explains how to use sd-boot for the Arch kernel.

Similarly, */etc/sd-boot/efi-tools.packages* lists the efi tool packages that
sd-boot is permitted to install.

Package names here are the standard Arch package names.

Please ensure any package to be managed by sd-boot is listed appropriately.

By default kernel-install sets kernel boot options using in order::

    /etc/kernel/cmdline
    /usr/lib/kernel/cmdline
    /proc/cmdline

Putting kernel command line options into */etc/kernel/cmdline* will then over-ride the
default. If no file provides the kernel options, then The default is to use /proc/cmdline
which proivdes the kernel command line option of the currently booted kernel.

On initial install sd-boot provides
*/etc/kernel/install.conf* which sets the layout to *uki*, the initrd generator to *dracut*
and the uki generator to *ukify*.

The `RFC 66 Discussion <https://gitlab.archlinux.org/archlinux/rfcs/-/merge_requests/66#note_452083>`_
about changing to use *kernel-install* triggered me to migrate my own kernels and
see how it works in practice. I found it works really well and am sharing this
with the community in case its helpful to others.

The original version of *sd-boot* was written in bash, but the current version
coded in *C* has since replaced it.

Getting Started
===============

For a kernel to be managed by *sd-boot* the kernel package should install to::

   /usr/lib/modules/<kernel-version>

with the kernel image at::

   /usr/lib/modules/<kernel-version>/vmlinuz

A kernel package must **NOT** install the kernel or any initrd into *<EFI>* or *<BOOT>*.
It should not even create an initrd. Installing anything to these *$BOOT* directories
must be left to sd-boot. This applies equally to any efi tools.

Once the package is installed, then grant sd-boot permission to manage
those kernels and efi-tools by listing them in the files::

   /etc/sd-boot/kernel.packages
   /etc/sd-boot/efi-tools.packages

On the next update (or re-install or remove), if the kernel or efi-tool is designated
to be managed by *sd-boot* then it will handle installing it into $BOOT.

You can (re-)install a kernel at any time at the command line using::

   sd-boot-kernel-install <kernel package name>

or trigger a pacman refresh with :

.. code-block:: bash

    touch /usr/lib/modules/<kernel-version>/vmlinuz
    pacman -Syu

sd-boot relies on each kernel package providing a file in the kernel module
directory that contains that kernel package name::

    /usr/lib/modules/<kernel-version>/pkgbase

or::

    /usr/lib/modules/<kernel-version>/pkgbase-sdb

Arch kernels provide *pkgbase* file.
While *sd-boot* uses either of these files, *pkgbase-sdb* is preferred
since the Arch kernel install tools act on any kernel using *pkgbase*.
Using a different filename prevents the Arch tools from installing kernels
sd-boot is already handling.

Command Line Tools
==================

The following tools may be run from the command line:

* /usr/bin/sd-boot-efi-tool-update
* /usr/bin/sd-boot-kernel-update
* /usr/lib/sd-boot/sd-boot-efifs-update
* /usr/lib/sd-boot/sd-boot-find-boot-mounts

There is a man page for each of these. For example see *man sd-boot-kernel-update*.

sd-boot is normally triggered into action from pacman via ALPM hooks. Kernels and efi tools
may also be manually installed or removed from $BOOT.

For example to install a bootable efi shell, provided by the *adk2-shell* package:

.. code-block:: bash

   sd-boot-efi-tool-update add edk2-shell

To install a kernel provided by the *linux-custom* package

.. code-block:: bash

    sd-boot-kernel-update add linux-custom


Please ensure that any packages to be managed by sd-boot are listed appropriately in:

.. code-block:: bash

   /etc/sd-boot/efi-tool.packages
   /etc/sd-boot/kernel.packages

To install efi filesystem drivers:

.. code-block:: bash

    /usr/lib/sd-boot/sd-boot-efifs-update add

To list ESP and XBOOTLDR partions mount points run (as root):

.. code-block:: bash

   /usr/lib/sd-boot/sd-boot-find-boot-mounts

which will list them all and those in use by the current booted system are marked
with an asterisk.

Replace *add* by *remove* to remove them from $BOOT. Note that *add* and *remove* refers
only to a copy of the image in $BOOT and does not install or remove the package itself.
That job belongs to pacman.

