Changelog
=========

Tags
====

.. code-block:: text

	3.6.2 (2026-04-21) -> 5.11.0 (2026-06-29)
	73 commits.

Commits
=======


* 2026-06-29  : **5.11.0**

.. code-block:: text

              - **5.11.0**
            
                * Remove *sd-boot* as a trigger in hooks - it is not needed.
            
                * Change location of dracut and kernel-install config files from /etc to:
            
                  - /usr/lib/kernel/install.conf.d/
                  - /usr/lib/dracut/dracut.conf.d
            
                  They were incorrectly installed in /etc/dracut.conf.d and /etc/kernel/
            
                * Updating all managed packages.
            
                  - *sd-boot-kernel-update* and *sd-boot-efi-update* may now be applied to
                    all kernels and efi-tools managed by sd-boot. This is done by using
                    "--all--" as the package name instead of one specific package.
            
                * Code improvements.
                  - more code reorg and simplifications.
                  - additional checks reading triggers from stdin.
                  - tests that must be run sequentially are now grouped into "suites".
                    tests in each suite are run sequentially.
                    Assists testing non-trivial "remove" by ensuring that an "add" is
                    completed before the remove.

* 2026-06-22  : **5.10.0, origin/master**

.. code-block:: text

              - **5.10.0**
            
                * Replace remaining strncpy() with strlcpy()
                * Small man page makefile tweak.

* 2026-06-21  : **5.9.0**

.. code-block:: text

              - **5.9.0**
            
                * Kernel plugins can now be de-activated in */etc/sd-boot/config.yaml*
                  See the *Skipping Kernel Plugins* section below for more detail.
            
                * With uki layout and ukify as the uki_generator, the *91-sbctl.install* plugin
                  is not needed and is now automatically disabled.
            
                  Because ukify handles signing the uki image file, there is no need for the
                  sbctl plugin in this case, even if it does nothing.
                  This is a precaution to guarantee the image is signed exactly once.
            
                * Drop */etc/sd-boot/config*
            
                  Old (toml format) config has been replaced by or auto migrated to *config.yaml* in version 5.5.0.
                  Any older config file would already have been auto converted.
            
                * More Code Reorg
            
                  - split header files
                  - additional sub directories
                  - meson.build file per subdir
                  - Array_str consistently used for all argv/envp lists.
            
                * Drop a loop which led to a benign valgrind complaint of a benign UMR
            
                  An unitialized memory read (UMR) can happen with optimized code and this one
                  stems from AVX over-runs.
            
                  Confirmed by compiling the file using scalar version of loop (no-tree-vectorize).
                  After removing the vectorization, th valgrind UMR goes away.
            
                  While this UMR is benign, we chose to remove the loop entirely since
                  we want all code to be valgrind clean.
            
                * Use clean environment whenever execve() runs any executable.
            
                  When running in test mode, the Environment is expanded
                  with a few additional variables (like LD_LIBRARY_PATH). This one
                  ensures that newly build shared libraries are used during tests.
                  When not in testing mode, only the clean base environment is used.
            
                * Additional verbose control:
            
                  In /etc/sd-boot/config.yaml, setting verb >= 3 adds the "-v" option to *kernel-install*.
                  This provides a simple way to get more detailed infomation. It can be quite verbose
                  so avoid unless needed.
            
                * Test mode: Loader Entry Change
            
                  - Applies only to:
                     - test mode for efi tools and kernels using bls layout.
            
                  - 90-loaderentry.install plugin locates the boot / esp mount point.
            
                    However, in test mode, this belongs to the root of the testing directory.
                    This results in the loader entry file content having quite long path names.
                    This is cosmetic as it never actually used. The test goal is to validate the output
                    not boot from it.
            
                    In normal production use this is all fine.
            
                    It works by test mode providing an additional env variable to the plugin.
            
                * Tools that need EFI mount point but there is none.
            
                  - If the testing environment (chroot for example) prevents identifyig the ESP mount,
                    point, then these tests are now skipped.
            
                  - Affect sd-boot-efifs-update and sd-boot-efi-tool-update.
            
                * Building in chroot
                  - With above changes the tests in PKGBUILD check() should work in a chroot.
 2026-06-11   ⋯

