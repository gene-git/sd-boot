Changelog
=========

Tags
====

.. code-block:: text

	1.1.0 (2022-10-02) -> 4.7.1 (2026-05-19)
	238 commits.

Commits
=======


* 2026-05-19  : **4.7.1**

.. code-block:: text

              - Update version file
              - Update version file
              - readme
              - Clean up cmake input file
              - Simplify the liblkid by taking the function out of library and including in only application using it
              - type in test script mmessage text
              - **4.7.1**
            
                * Change PKGBUILD arch to x85_64 not any.
                * Change linking to only use libblkid where needed.
              - more defensive checks

* 2026-05-19  : **4.7.0**

.. code-block:: text

              - Update version file
              - readme
              - Catch a potential memory error from strdup
              - code tidy
              - Add sbctl and systemd-ukify as dependencies
              - kernel-install remove does not need kernel image
              - Mesasge white space adjustment
              - * Read /etc/kernel/install.conf so code is aware of layout being used.
                * Loader entry plugin skipped earlier for uki kernels since there are no loader entries.

* 2026-05-18  : **4.6.0**

.. code-block:: text

              - Update version file
              - readme
              - Remove another message when loader entry mod cannot open the (non existent) file
              - Loader entry files only apply for BLS layout.
                  Make missing loader entry file not an error

* 2026-05-18  : **4.5.0**

