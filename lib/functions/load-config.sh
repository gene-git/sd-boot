#!/usr/bin/bash
# SPDX-License-Identifier: GPL-2.0-or-later 
# Read the config file : /etc/sd-boot/config
# 
# Args 
#   - config (associative array to fill out
#   - root
#
sd_boot_load_config() {
    # read config file
    local -n _config="$1"
    local _root="$2"

    if [[ -z "$_root" ]] ; then
        _root="/"
    fi

    local file="${_root}etc/sd-boot/config"

    if [[ ! -f "$file" ]] ; then
        return
    fi

    while IFS='=' read -r key value; do
        # Skip comments / empty lines
        [[ "$key" =~ ^#.*$ ]] || [[ -z "$key" ]] && continue

        # Trim whitespace
        key=$(echo "$key" | /usr/bin/xargs)
        value=$(echo "$value" | /usr/bin/xargs)

        _config["$key"]="$value"

    done < "$file"
}
