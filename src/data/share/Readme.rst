.. SPDX-License-Identifier: GPL-2.0-or-later

.. _read_me:

#######
sd-boot
#######

Recent Changes
==============

**5.8.1**

* Man page update.
  Update sd-boot-find-boot-mounts man page with additional information about
  multiple ESP partitions and possible mismatch of nvram with current ESP as observed
  on some machines.

**5.8.0**

* sd-boot-find-boot-mounts: Handling Multiple ESP partitions (multi-disk).
  
  sd-boot-find-boot-mounts has been improved to display additional information.
  
  On systems with multiple ESP partitions where the motherboard boot loader boots 
  from a different partition than the expected partition leads to some output ambiguity. 

  This should not happen and on most machines it will not. 
  But it can and when it does then *bootctl status* displays: 
  
    WARNING: The boot loader reports a different partition UUID than the detected ESP.

  To resolve this we use bootctl to detect the "current" ESP. 
  
  I've seen this on happen 2 lenovo machines. What seems to be going on is that
  the nvram boot order can sometimes be ignored by the motherboard which choses to 
  boot from a different ESP. This in turn is saved to nvram. 
  
  This ESP is not the one requested to be used to boot from which leads to
  this tension between the two. 
  
  This may be a bug / feature of (some) lenovo motherboard firmware.

  NB: This does not affect any update/install/remove functionality; just 
  the information displayed by sd-boot-find-boot-mounts. 

  To highlight this sd-boot-find-boot-mounts now displays both
  *active* partitions (those detected by motherboard to nvram) as well
  as the expected or *current* partitions.

  See *man sd-boot-find-boot-mounts* for some additional information such as how to 
  manually identify this (quirky) case.

* Tune sd-boot-find-mounts
  Skip working on any non-block filesystems while looping over scanned mounts.

