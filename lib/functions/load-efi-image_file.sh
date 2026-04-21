#!/usr/bin/bash
# SPDX-License-Identifier: GPL-2.0-or-later 
# Support for efi tools that are installed into the EFI
#

read_file_row() {
    #
    # Read file and return first non-empty, non-comment row
    #
    local _file="$1"
    local -n _row="$2"

    _row=''
    if [[ -z "$_file" ]] ; then
        return
    fi

    while read -r row; do
        # Skip comments / empty lines
        [[ "$row" =~ ^#.*$ ]] || [[ -z "$row" ]] && continue

        # Trim whitespace
        _row=$(echo "$row" | /usr/bin/xargs)
        break
    done < "$_file"
}

sd_boot_load_efi_image_file() {
    #
    # Return the ".efi" image file give it's package name
    #
    # Read the file : /etc/sd-boot/<pkgbase>.image
    # File contains path the efi image file.
    # Comments ignored
    # Everything after the first non-comment is ignored
    # Args:
    #   $1 = package name
    #   $2 = namerefs variable to be filled with image path
    #   $3 = root directory - optional. Defaults to "/"
    #
    local _pkgbase="$1"
    local -n _efi_image="$2"
    local _root="$3"
    
    if [[ -z "$_root" ]] ; then
        _root="/"
    fi

    local file="${_root}etc/sd-boot/${_pkgbase}.image"

    _efi_image=''
    if [[ ! -f "$file" ]] ; then
        #echo "sd_boot_load_efi_image_file: no file $file"
        return
    fi
    
    _efi_image=""
    read_file_row "$file" _efi_image
}

sd_boot_efi_image_to_package() {
    #
    # Return the package give the ".efi" image file
    #
    # Args:
    #   $1 = .efi image path
    #   $2 = namerefs variable to be filled with package name
    #
    local _efi_image="$1"
    local -n _pkgbase="$2"
    local _root="$3"

    _pkgbase=''
    if [[ -z "$_efi_image" ]] ; then
        return
    fi

    if [[ -z "$_root" ]] ; then
        _root="/"
    fi


    local path_dir="${_root}etc/sd-boot"
    for path in "${path_dir}"/*.image
    do
        this_efi_image=""
        read_file_row "$path" this_efi_image

        # echo "$path -> $this_efi_image"
        if [[ "$this_efi_image" == "$_efi_image" ]] ; then
            file="${path##*/}"
            _pkgbase="${file%.image}"
            return
        fi
    done
}

