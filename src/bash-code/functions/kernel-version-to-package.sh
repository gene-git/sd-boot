#!/usr/bin/bash
# SPDX-License-Identifier: GPL-2.0-or-later 
#
# Get the kernel package from it's version
#
# Every kernel version is unique and its extremely unlikely any other bootable efi
# program would have an identical version to any kernel version.
#
# 

sd-boot-kernel-version-to-package() {
    #
    # Args:
    #   $1 = kernel version
    #   $2 = namerefs variable to be filled with kernel package with it's version
    #
    local _kern_vers="$1"
    local -n _kern_package="$2"

    #
    # Load support functions
    # - sd_boot_load_package_version()
    #
    function_dir="/usr/lib/sd-boot/bash/functions"

    # shellcheck source=./functions/load-package-version.sh
    source "$function_dir/load-package-version.sh"

    #
    # Check any known package versions with version == _kern_vers
    #
    shopt -s nullglob
    files=(/var/lib/sd-boot/*.version)
    shopt -u nullglob

    local _vcurr=''
    local _vprev=''

    for file in "${files[@]}"
    do
        pkg="${file##*/}"
        sd_boot_load_package_versions "$pkg" _vcurr _vprev
        if [[ "$_vcurr" == "$_kern_vers" ]] ; then
            _kern_package="$pkg"
            if [[ "$pkg" != "linux"* ]] ; then
                echo "    Warning package $pkg may not be a kernel package"
            fi
            break
        fi
    done
}