.. code-block:: text

              - add warning about needing root to run find-boot-mounts
              - Update version file
              - readme update
              - add call to endmntent()
              - close file pointer from setmntent()
              - typo
              - .
              - ...
              - duh
              - tidy find_efi
              - null after free
              - simplify find_efi_current
              - Free mem in sd-bot-find-boot-mounts
              - Enhance sd-boot-find-mounts using liblkid
              - Silence missing loader entry - uki there are none but plugin is still called
              - Add /etc/kernel/* changes to trigger kernel fresh kernel build
              - remove unused
              - Update version file
              - readme tweak
              - update readme
              - Tidy up the c-code checks / valgrind runs
                Simplify some code to keep it readable
                Add some additional boundary case checks
              - fix typo in c-code check script
              - efi tool loader entry update to also modify uki xxx to efi xxx
              - Add signing key changes trigger to kernel update alpm hook.
                Messages more consistent
                  - Errors prefixed with !
                  - Top levl prefixed with ⦁
                  - sub level prefix ↳

* 2026-05-15  : **4.3.0**

.. code-block:: text

              - Update version file
              - delete unused
              - * Drop the bash code version and sd-boot-set-alternative since its no longer needed.
                * Simplify installation.
                * When adding a new kernel version, check if previous version still there before
                  calling *kernel-install remove* to remove the previous version since it may have
                  already been removed by pacman. No functional impact other than avoiding one
                  unnecessary fork/exec.
                * Remove one unnecessary trigger from kernel update alpm hook.

* 2026-05-07  : **4.2.2**

.. code-block:: text

              - Update version file
              - Small code comment clean ups - no functional change
              - Improve information message when modify efi tool loader entry

* 2026-05-07  : **4.2.1**

.. code-block:: text

              - Update version file
              - Make sure test suite runs in fresh set up - fixed

* 2026-05-07  : **4.2.0**

.. code-block:: text

              - Update version file
              - typo
              - Improve trigger handling for paths (lacking leading /)
              - Remove develop debug print
              - Update version file
              - Bug - pacman trigger paths are sent without a leading "/" which
                  led to a kernel image trigger being treated not belonging to that kernel, but instead a trigger
                  to update all kernels. Fixed!!
              - temp debug print
              - move check for whether image page belongs to a kernel to library
              - Add check in sd-boot-efi-update to skip packages not listed
                  in /etc/sd-boot/efi-tools.packages
              - protect against possible nullptr for non-existent kernel image

* 2026-05-07  : **4.0.1**

.. code-block:: text

              - Update version file
              - update aur PKGBUILD

* 2026-05-07  : **4.0.0**

.. code-block:: text

              - Update version file
              - add testing template file efi-tool.packages
              - Update plugin for efi tool to use the new unified plugin 95-sd-boot-loaderentry-modify.install
              - tidy up loadentry struct element
              - Add etc/sd-boot/efi-tools.packages
                Update Readme
 2026-05-06   ⋯

.. code-block:: text

              - Remove unused plugin tests
                Fix remove_file to be non-error if non-existent
              - USe a single loaderentry plugin which handles kernels and non-kernels
                set-alternative now sets link directly to foo -> binary/foo etc. No more intervening link alt -> binary
              - plugins - initialization now determines if dealing with kernel or not
              - plugin tweaks avoiding entries theyre not supposed to work on
              - plugins - plugins are always called. So when plugin is called and
                  input is not applicable, be nicer/quiet about it. For example when kernel being updated both
                  efi and kernel plugins are called to modify the loader entry.  And vice versa.
                  It is important they do ignore - efi for example removed the kernel command line as
                  not relevant - fine for efi tool - not fine at all for a kernel
              - Remove making var/lib/sd-boot and have code do it if missing instead
              - typo
              - loader entry plugins - add a message
              - efi-tool update - add some messages add/remove etc
              - msg() - flush stdout
              - dont need usr/bin dir in installer any longer
              - Work around makepkg using libfakeroot using LD_LIBRARY_PATH to set default alternative
              - installer - another alternative fix
              - Package build install - set default alternative using build/none
              - Prep for package building
              - aur PKGBUILD

* 2026-05-06  : **3.99.0**

.. code-block:: text

              - Update version file
 2026-05-05   ⋯

.. code-block:: text

              - Add license and change c++ comments to c comments
              - update readme
              - Add Testing/Template directory so code checks and test suite can run fresh
              - plugins are always run - plugin must decide if it should be doing anything.
                If not relevant they exit quietly.
              - Array_str replaces older Array_2dp - simpler and cleaner
              - Restore original hooks (without extra ../sd-boot/tools).
                More work on installer
 2026-05-04   ⋯

.. code-block:: text

              - more prep work on installers
              - Readme small tweak about c-code version
              - rename dir with library source to lib/
              - Install set-alternative directly into /usr/lib/sd-boot
              - bash - remove tools/sd-boot-find-xbootldr
              - bash:sd-boot-find-boot-mounts replaces sd-boot-find-efi sd-boot-find-xbootldr - now matches the c-code
              - dev snap
              - Handle kernel trigger for package not installed or missing pkgbase file
 2026-05-03   ⋯

.. code-block:: text

              - sd-boot-kernel-update working and tested
 2026-05-02   ⋯

.. code-block:: text

              - dev snap
              - dev snap
 2026-05-01   ⋯

.. code-block:: text

              - Development snap
 2026-04-30   ⋯

.. code-block:: text

              - Begin re-coding tools in C.
                Reorg the source and install to support alternatives
                  bash = src/bash-code
                  binary = src/c-code
                Add tool to set alternative to use
                  sd-boot-set-alternative binary (or bash)

* 2026-04-24  : **3.8.0**

.. code-block:: text

              - Update version file
              - Clean up sd-boot-efifs-update

* 2026-04-22  : **3.7.0**

.. code-block:: text

              - Update version file
              - typo in load-efi-image_file unfixing re-entrant buglet

* 2026-04-22  : **3.6.7**

.. code-block:: text

              - Update version file
              - changelog
              - undo strip whitespace - read is not re-entrant in bash!
              - bash scripts: simplify stripping whitespace

* 2026-04-21  : **3.6.6**

.. code-block:: text

              - Update version file
              - PKGBUILD : tweak the description

* 2026-04-21  : **3.6.5**

.. code-block:: text

              - Update version file
              - Drop our memtest package from optdepends until we release it

* 2026-04-21  : **3.6.4**

.. code-block:: text

              - Update version file
              - PKGBUILD should be arch "any"

* 2026-04-21  : **3.6.3**

.. code-block:: text

              - Update version file
              - PKGBUILD - no .install needed

* 2026-04-21  : **3.6.2**

.. code-block:: text

              - Update version file
              - Update version file
              - Add arch PKGBUILD

* 2026-04-21  : **3.6.1**

.. code-block:: text

              - readme typo

* 2026-04-21  : **3.6.0**

.. code-block:: text

              - Inline the migrate code into PKGBUILD
                  has to be done pre-install and script wont be available till post install
              - Fix sd-boot-package-version-migrate
                   Remove unused dir /etc/sd-boot/kernel-install

* 2026-04-21  : **3.5.0**

.. code-block:: text

              - installer updated to install functions/kernel-version-to-package.sh
              - clean up installer ready for next release
              - tidy installer
              - Remove
                  hooks/60-sd-boot-kernel-pkgbase-remove
                  hooks/90-sd-boot-kernel-pkgbase-install
                  lib/sd-boot-kernel-pkgbase-update

* 2026-04-20  : **3.4.1**

.. code-block:: text

              - Tweak some of the print statements
 2026-04-19   ⋯

.. code-block:: text

              - kernel updates - when there is a kernel update avoid updating other kernels unless there is a different update trigger
              - Add linux-stable to list of packages using kernel-install

* 2026-04-19  : **3.4.0**

.. code-block:: text

              - more work on previous version removal
              - kernel update now calls sd-boot-package-version-update
              - rework the installer scripts to deal with package version updates
                  - drop the older hooks tasked with this job
              - duh
              - installer change for new hook priority
              - alpm hook to update package version moved ahead of kernel install hook
              - update etc/kernel/install.d/95-loaderentry-kernel.install
              - installer update
              - Rework tools - when package is updated it is our job to remove the previous version.
                  - version tracker file /etc/sd-boot/kernel-install/<pkg>.version holds version and previous version
                  - used by both kernels and efi tools
                  - on update add the new version and remove the previous version.
                    Care needed if reinstalling same version - in this case only add the package (dont remove)
              - bug remove of efi tool

* 2026-04-18  : **3.3.0**

.. code-block:: text

              - alpm hook for efi tools - bug fix - adds NeedsTargets
              - remove test print from 95-loaderentry-efi.install

* 2026-04-18  : **3.2.0**

.. code-block:: text

              - more efi loader entry fixes
              - efi program loader entry - more changes in 95-loaderentry-efi.install
              - bug fixes for efi tool install
              - fix typo in sd_boot_load_package_version

* 2026-04-18  : **3.1.0**

.. code-block:: text

              - kernel-install loaderentry plugin: change output message
              - plugin etc/kernel/install.d/95-loaderentry-efi.install: exit not return
              - installer - add missing efi program image names
              - installer hook name fix
              - Major changes
                - fixed bug when updating multiple kernels may not update them all
                - changed alpm hooks, kernel-install plugins
                - added support for efi tools like efi-shell and memtest
 2026-04-14   ⋯

.. code-block:: text

              - Using kernel-install: linux-custom, linux-next and linux-test
              - update Docs/Changelog.rst

* 2026-04-14  : **3.0.0**

.. code-block:: text

              - Add comment that boot menu only shows version of title is not unique
              - 95-loaderentry-title.install: Make the title the package name
                  kernel-install adds a version in entry and so menu should then be: "title (version)"
 2026-04-13   ⋯

.. code-block:: text

              - remove verb - somethign not right
              - update Docs/Changelog.rst

* 2026-04-13  : **2.16.0**

.. code-block:: text

              - Duh - we commented out kernel-install for testing .. .fix that
                Add env KERNEL_INSTALL_VERB=y to pass "-v" to kernel-install
              - loader entry - add white space so content is better aligned
              - kernel .install files must be executable
              - update Docs/Changelog.rst

* 2026-04-13  : **2.15.0**

.. code-block:: text

              - FIx etc/kernel/install.d/95-loaderentry-title.install
              - /etc/kernel/install.d/95-loaderentry-title.install must be executable
              - Installer - install /etc/kernel/install.d/95-loaderentry-title.install
              - bug in 95-loaderentry-title.install
              - update Docs/Changelog.rst

* 2026-04-13  : **2.14.0**

.. code-block:: text

              - sd-boot-kernel-update: handle kernel and non-kernel triggers.
                  - kernel trigger applies to that one kernel
                  - non-kernel trigger applies to all (our own) kernels
              - update Docs/Changelog.rst

* 2026-04-13  : **2.13.0**

.. code-block:: text

              - update alpm hooks
              - shellcheck all bash scripts and tidy up
              - Add etc/kernel/install.d/95-loaderentry-title.install
                  Fixes the loader entry title to PKGBASE (kernel version)
              - sd-boot-kernel-update: drop initrd from command line
              - Small tweaks
              - sd-boot-kernel-update: Add initrd file to install case
              - installer tweak
              - Prep for kernel-install
                /usr/lib/sd-boot/sd-boot-kernel-update
                /etc/kernel/xxx
                /usr/share/libalpm/hooks/90-kernel-install-gc.hook
                /usr/share/libalpm/hooks/60-kernel-remove-gc.hook
 2026-04-12   ⋯

.. code-block:: text

              - update Docs/Changelog.rst

* 2026-04-12  : **2.12.1**

.. code-block:: text

              - handle errors from permissions
              - duh
              - installer - ensure loader entries go to efi directory.
                Set default perms on /efi to 700 as is now the norm
              - update Docs/Changelog.rst

* 2026-04-12  : **2.12.0**

.. code-block:: text

              - another installer booboo
              - dracut.conf - missing whitespace for omit
              - fix installer
              - New tools:
                  - sd-boot-find-efi
                  - sd-boot-find-xbootldr
            
                Update hook to install efifs drivers for systemd to use when XBOOTLDR filesystem is not fat 32
                Update hook to install efi-shell
                The hooks now use sd-boot-find-xxx to locate where esp / xbootldr partitions are mounted
 2026-01-29   ⋯

.. code-block:: text

              - update Docs/Changelog.rst

* 2026-01-29  : **2.11.0**

.. code-block:: text

              - Put boot loader sample entries under entries/samples in the repo source
 2025-12-13   ⋯

.. code-block:: text

              - update Docs/Changelog.rst

* 2025-12-13  : **2.10.0**

.. code-block:: text

              - Remove boot entries for *full* as we no longer need them
 2025-02-01   ⋯

.. code-block:: text

              - update Docs/Changelog.rst

* 2025-02-01  : **2.9.0**

.. code-block:: text

              - dracut version 106 outputs "Creating user/group" which we grep out until quiet mode works properly

* 2024-11-16  : **2.8.1**

.. code-block:: text

              - We only create hostonly initrd so change print to no longer say full as well
 2024-01-15   ⋯

.. code-block:: text

              - update Docs/Changelog.rst

* 2024-01-15  : **2.8.0**

.. code-block:: text

              - Change to only produce hostonly initrd.
                  remove "full" image if found in /boot
 2023-11-30   ⋯

.. code-block:: text

              - entries/memtest86_64.conf is now part of memtest86 package
 2023-11-27   ⋯

.. code-block:: text

              - update Docs/Changelog.rst

* 2023-11-27  : **2.7.1**

.. code-block:: text

              - fix installer to use Docs/Changelog.rst
              - Add Docs/Changelog.rst to repo

* 2023-11-14  : **2.7.0**

.. code-block:: text

              - sd-boot-efi-shell-full.hook make sure /boot/tools is directory
 2023-09-19   ⋯

.. code-block:: text

              - update CHANGELOG.md

* 2023-09-19  : **2.6.1**

.. code-block:: text

              - sd-boot-install-efifs now requires ESP directory on command line
              - update CHANGELOG.md

* 2023-09-19  : **2.6.0**

.. code-block:: text

              - Add standalone tool sd-boot-install-efifs
              - Add --mkpath so dirs created during install
 2023-04-30   ⋯

.. code-block:: text

              - update CHANGELOG.md

* 2023-04-30  : **2.5.1**

.. code-block:: text

              - Fix installer for name change of CHANGELOG
              - update do-install with sd-boot-refresh-initrd
              - Update CHANGELOG - renamed from Changelog

* 2023-04-30  : **2.5.0**

.. code-block:: text

              - Provde sd-boot-refresh-initrd to update initrd
 2023-02-20   ⋯

.. code-block:: text

              - Update sample loader entries to current path usage
 2022-10-27   ⋯

.. code-block:: text

              - update changelog
              - rename /sd-boot-efi-shell.hook sd-boot-efi-shell-full.hook
              - upd changelog

* 2022-10-27  : **2.4.0**

.. code-block:: text

              - Change efi shell to now install in /boot/tools (was /boot)
                Add sd-boot loader entry to /boot/loader/entries
 2022-10-21   ⋯

.. code-block:: text

              - echo doesnt grok \t
              - update changelog
              - Add message after each initrd is completed
 2022-10-20   ⋯

.. code-block:: text

              - update changelog

* 2022-10-20  : **2.3.0**

.. code-block:: text

              - Add --quiet option to dracut
 2022-10-11   ⋯

.. code-block:: text

              - update changelog

* 2022-10-11  : **2.2.2**

.. code-block:: text

              - Add changelog to /usr/share/sd-boot
              - Add Changelog

* 2022-10-10  : **2.2.1**

.. code-block:: text

              - Change Type to Path - File is deprecated
              - No longer need to remove older locations like /boot/<kernel>/xxx
                Tidy up script a bit

* 2022-10-03  : **2.2.0**

.. code-block:: text

              - Fix bug for initrd check (used -d instead of -f when changed prev commit)

* 2022-10-03  : **2.1.1**

.. code-block:: text

              - Tweak check for cleaning up old /boot/xxx/initrd

* 2022-10-03  : **2.1.0**

.. code-block:: text

              - Also remove /boot/<kernel>/initrd*img

* 2022-10-03  : **2.0.0**

.. code-block:: text

              - Change to use /boot again for initrd
                Add clean up for older initrd and mkinitcpio initramfs

* 2022-10-02  : **1.1.2**

.. code-block:: text

              - Bug fix - both initrd were hostonly
              - Bug - remove double --kver

* 2022-10-02  : **1.1.1**

.. code-block:: text

              - Remove old entry samples for kernel/initrd in /boot. They are now in /boot/linx-stable etc
              - typo on hook source
              - Add execute bit for sd-boot-install-initrd

* 2022-10-02  : **1.1.0**

.. code-block:: text

              - Initial commit


