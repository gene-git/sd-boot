.. SPDX-License-Identifier: GPL-2.0-or-later

Recent Changes
==============

For the complete changelog please see *src/data/share/Changelog.rst*

**6.12.0**

* Speed up initrd by changing dracut compression level from 15 to 3. 
  The bulk of the initrd is already compressed kernel modules. For example
  the initrd generation takes less than half the time of the higher compression level
  with a UKI size increase of roughly 4%.

  You can override dracut configuration settings as usual using, for example, 
  /etc/dracut.conf.d/90-compress.conf containing: compress_level_zstd="15"

**6.11.0**

* When using new (but currently unused) timeout option to run an executable, forward parent signals.
  A keyboard interrupt, for example, is now passed to the child process group.
* Split run_cmd_output.c into smaller files (readability)
* Add manual section discussing posix_spawn vs fork/exec

**6.10.0**

* Improve output format from subprocesses by capture of child process stdout/stderr
  and formatting appropriately indented.
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

