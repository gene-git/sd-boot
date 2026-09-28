==================
Bootable efi tools
==================

Please ensure the Arch package name is listed in /etc/sd-boot/efi-tools to let sd-boot
know which efi tools it is permitted to install.

sd-boot supports bootable efi tools such as *efi-shell* provided by the *edk2-shell* package.

The alpm hook *99-sd-boot-efi-tool-install.hook* provides the trigger based on package name
and the location of the efi file itself is found in

.. code-block:: text

    /etc/sd-boot/edk2-shell.image

This file contains the efi file path. It may also contain comments (lines starting with #).

Loader entries for efi tools are not kernels, and sd-boot uses a kernel-install plugin
to modify the entry appropriately.

To add any efi tool and have it *just work*, 2 things are needed.

* ALPM hook files to install and remove the tool from $BOOT.
* A file containing the path to the bootable efi file itself.

Simplest way to add a new efi tool, is to use the efi shell as templates and modify appropriately.

For example, if the efi tool package is called XXX-efi
then the alpm hook install file, installed (as usual) in::

   /usr/share/libalpm/99-XXX-efi-install.hook

should contain::

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

The remove file::

   /usr/share/libalpm/70-XXX-efi-remove.hook

should contain::

    [Trigger]
    Type = Package
    Operation = Remove
    Target = XXX-efi

    [Action]
    Description = sd-boot: Removing efi tool from ESP
    When = PreTransaction
    Exec = /usr/lib/sd-boot/sd-boot-efi-tool-update remove
    NeedsTargets

The last file provides the location of the efi file itself. This should be located in::

    /etc/sd-boot/XXX-efi.image

and contain the path itself::

   /usr/share/XXX-efi/x64/xxx-tool.efi


A note on memtest86 plus
------------------------

If sd-boot is used to install this please do not use the (current)
version of *memtest86+-efi* in the Arch repo. As of this writing, this package installs
files directly into */boot* which violates the basic requierments.
It is also somewhat out of date.

I may provide a compatible version to the AUR at some point if its helpful.
Note that the latest open source version from memtest.org is 8.x. There is
also a non-open source (commercial) version) 11.x in the AUR that installs to /usr/share.
The latest open source version is v8.10 at this time.

