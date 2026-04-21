#!/usr/bin/bash
# SPDX-License-Identifier: GPL-2.0-or-later 
# Read the file : 
#   /etc/sd-boot/kernel.packages
# which has the list of kernel packages using our "sd-boot" kernel-install tools.
#
sd_boot_load_kernel_packages() {
    #
    # read file with list of packages
    # Save result into array variable passed as 1st argument
    # Args:
    #   $1 = array variable to get filled with packages (namerefs)
    #   $2 = root directory - optional. Defaults to "/"
    #
    local -n _packages="$1"
    local _root="$2"
    
    if [[ -z "$_root" ]] ; then
        _root="/"
    fi

    local file="${_root}etc/sd-boot/kernel.packages"

    if [[ ! -f "$file" ]] ; then
        # echo "sd_boot_load_kernel_packages: no file $file"
        return
    fi

    _packages=()
    while IFS='=' read -r row; do
        # Skip comments / empty lines
        [[ "$row" =~ ^#.*$ ]] || [[ -z "$row" ]] && continue

        # Trim whitespace
        row=$(echo "$row" | /usr/bin/xargs)
        _packages+=("$row")

    done < "$file"
}
# export -f sd_boot_load_kernel_packages