.. code-block:: text

              - update Changelog

* 2026-06-11  : **5.8.1**

.. code-block:: text

              - **5.8.1**
            
                * Man page update.
                  Update sd-boot-find-boot-mounts man page with additional information about
                  multiple ESP partitions and possible mismatch of nvram with current ESP as observed
                  on some machines.
 2026-06-09   ⋯

.. code-block:: text

              - update Changelog

* 2026-06-09  : **5.8.0**

.. code-block:: text

              - **5.8.0**
            
                * sd-boot-find-boot-mounts: Handling Multiple ESP partitions (multi-disk).
            
                  sd-boot-find-boot-mounts has been improved to display additional information.
            
                  On systems with multiple ESP partitions where the motherboard boot loader boots
                  from a different partition than the expected partition leads to some output ambiguity.
            
                  This should not happen and on most machines it will not.
                  But it can and when it does then *bootctl status* displays:
            
                    WARNING: The boot loader reports a different partition UUID than the detected ESP.
            
                  To resolve this we use bootctl to detect the "current" ESP.
            
                  Ive seen this on happen 2 lenovo machines. What seems to be going on is that
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

* 2026-06-08  : **5.7.1**

.. code-block:: text

              - update Changelog
              - **5.7.1**
            
                * Update sd-boot-find-boot-mounts
                  - now works for non-root users
                  - udev/libmount instead of low level libblk
                  - Adds systemd-libs dependency.
                  - Simpler, more robust and significantly faster.
            
                Timing example Machine with 2 disks (2 ESP, 2 XBOOTLDR):
            
                Previous:
                  # time sd-boot-find-boot-mounts
            
                  1st:
                    real    0.029 - 0.046s
                    user    0.009 - 0.006s
                    sys     0.020 - 0.012s
            
                New:
                    # time sd-boot-find-boot-mounts
            
                    1st:
                    real    0.003s
                    user    0.000s
                    sys     0.003s
            
                Some caching likely especially with previous approach.
 2026-06-06   ⋯

.. code-block:: text

              - update Changelog

* 2026-06-06  : **5.6.2**

.. code-block:: text

              - **5.6.2**
            
                * Install sd-boot-find-boot-mounts in /usr/bin.
                * Documentation
                    * Add man pages for command line tools.
                    * Update readme.
                * No functional change.
 2026-06-05   ⋯

.. code-block:: text

              - Update Changelog
              - Tweak readme file
 2026-06-04   ⋯

.. code-block:: text

              - update Changelog

* 2026-06-04  : **5.6.0**

.. code-block:: text

              - **5.6.0**
            
                * The loader entry plugin is now installed in /usr/lib/kernel/install.d.
                  This allows administrators to over-ride in /etc/kernel/install.d/xxx.
            
                  i.e. /usr/lib/kernel/install.d/95-sd-boot-loaderentry-modify.install
            
                * Switch /etc/sd-boot/config to /etc/sd-boot/config.yaml
                  Yaml config will be auto created from existing config.
                  This adds a new dependency on *libyaml*.
            
                * Package dependencies updated.
                * Some code tidy up. Simplify style used to check for nullptr.
 2026-06-03   ⋯

.. code-block:: text

              - update Changelog

* 2026-06-03  : **5.4.1**

.. code-block:: text

              - **5.4.1**
            
                * Some code clean ups
              - update Changelog

* 2026-06-03  : **5.4.0**

.. code-block:: text

              - **5.4.0**
            
                * Fix bug failing to create /var/lib/sd-boot
                  Resolves issue #1 reported by @coolreader18
 2026-06-02   ⋯

.. code-block:: text

              - update Changelog

