/*
 * gcc -O2 version_check.c -o pkg_ver $(pkg-config --cflags --libs libalpm)
 */
#include <alpm.h>
#include <string.h>

#include "sd-boot-alpm.h"
#include "sd-boot-msg.h"


char *get_curr_pkg_vers_alpm(char *pkg_name) {

    char *pkg_vers = nullptr;

    if (!pkg_name) {
        return nullptr;
    }

    /*
     * Initialize
     */
    alpm_errno_t err = ALPM_ERR_OK;
    alpm_handle_t *handle = alpm_initialize("/", "/var/lib/pacman", &err);
    if (!handle) {
        msg(MSG_ERR, "! sd-boot: ALPM initialize error: %s\n", alpm_strerror(err));
        return nullptr;
    }

    /*
     * Open local package database
     * - lookup package version 
     */
    alpm_db_t *db_local = alpm_get_localdb(handle);
    if (db_local) {
        alpm_pkg_t *pkg = alpm_db_get_pkg(db_local, pkg_name);
        if (pkg) {
            const char *vers = alpm_pkg_get_version(pkg);
            if (vers) {
                pkg_vers = strdup(vers);
            }
        }
    }

    /*
     * Failing means we did not find the package version.
     */
    alpm_release(handle);
    return pkg_vers;
}

