.. SPDX-License-Identifier: GPL-2.0-or-later

Recent Changes
==============

For the complete changelog please see *src/data/share/Changelog.rst*

**6.9.0**

* Improve output format from subprocess (sbctl).
* Openssl 4.x requires different valgrind suppression rules 3.x
  to silence its global allocated cache not being freed.
* Tidy ups to keep clang-tidy 23.1.4 happy
* clang-format to enforce consistent style.

**6.8.0**

* Update manual on using sd-boot with Arch kernels
* Add a couple more defensive memory checks for find_boot_mounts.

**older**

* Doc update about systemd tpm2 boot warnings and possible work arounds.
* Update man page for sd-boot-find-boot-mounts
* sd-boot-find-mounts: show filesystem UUIDs

  * Expand secure boot section
  * Add section on systemd validation of the kernel (sysetmd-tpm2-setup)
  * Add section on the (benign) systemd TPM2 warning: **Failed to start Early TPM SRK Setup**

* Update readthedocs manual: https://sd-boot.readthedocs.io/