Please see Changelog for more history (found in */usr/share/sd-boot/*).
Or for recent changes: *pacman -Qc sd-boot*.

Overview
========

Provide tools to install linux kernels and efi programs using systemd's *kernel-install*.
This provides a robust and systematic way to install or remove bootable software to or 
from the *boot* partition.. 
*kernel-install* referrs to this as *$BOOT* and is usually one of */efi* or */boot*. In the 
past it was sometimes */boot/efi*, but mounting the ESP under /boot is discouraged.

The boot process requires access to the ESP partition 
and there are two distinct recommendations where the ESP should be mounted:

- Always mount ESP partition onto */efi* and XBOOTLDR partition, if used, onto */boot*
- Mount ESP partition onto */boot* or on */efi* if there is an XBOOTLDR partition 

The first is recommended by kernel-install and the second by the 
`UAPI Group Specifications: Mount Points <https://uapi-group.org/specifications/specs/boot_loader_specification>`_.

kernel-install happily works with either approach.

We designate the ESP mount point location as *<EFI>*, which is either */efi* or */boot* as
appropriate. We also refer to the directory where kernels are put as *<BOOT>*.
If you mount the ESP onto */boot* then both *<EFI>* and *<BOOT>* refer to */boot*.
If the ESP is mounted on */efi* and you have an XBOOTLDR partition mounted on */boot*
then they refer to those (obviously).

sd-boot provides:

- pacman alpm hooks
- command line install tools
- installers that are triggered from ALPM hooks.
- kernel-install plugin(s)

Both kernels and bootable efi tools can be installed by sd-boot. 

Configuration
-------------

In addition to the files kernel-install itself uses, such as
those in the */etc/kernel/* directory, *sd-boot* configuration files
reside in */etc/sd-boot/*.

The file */etc/sd-boot/kernel.packages* lists the package names
of kernels sd-boot is permitted to install. By default the Arch kernel is excluded.

Similarly, /*/etc/sd-boot/efi-tools.packages* lists the efi tool packages that
sd-boot is permitted to install. 

Package names here are the standard Arch package names.

Please ensure any package to be managed by sd-boot is listed appropriately.

By default kernel-install sets kernel boot options using in order:

.. code-block:: bash

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

For a kernel to be managed by *sd-boot* it must be installed in:

.. code-block:: text

   /usr/lib/modules/<kernel-version>
     with the kernel in
   /usr/lib/modules/<kernel-version>/vmlinuz

A kernel package must **NOT** install the kernel or any initrd into *<EFI>* or *<BOOT>*.
It should not even create an initrd.

Once the package is installed, list the kernels and efi-tools to be managed 
by sd-boot by editing and listing them in the files:

.. code-block:: text

   /etc/sd-boot/kernel.packages
   /etc/sd-boot/efi-tools.packages

On the next update (or re-install or remove), if the kernel / efi-tool is listed 
to be managed by *sd-boot* then it will handle installing it into $BOOT.

You can (re-)install a kernel from command line:

.. code-block:: bash

   sd-boot-kernel-install <kernel package name>

or trigger a pacman refresh with :

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

Arch kernels provide *pkgbase* file.

While *sd-boot* uses either of these files, *pkgbase-sdb* is preferred
since the Arch kernel install tools act on any kernel using *pkgbase*. 
Using a different filename prevents the Arch tools from installing kernels 
sd-boot is already handling.

Command Line
------------

The following tools may be run from the command line:

* /usr/bin/sd-boot-efi-tool-update
* /usr/bin/sd-boot-kernel-update
* /usr/lib/sd-boot/sd-boot-efifs-update
* /usr/lib/sd-boot/sd-boot-find-boot-mounts

sd-boot is normally triggered by pacman using ALPM hooks. Kernels and efi tools
may also be manually installed or removed from $BOOT. jj

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

Boot Loader Entries
-------------------

There are two kinds of systemd boot entries. These show up in the boot menu.

*type #1" use a *loader entry file* and and these are used when there are 
separate kernel and initrd images.

UKI images, which have the kernel and initrd combined into a single file, use *type #2* 
in which the UKI file itself is sufficient to provide the boot loader menu without 
any additional loader entry file.

Since sd-boot uses *kernel-install* to do the real work, type #1 boot loader entries are
automatically created for *bls* layout. It creates one new entry for each kernel version.

This means you should remove any stale (fixed) loader entries from */boot/loader/entries*
or */efi/loader/entries* (if that's where they are) for any kernel package managed by 
sd-boot.

sd-boot also provides a kernel-install plugin that modifies the raw type#1 loader entries.
These are generated for efi tools and for kernels using *bls* layout. 

The primary difference is the *Title* which is modified to be the package name. 
The title is shown in the boot menu.

By default the title is *Arch Linux* which comes from */etc/os-release* for every
kernel and efi tool.

efi-tools additionally remove the kernel boot command line options from the entry
and change the line from:

.. code-block:: text

   linux <tool>.efi

to

.. code-block:: text

   efi <tool>.efi

in BLS layout. 


It also means that the systemd-boot *loader.conf* file located in the *EFI* 
probably needs to be changed as well if the default kernel is one of those managed by sd-boot.

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

The wildcards are standard shell file globbing used to match the loader entry file.
With this change its helpful to verify things are seen correctly by running:

.. code-block:: bash

   bootctl
   bootctl list

For *uki* layout there are no loader entry files but the comments on *loader.conf* apply 
similarly.

dracut options
--------------

sd-boot provides a default dracut config file:

.. code-block:: text

   /etc/dracut.conf.d/010-dracut.conf

This file can be modifued, since it is listed in the PKGBUILD backup() array, or a new file may be added.
The last file read by dracut define the options that it uses. For example a file called 020-dracut.conf
will over-ride any settings in the one provided by sd-boot.

Efi Filesystem Drivers
----------------------

While the *EFI* is always Fat-32, the */boot* partition may use other filesystems. 
In this case the boot loader requires efi filesystem drivers that provided by the *efifs* package.

When this package is installed the driver files are located in the directory */usr/lib/efifs-x86/*,
sd-boot detects this, via an alpm hook, and installs them into 

.. code-block:: text
   
    <EFI>/EFI/systemd/drivers/

where systemd-boot expects to find them.

These can also be manually installed (or removed) from the command line using:

.. code-block:: bash

   /usr/lib/sd-boot/sd-boot-efifs-update add

or

.. code-block:: bash

   /usr/lib/sd-boot/sd-boot-efifs-update remove


Signing Kernels & Secure Boot
=============================

Kernels will be signed if keys are available thanks to kernel-install and sbctl.
Many thanks to Foxboron for writing sbctl which simplifies things enormously.

This is a very brief overview.
Please see Arch wiki and man pages for details on secure boot and kernel signing.

Signing Kernel
--------------

The sbctl package must be installed.

To have the kernels signed by local keys, first create the keys:

.. code-block:: bash

   sbctl create-keys

You may be prompted to *migrate* an older install, if so then do it:


.. code-block:: bash

   sbctl setup --migrate

Thats all that's required. This will sign the kernel image. Note that if the
layout is *bls* then the initrd is separate and unsigned. 

You may want to change to UKI where the initrd and the kernel image are packaged into 
a single file which gets signed. To change the layout simply edit
*/etc/kernel/install.conf* and change the layout to uki:


.. code-block:: bash

   layout=uki
   initrd_generator=dracut
   uki_generator=ukify

That's all that's required to have a unified kernel image signed by the local keys.

Secure Boot
-----------

**Big caveat**. Per the wiki, using local keys can lead to significant problems and may even break 
the machine. 

To use secure boot, the keys must be enrolled and secure boot activated in the UEFI Bios.

To enroll the signing keys the bios needs to have secure boot set to *setup* mode and then
boot up the machine and use *sbctl* to enroll the keys.

.. code-block:: bash

   sbctl enroll-keys -m

At this point the machine is in secure boot mode and will only boot signed kernels.

Layout: BLS vs UKI
==================

First off, I prefer *uki* layout. It is simpler, does not require separate loader entry
files, and both kernel and initrd are signed since they are in one file. The *uki* file
itself is an *efi* file and can therefore be directly booted without the need of a boot manager
should that ever be needed.

For this reason the dafault *install.conf* file provided by *sd-boot* uses *uki* layout.

UKI layout uses:

.. code-block:: text

    $BOOT/EFI/Linux/<machine-id>-<kernel-version>.efi

Of particular note is that there are no loader entry files in UKI layout.

BLS layout uses:

.. code-block:: text

    $BOOT/<machine-id>/<kernel-version>/linux
    $BOOT/<machine-id>/<kernel-version>/initrd
    $BOOT/loader/entries/<<machine-id>-<kernel-version>.conf

To switch from BLS to UKI layout first adjust */etc/kernel/install.conf*

.. code-block:: text

    layout=uki
    initrd_generator=dracut
    uki_generator=ukify

Then install the kernel package again. Please note that this does not remove the 
old loader entry or the old kernel. THese will need to be manually removed.

.. code-block:: text

   rm $BOOT/loader/entries/<<machine-id>-<kernel-version>.conf
   rm -rf $BOOT/<machine-id>/<kernel-version>

While dracut can generate the UKI file without using ukify, this has some limitations.
As of now, I recommend using ukify.

We would like to provide an additional option *OSRelease=* to ukify with a 
modified version of os-release having *PRETTY_NAME* and *BUILD_ID*  leading to
a more user friendly boot menu title. This option is available in the ukify
command line tool, but kernel-install does not call this, instead it imports
the ukify python module and calls the functions directly. The kernel-install
code does not provide for the *OSRelease=* option.

At this time, however, there is no clean way to do this that I could find.
Hopefully kernel-install will allow this in the future. In the meantime
the boot menu items in *uki* mode are precise but very long.

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

C-code Version
==============

While I was tempted to do this in python, in part since systemd chose to use it for 
*/usr/lib/kernel/install.d/60-ukify.install*, I decided on C. 

It's important to be able to do initial testing and validation as non-root user
and of course without touching any important files in any actual root directory.

While the the C-code version is a bit more work and a little more complicated to create,
the benefit, in my view, is that the c-code is much easier to test and debug
and keep organized (than bash versin). 

Testing and development are done using a *Testing* directory that is writable by non-root user.
This is where kernels will be installed, loader entries updated and so on. The tools 
are installed into this tree and are statically linked. Since dracut requires *PATH* 
to be set in the environ it is availble. Otherwise the environment is kept clean.

This allows for all programs to be tested by leveraging kernel-install's ability to work
with such a test set up. 

The environment variable *SDB_DEV_TEST* activates this and allows testing and debugging.

There are two test sets provided under *src/c-code/scripts*. Both test sets
should be run in the *src* directory.

* ./tests/scripts/run-test-suite

  This runs the the tools with *root* set to *Testing/__root__*.
  This is run as part of the build check process.
  It also runs the tools under valgrind.

* ./tests/scripts/static-analysis:
  
  It runs static code analysis using cppcheck and clang-tidy.

  Note that kernel-install always runs *chown* and since all tests are run as ordinary
  (non-root) user this leads to some warning messages landing in the testing logs. 
  These are benign as kernel-install ignores them. They dont happen in production 
  where kernel-install is running as root and chown is permitted.

  For each of these tests, the logs save stdout, stderr and the exit status of the tool
  along with the output from valgrind.

The image and initrd files will be installed in:

.. code-block:: text

    Testing/__root__/boot/<machine-id>/<kernel-version>/

while the loader entry files, for efi tools and kernels in *bls* layout  will be in 

.. code-block:: text

    Testing/__root__/boot/loader/entries/<machine-id>-<kernel-version>.conf 

You may notice that the loader entries have a longer path to the kernel image and initrd. 

This is normal.


When run in a test tree, kernel-install identifies the *mount* point and removes it from
the front of the path. In test mode where the test directory is not actually a mount point
(such as /boot) the pathname written to the loader entry will include more elements.
This is fine and when run in producion the image file will simply be *linux* instead of
*/long/path/to/linux*. The same is true for the initrd file as well.

Development and Debugging
-------------------------

Debuggable executables may be installed into the test root directory. 
From the *src* directory:

.. code-block:: bash

   ./do-dev-build
   cd tests/Testing
   export SDB_DEV_TEST=true

Then for example:

.. code-block:: bash

   gdb ./__root__/usr/bin/sd-boot-kernel-update

And from gdb run using for example:

.. code-block:: bash

    run add linux

Plugins are called by kernel-install but may be run manually as well:

* ./plugin-tests/run-loaderentry-efi (for bls plugin-tests/run-loaderentry-kernel)

  Standalone tests of the plugin that modify the raw loader entry files.
  The scripts run the tests under valgrind, but there is an option
  to run in the debugger as well.


Possible Todo
-------------

* When switching layout from bls to uki, previous kernel is not removed 
  since it was installed using bls layout while *kernel-install remove*
  is now using uki layout. This leaves the previous bls kernel install instead of 
  removing it. Same would be true switching layout from uki back to bls. 

  This means, at least for now, that manual intervention is necessary after changing layout 
  to avoid leaving un-needed files. While it is benign they do take disk space.

  For example after changing to uki layout, check */boot/loader/entries* and
  */boot/<machine-id>/* and remove the older kernel(s) that are no longer needed. 
  
  In uki mode there are no loader entry files at all and kernels are installed 
  in */boot/EFI/Linux* not in */boot/<machine-id>*.

  Here */boot* means either */boot* or */efi* as appropriate.

* So, it would be good to have the code be more helpful with this.

