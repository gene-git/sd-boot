.. SPDX-License-Identifier: GPL-2.0-or-later

Recent Changes
==============

For the complete changelog please see *src/data/share/Changelog.rst*

**6.5.1** Sat Sep 19 06:55:31 2026 -0400

* Improve documentation for "inspect" (no functional change)
  * Update ran pages and usage outputs.
  * sd-boot-kernel-udate
  * sd-boot-efi-tool-update

* sd-boot-efi-tool-update terminal message:
  * show operation and package version (same as sd-boot-kernel-udate)

* Typo in check-permissions output.

* Recent changes now in separate file.

**6.4.2** Wed Sep 16 08:27:46 2026 -0400

* Update Readme (esp. dracut config section)

**6.4.1** Mon Jul 27 12:41:40 2026 -0400

* Update src/.clang-tidy
* Add nvcheker to the AUR package not just packaging directory in the git repo

**6.4.0** Fri Jul 17 11:03:38 2026 -0400

* BugFix: sd-find-boot was not showing output unless verbose = 2 in version 6.3.0.
* Change how older efi-tool versions are removed when updating/(re)installing:
    - Scan for older versions which is more robust than a file database tracker.
    - When multiple older versions are found, they are all removed
* Uses libalpm instead of spawning pacman to get some package info.
  Note libalpm uses libcurl/libcrypto from which valgrind
  finds some (benign) reachable memory leaks. We treat any memory leak as
  an error, including any from external libraries. Add valgrind suppression rules 
  to ignore these specific ones.


**6.3.0** Mon Jul 13 09:33:07 2026 -0400

* Copy file with kernel mediated copy_file_range: 
  If copy_file_range() is not available for the pair of source and destination
  devices and filesystems, fallback to (slower) read source and write destination.
* Bug fix: file that tracks previous version can fail to be updated

**6.2.0** Thu Jul 9 06:55:47 2026 -0400

* Comment on building package: 
  PKGBUILD uses -Dcpu_level=x86-64-v3.
  If deploying on pre-haswell / pre-2013 machine, then change the 
  build option to -Dcpu_level=baseline or remove this flag.

* efifs-update: 
  - copy files directly instead of calling rsync.
  - installed files are now timestamped with current time.
  - faster with fewer system calls.
  - Fix (benign) double // in pathname.
* Drop all include <stdbool.h> (not needed with C23).

**6.1.0** Sun Jul 5 20:14:53 2026 -0400

* Bug fix installing efifs drivers: typo in destination directory name. 
* Improve build options / hardening.
* efi-tool (always uses bls layout) - simplify the bls initialization.

  Assisted-by: Claude (Anthropic) <https://claude.ai>

**6.0.0** Sat Jul 4 15:20:33 2026 -0400

* Happy 250th USA!
* Small change to how meson installs config/hooks/man pages
  No change in the end result just the meson.build.
* Read all kernel-install "install.conf" files including drop-ins to get layout etc.,
* Spawning processes - improve code.
* Code review wiht focus on exception handling
* Config use libcyaml.
* Add .nvchecker.toml file (pkgctl version check)

