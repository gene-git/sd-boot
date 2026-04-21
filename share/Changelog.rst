Changelog
=========

Tags
====

.. code-block:: text

	3.1.0 (2026-04-18) -> 3.4.1 (2026-04-20)
	26 commits.

Commits
=======


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
              - rework the installer scripts to deal with package version updates ...
              - duh
              - installer change for new hook priority
              - alpm hook to update package version moved ahead of kernel install hook
              - update etc/kernel/install.d/95-loaderentry-kernel.install
              - installer update
              - Rework tools - when package is updated it is our job to remove the previous version. ...
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
              - Major changes ...
 2026-04-14   ⋯

.. code-block:: text

              - Using kernel-install: linux-custom, linux-next and linux-test
              - update Docs/Changelog.rst


