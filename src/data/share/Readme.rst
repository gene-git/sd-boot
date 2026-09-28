.. SPDX-License-Identifier: GPL-2.0-or-later

.. _read_me:

=======
sd-boot
=======

Overview
========

**sd-boot** installs linux kernels and efi programs to the *boot* partition using systemd's *kernel-install*.
This is a robust and systematic way to install or remove bootable software to or 
from the *boot* partition. 

*kernel-install* uses *$BOOT* to refer to this partition or it's mount point. 
It is usually mounted on either */efi* or */boot*. 
In the past it */boot/efi* has also been used, but mounting the ESP under /boot is now strongly discouraged.

The boot process requires access to the ESP partition 
and there are two distinct recommendations. 

Mount the ESP partition:

- onto */efi* and an XBOOTLDR partition, if any, onto */boot*
- onto */boot* or */efi* if there is an XBOOTLDR partition and mount that on */boot*.

The first is recommended by kernel-install and the second by the 
`UAPI Group Specifications: Mount Points <https://uapi-group.org/specifications/specs/boot_loader_specification>`_.

sd-boot and kernel-install both happily work with either approach.

Our preference follows kernel-install; mount the ESP on */efi* and an
XBOOTLDR partition on */boot*. This is clean, simple and unambiguous.

We designate the ESP mount point location as *<EFI>*, which is either */efi* or */boot* as
appropriate. We also refer to the directory where kernels are put as *<BOOT>*.
If you mount the ESP onto */boot* then both *<EFI>* and *<BOOT>* refer to */boot*.
If the ESP is mounted on */efi* and you have an XBOOTLDR partition mounted on */boot*
then they refer to those (obviously).

While we recommend using the first approach for these mount points, we must also support 
any systems where the ESP is mounted on */boot*. This guarantees that all systems 
continue to work without any change.

sd-boot provides:

- pacman alpm hooks 

  These ensure that kernels and efi-tools  are installed or removed from the boot partition when
  a kernel package is installed, updated or removed.

- installers that are triggered from ALPM hooks.

  These do the actual work. 

- command line tools

  Give administrators a simple way to directly (re-)install or remove kernels from the boot partition.

- kernel-install plugin(s)

  Used for loader entries for non-UKI kernels and efi tools.

*Installing* a kernel (or efi tool) means placing the appropriate image in *$BOOT* partition so
that the image can be booted. 

Any kernel packages must therefore place kernel images in the */usr* partition only. 

The same applies for any bootable efi tools such as *edk2-shell* and *memtest86+*.

Documentation
-------------

The manual provides detailed information about using sd-boot.
It is available in both HTML and PDF formats.
Both are installed under */usr/share/sd-boot/docs*

The manual is also available at: `readthedocs <https://sd-boot.readthedocs.io>`_.

Each executable comes with a man page:

- sd-boot-efifs-update
- sd-boot-efi-tool-update
- sd-boot-find-boot-mounts
- sd-boot-kernel-update

For convenience, the manual includes all the man pages.

Coding Standards
----------------

All code should be carefully reviewed and checked. This is particularly true for
system utilities where the importance of avoiding potential problems is high. 

Placing trust in software requires care to ensure that it not only performs 
the required tasks but that the code is robust, handles exceptions gracefully and does 
not suffer from potential memory related issues. And of course no bugs!
Sure, having zero bugs ever is tricky. While we cannot guarantee perfection, 
there are things we can do to minimize them and bolster confidence in the code.

To help with this, sd-boot:

- has undergone functional testing
- has undergone static analysis with tools such as clang-tidy
- has undergone compiler analysis using tools such as -fanalyzer
- is completely clean when run under valgrind 
- has undergone human code review
- has underfone additional AI reviews.

  - Assisted-by: Claude (Anthropic) <https://claude.ai>

Quick Start
===========

There are two sets of configuration files

- kernel-install configuration
- sd-boot configuration

Both follow the Linux standard priority order and *drop-in* files that over-ride
settings in lower priority files. In rough terms:

- Files within a directory are prioritized alpha-numerically.
- Files in */etc* over-ride those in */usr*
- System provided files reside in */usr*
- Administration settings reside in */etc*
- Drop-in files in directory *xxx.d* may override partial settings. 

See *man 5 systemd.unit* for additional information about drop-in configuration files.

kernel-install
--------------

Please see the kernel-install documentation for full details.
sd-boot provides:

- /usr/lib/kernel/install.conf.d/010-sd-boot-install.conf

which sets the following::

    layout=uki
    initrd_generator=dracut
    uki_generator=ukify

These may be overridden in:

- /etc/kernel/install.conf
- /etc/kernel/install.conf.d/xxx.conf

We recommend sticking with UKI and dracut.

sd-boot
-------

All the configuration files are located in */etc/sd-boot*:

- **/etc/sd-boot/config.yaml**

  Settings::

    verb: 1
    skip_kernel_plugins: []

- **/etc/sd-boot/efi-tool.packages**

  A list of all package names of efi tools to be managed by sd-boot::

    memtest86_64-git
    edk2-shell

- **/etc/sd-boot/kernel.packages**

  A list of kernel packages managed by sd-boot::

    linux-custom
    linux-next
    linux-test
    linux-stable

Unlike kernels whihc are all installed in a fixed location,
efi tools must provide an *<package-name>.image* file which has the 
full path to the *.efi* program.  

These are included in sd-boot package:

- /etc/sd-boot/edk2-shell.image
- /etc/sd-boot/memtest86_64-git.image

Comment lines starting with **#** are permitted in all config files.

dracut
------

sd-boot provides a default dracut configuration in /usr/lib/dracut/dracut.conf.d/010-sd-boot-dracut.conf
which may be overridden in:

- **/etc/dracut.conf.d/010-sd-boot-dracut.conf**