* 2026-06-01  : **5.3.0**

.. code-block:: text

              - one more file
              - ** 5.3.0
            
                * Fix missing checkdepends() - test suite relies on edk2-shell.
                  Fixes build failing when edk2-shell not installed.
                  Thanks to @coolreader18 for aur report.
                * Avoid another double "//"
              - update Changelog

* 2026-06-01  : **5.2.0**

.. code-block:: text

              - **5.2.0**
            
                * Shadow "install.con" for non-kernels now sets generator in addition to layout = bls.
                  Also fixes missing "newline" bug.
 2026-05-31   ⋯

.. code-block:: text

              - update Changelog

* 2026-05-31  : **5.1.0**

.. code-block:: text

              - **5.1.0**
            
                * Avoid double "//" in efi tool shadow directory path name.
                * Error messages to stderr instead of stdout.
                * Code structure re-org.
                * More code tidy ups.
                * Add standalone tools (installed in /usr/bin):
            
                  * sd-boot-efi-tool-update <oper> <package-name>
                  * sd-boot-kernel-update <oper> <package-name>
            
                  Where <oper> is add or remove or inspect (see man kernel-install).
                  Note that <package-name> must be already installed via pacman.
                * Improve way shadow "install.conf" is created. If file exists and is correct then
                  dont write it again.
 2026-05-30   ⋯

.. code-block:: text

              - update Changelog

* 2026-05-30  : **4.9.1**

.. code-block:: text

              - **4.9.1**
            
                * Fix broken efi tool installin "uki" layout.
            
                  efi tool installs only work in bls layout. kernel-install (as of systemd version 260)
                  provides no clear way to specify the layout at run time. We work around this for
                  (non-kernel) efi tools by using a "shadow" kernel-install config with bls layout.
            
                  Please reinstall any efi tools (e.g. pacman -Syu edk2-shell) to ensure latest version
                  is installed in $BOOT.
            
                * Efi tool ALPM hook now triggers on sd-boot.
            
                * Set default layout to "uki" in /etc/kernel/install.conf
            
                  May need to be changed manually.
            
                * Some code tidy ups.
 2026-05-29   ⋯

.. code-block:: text

              - update Changelog

* 2026-05-29  : **4.8.0**

.. code-block:: text

              - **4.8.0**
            
                * Fix installer typo for loaderentry modify.
              - update Changelog

* 2026-05-29  : **4.7.5**

.. code-block:: text

              - fix license install typo
              - update Changelog

* 2026-05-29  : **4.7.4**

.. code-block:: text

              - **4.7.4**
            
                * loader flags to meson build
              - update Changelog

* 2026-05-29  : **4.7.3**

.. code-block:: text

              - **4.7.3**
            
                * Change build tooling to meson (was cmake).
                  Simpler and cleaner.
                * Use meson to run tests.
                * No functional change.
 2026-05-25   ⋯

.. code-block:: text

              - update Changelog

* 2026-05-25  : **4.7.2**

.. code-block:: text

              - **4.7.2**
            
                * Loaderentry: take "layout" provided by kernel-install since KERNEL_INSTALL_LAYOUT env. can override
                  /etc/kernel/install.conf. This is more robust approach. The layout is only used to avoid an
                  unnecessary fork/exec when layout is *uki*.
                * When kernel is removed, the display message is now more explicit on *remove* vs *add*.
                * Some code tidy ups.
                * No functional change.
 2026-05-19   ⋯

.. code-block:: text

              - update Changelog

* 2026-05-19  : **4.7.1**

.. code-block:: text

              - **4.7.1**
            
                * Change PKGBUILD arch to x85_64 not any.
                * Change linking to only use libblkid where needed.
                * Clean up cruft from CMakeLists.txt
              - update Changelog

* 2026-05-19  : **4.7.0**

