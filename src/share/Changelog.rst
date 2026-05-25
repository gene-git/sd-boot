Changelog
=========

Tags
====

.. code-block:: text

	3.6.2 (2026-04-21) -> 4.7.2 (2026-05-25)
	36 commits.

Commits
=======


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


