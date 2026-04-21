#!/usr/bin/bash
# SPDX-License-Identifier: GPL-2.0-or-later 
# Read the file : /var/lib/sd-boot/<pkgbase>.version
# File contains
#   current = <current_version>
#   previous = <previous_version>
#
# File contains the version of the most recently installed package.
# and previous version (may be empty if no package updates since install)
# The version is created for specific packages by the alpm hook
# 98-sd-boot-package-version-update.hook
#
# Includes memtest86_64-git, edk2-shell, our kernels and possibly others.
#
# Comments in file are ignored
# If duplicate entries found (should never happen) the last entry prevails.
#
# Args:
#   $1 = package name
#   $2 = namerefs variable to be filled with current_version
#   $3 = namerefs variable to be filled with previous_version
#   $4 = root directory - optional. Defaults to "/"
#
sd_boot_load_package_versions() {
    # 
    # read existing versions 
    # - current version which will become the new "previous" version
    # - handle old style file with just 'version' and not key = value
    #
    local _pkg="$1"
    local -n _curr_vers="$2"
    local -n _prev_vers="$3"
    local _root="$4"

    if [[ -z "$_root" ]] ; then
        _root="/"
    fi

    local dest_dir="${_root}"var/lib/sd-boot
    local file="$dest_dir/$_pkg".version

    _curr_vers=''
    _prev_vers=''

    if [[ ! -f "$file" ]] ; then
        # no file , so no known versions
        # echo "XXX no file : $file"
        return
    fi

    # read the file
    local rows=()
    while read -r row; do
        [[ "$row" =~ ^#.*$ ]] || [[ -z "$row" ]] && continue

        row=$(echo "$row" | /usr/bin/xargs)
        rows+=("$row")
    done < "$file"

    num_rows="${#rows[@]}"
    if ((num_rows < 1)) ; then
        return
    fi

    # 
    # older versions had only 1 row (curr_vers)
    # Backward compat only - delete once no longer needed
    #
    if ((num_rows == 1)) ; then
        # echo "old version: "
        key="${rows[0]}"
        _curr_vers="$key"
        return
    fi

    # parse the file
    # fresh installs will ahve 2 rows (current = curr_vers, previous = "")
    # if dup lines keep last 
    #
    for row in "${rows[@]}"
    do
        IFS="=" read -ra parts <<< "$row"
        count="${#parts[@]}"
        # echo "row: \"$row\": count=$count"

        if (( count > 0 )) ; then
            key="${parts[0]}"
            val="${parts[1]}"
            key=$(echo "$key" | /usr/bin/xargs)
            val=$(echo "$val" | /usr/bin/xargs)
            # echo "key=\"$key\" val=\"$val\""

            case "$key" in
                "current")
                    _curr_vers="${val}"
                    ;;

                "previous")
                    _prev_vers="${val}"
                    ;;

            esac
        fi
    done
}

