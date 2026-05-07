#!/usr/bin/bash
#
# strip_whitespace
#   Args:
#       Arg-1   = string to clean up
#   Returns:
#       output is string with leading / trailing white space removed
#
# When using bash read in a while loop one cannot use "read -rd '' xxx <<< $xxx" because
# read is non-reentrant and do so breaks things badly.
#
# So we provide simple shell bash param expansion method.
#

strip_whitespace() {
    local _string="$1"
    local _clean=''
    shopt -s extglob
    clean="${_string##+([[:space:]])}"      # leading
    clean="${clean%%+([[:space:]])}"        # trailing
    shopt -u extglob
    echo "$clean"
}