.. code-block:: text

              - **4.7.0**
            
                * Read /etc/kernel/install.conf so code is aware of layout (bls or uki).
                * Loader entry plugin now exits earlier for uki kernels which has no loader entries.
                * kernel-install remove does not need kernel image
                * Add sbctl and systemd-ukify as dependencies
                * Little code tidy
 2026-05-18   ⋯

.. code-block:: text

              - update Changelog

* 2026-05-18  : **4.6.0**

.. code-block:: text

              - **4.6.0**
            
                * Loader entries are not used for UKI layout.
                  Turn off messages about missing loader entries.
            
                **4.5.0**
            
                * Enhance sd-boot-find-mounts using liblkid (new dependency)
                * Kernel ALPM hook trigger update.
              - update Changelog

* 2026-05-18  : **4.4.0**

.. code-block:: text

              - **4.4.0**
            
                * Add secure boot signing key changes trigger to ALPM hook to update kernel.
                * Messages more consistent with prefixes (! = error, ⦁ = top level, ↳ = sub level).
                * Add brief section on signing kernels and secure boot.
                * Fix c-code checker (clang-tidy, valgrind)
                * Simplify some code (readability, maintainability)
                * Additional boundary case checks.
                * Loader entry modifier for efi tools handle "uki" row in addition to "linux"
 2026-05-15   ⋯

.. code-block:: text

              - update Changelog

* 2026-05-15  : **4.3.0**

.. code-block:: text

                  - * Drop the bash code version and sd-boot-set-alternative since its no longer needed.
                    * Simplify installation.
                    * When adding a new kernel version, check if previous version still there before
                      calling *kernel-install remove* to remove the previous version since it may have
                      already been removed by pacman. No functional impact other than avoiding one
                      unnecessary fork/exec.
                    * Remove one unnecessary trigger from kernel update alpm hook.
 2026-05-07   ⋯

.. code-block:: text

              - update Changelog

* 2026-05-07  : **4.2.2**

.. code-block:: text

              - Small code comment clean ups - no functional change
              - delete old now unused directories
              - update Changelog

* 2026-05-07  : **4.2.1**

.. code-block:: text

              - Make sure test suite runs in fresh set up - fixed
              - update Changelog

* 2026-05-07  : **4.2.0**

.. code-block:: text

              - tidy up for 4.2.0 release
              - **4.0.1**
            
                * New C-code alternative to bash tools..
            
                  Activate: /usr/lib/sd-boot/set-alternative binary
                  Bash:     /usr/lib/sd-boot/set-alternative bash
            
                  Bash version remains available for now, and can be re-activated
                  using *set-alternative*.
            
                  See Release for details
 2026-04-30   ⋯

.. code-block:: text

              - update Changelog

* 2026-04-30  : **3.9.0**

.. code-block:: text

              - Plugins - simplify getting the loader entry file location
                Fix efi-tool package remove buglet getting package vers (only impacts memtest86 removal)
 2026-04-24   ⋯

.. code-block:: text

              - update Changelog

* 2026-04-24  : **3.8.0**

.. code-block:: text

              - Tidy up sd-boot-efifs-update a bit
 2026-04-22   ⋯

.. code-block:: text

              - update Changelog

* 2026-04-22  : **3.7.0**

.. code-block:: text

              - Fix typo in print statement
 2026-04-21   ⋯

.. code-block:: text

              - update Changelog

* 2026-04-21  : **3.6.6**

.. code-block:: text

              - PKGBUILD : tweak the description
              - update Changelog

* 2026-04-21  : **3.6.5**

.. code-block:: text

              - Drop our memtest package from optdepends until we release it
              - update Changelog

* 2026-04-21  : **3.6.4**

.. code-block:: text

              - PKGBUILD should be arch "any"
              - update Changelog

* 2026-04-21  : **3.6.3**

.. code-block:: text

              - PKGBUILD - no .install needed
              - update Changelog

* 2026-04-21  : **3.6.2**

.. code-block:: text

              - Add arch PKGBUILD
              - initial commit of sd-boot


