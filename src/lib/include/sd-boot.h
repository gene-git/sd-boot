// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * sd-boot.h
 */
#ifndef SD_BOOT_H
#define SD_BOOT_H

#include <linux/limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/stat.h>


/*
 * Constants
 */
enum CONF_LEN {
    MAX_KEY_LEN = 64,
    MAX_VAL_LEN = 256,
    ROW_MAX = 256,
    MSG_MAX = 256,
    CMD_MAX = 4096,
    BASE_10 = 10,
    ALIGN_16 = 16,
    ALIGN_64 = 64,
    ALIGN_128 = 128,

};

/*
 *
 * Message Options (Verbosity)
 */
enum MsgVerb{
    MSG_ERR = -1,
    MSG_VERB = 0,
    MSG_NORMAL = 1,
    MSG_QUIET = 2,
};

/*
 *
 * Command line kernel-install operations
 */
typedef enum KernelInstallOper {
    ARG_SZ = 16,
    KI_BAD = 0,
    KI_ADD = 1,
    KI_REMOVE = 2,
    KI_INSPECT = 3,
    KI_ADD_ALL = 4,
    KI_LIST = 5,
} KernelInstallOper;


/*
 * Array of pointers to strings.
 * Each string has it's own length.
 * num_rows_used - convenience for caller when allocting
 * rows in chunks to reduce number of malloc calls.
 */
typedef struct {
    size_t num_rows;
    size_t num_rows_used;
    char **rows;
    size_t *row_len;

} Array_str;

/*
 * Dynamic string
 */
typedef struct {
    size_t num_used;
    size_t num_alloc;
    char *bytes;
} Dynamic_str;


/*
 * For Development & Testing
 */
typedef struct {
    uid_t euid;
    bool test;

    char root[PATH_MAX];

    /*
     * env_boot_root used by all
     * env_kernel_plugins used by kernel-update
     * efi-tool-update uses it's own list of plugins.
     */
    Array_str env_boot_root;
    Array_str env_all_plugins;

} DevInfo;

/*
 * Config Data
 * 
 * 2 Parts:
 * (a) list of elements
 * (b) development info (with testing root tree, ...)
 *
 * Each tool loads config and passes to 
 * each support routine (avoid statics please)
 */

/*
 * Config element
 */
typedef enum { 
    CONF_INT, 
    CONF_STR, 
    CONF_FLOAT 
} KvElemType;

typedef struct {
    // each item in config file.
    // key / value (int or string)
    char *key;
    KvElemType type;
    int max_str_len;
    int lo_val_int;
    int hi_val_int;
    union {
        int v_int;
        char v_str[MAX_VAL_LEN];
    } val;

} KvElem;

/*
 * ours from /etc/sd-boot/config
 * kern_install from /etc/kernel/install.conf
 */
typedef struct {
    DevInfo info;

    int verb;
    bool is_uki;
    char *layout;
    char *initrd_generator;
    char *uki_generator;

    KernelInstallOper oper;
    char *oper_str;

    char *kernel_conf_dir;
    char *kernel_conf_bls_dir;

} SdBoot;


/*
 *
 * Package Version
 */
typedef struct {
    char pkg[MAX_VAL_LEN+1];
    char current[MAX_VAL_LEN+1];
    char previous[MAX_VAL_LEN+1];
} PackageVersion;   // no packed



/*
 *
 * Kernel-Install Plugin
 */
typedef struct {
    // args 
    char *command;
    char *kernel_version;
    char *entry_dir_abs;
    char *kernel_image;

    char *initrd_1;
    char *initrd_2;

    // environ
    char *layout;
    char *verbose;
    char *machine_id;
    char *entry_token;
    char *boot_root;
    bool is_uki;

    char *loader_entry_dir;
    char *loader_entry_file;

    // kernel or efi tool
    bool is_kernel;
    bool is_efi_tool;

} KIplugin;

// How to modify a loader entry
typedef struct {
    bool is_efi_tool;
    bool is_kernel;

    char *loader_entry_dir;
    char *loader_entry_file;
    char *title;
} LoaderEntry;


/*
 *
 * Kernel Module info
 * Derived from kernel module directory
 * mod_dir = /usr/lib/modules/<kern-vers>
 * Files in mod_dir : sdb-pkgbase, pkgbase
 * package name is read from sdb-pkgbase or pkgbase
 *
 * kmod_dir =>
 * - package (from sdb-packbase, pkgbase)
 * - kern_vers (from mod_dir)
 * Note: plugins use KIplgiin which gets kern_vers/kern_image from by kernel-install.
 */
typedef struct {
    char *image;
    char *mod_dir;
    char *package;
    char *vers;

} KernelInfo; 

/*
 *
 * Kernel Trigger Info
 *
 * triggers are pathname or package names passed in
 * from ALPM hooks - each is provided as one line
 * on stdin.
 */
typedef struct {
    // size_t num_all;
    // char **all;

    size_t num_info;
    size_t num_info_alloc;
    KernelInfo *info;

    size_t num_other;

} Triggers;

/*
 *
 * Mount Points : efi and xbootldr partitions.
 */
typedef struct {
    char efi_dir[PATH_MAX];
    char xbootldr_dir[PATH_MAX];
} MountPoints;

/*
 *
 * Library function declarations
 *
*/

// arr_str.c
int array_str_new(size_t num_rows, Array_str *arr);
int array_str_resize(size_t num_rows, Array_str *arr);
int array_str_free(Array_str *arr);

// array_str_move.c
int array_str_move(Array_str *arr_1, Array_str *arr_2);

// config.c
int load_config(SdBoot *conf);
void clean_config(SdBoot *conf);

// copy_file.c
int copy_file(const char *src, const char *dst);

// dev_info.c
int init_devinfo(DevInfo *info);
void clean_devinfo(DevInfo *info);

// efi_image.c
char *efi_image_to_package(SdBoot *conf, const char *path);
char *package_to_efi_image(SdBoot *conf, char *pkg);

// dir_dup_links.c
int dir_dup_links(const char *src, const char *dst, Array_str *skips);

// dynamic_string.c
int dynamic_str_alloc(size_t num, Dynamic_str *str);

// env.c
int combined_env(size_t num1, char **envp1, size_t num2, char **envp2, char ***envp_p);
int count_envp_argv(char *const args[]);

// file_list_glob.c
int file_list_glob(const char *pattern, Array_str *files);

// find_efi_current.c
int find_efi_current_boot(MountPoints *mounts);

// find_boot_mounts.c
int find_efi_xbootldr_mounts(Array_str *efi, Array_str *xbootldr);

// get_one_line.c
char *get_one_line(char **state_p); 

// is_kernel_image.c
int is_kernel_image_path(char *image, bool *is_kernel);

// kernel_info.c
int kernel_image_path_to_info(KernelInfo *info);
void kernel_info_free(KernelInfo *info);

// kernel_info_all.c
int kernel_info_all(size_t *num_info_p, KernelInfo **info_p);

// kernel_triggers.
int get_kernel_triggers(Triggers *trigs);
void free_triggers(Triggers *trigs);

// keyval.c
int read_kv_elems(const char *path, size_t num, KvElem *elem, size_t *num_found_p);
int alloc_kv_elems(size_t num, KvElem **elem_p);

// ki_efi_conf_bls.c
int ki_make_kernel_conf_bls(SdBoot *conf);

// ki_efi_env.c
int ki_efi_update_env(SdBoot *conf, Array_str *env);

// ki_oper.c
KernelInstallOper kernel_install_oper(char *oper);

// ki_plugin.c
int plugin_init(int argc, const char *argv[], KIplugin *plugin);
void plugin_free(KIplugin *plugin);

// ki_plugin_env.c
int ki_plugins_test_env(char *test_root, Array_str *env);
int ki_plugins_efi_update_env(char *test_root, Array_str *env);

// ki_run.c
int kernel_install_run(SdBoot *conf, char *const args[], char *const envp[]);

// loaderentry_modify_file.c 
int loaderentry_modify_file(LoaderEntry *entry);

// loaderentry_efi.c
int loaderentry_modify_efi(SdBoot *conf, KIplugin *plugin);

// loaderentry_kernel.c
int loaderentry_modify_kernel(SdBoot *conf, KIplugin *plugin);

// managed_packages.c
int load_kernel_packages(SdBoot *conf, Array_str *arr);
int load_efi_tool_packages(SdBoot *conf, Array_str *arr);

// read_file.c
int read_file(const char *path, Array_str *arr);
char *read_file_first_row(const char *path);

// read_one_line.c
int read_one_line_fd(int fdes, char *buf, size_t len);

// makedir.c
int makedir(const char *path, mode_t mode);
int makepath(const char *path, mode_t mode);

// make_symlink.c
int make_symlink(const char *target, const char *linkname);

// misc.c
void strip_file_extension(char *filename, const char *ext);
int str_to_int(char *str, int low_value, int high_value);

// msg.c
[[gnu::format(printf, 2, 3)]]
void msg(int msg_level, const char *fmt, ...);
void set_verb_level(int verb_level);

// package_versions.c
int read_package_versions(SdBoot *conf, const char *pkg, PackageVersion *pkg_vers);
int update_package_versions(SdBoot *conf, const char *pkg, PackageVersion *pkg_vers);
int remove_package_versions(SdBoot *conf, const char *pkg);

// package_version_installed.c
int package_version_installed(const char *pkg, size_t len_vers, char *vers);

// parse_kernel_triggers.c
int parse_kernel_triggers(Array_str *trigs_all, Triggers *trigs);

// path_add_slash.c
int path_add_slash(char *path, char **path_p);

// read_triggers.c
int read_triggers(Array_str *arr);

// remove_file
int remove_file(char *path);

// rm_rf.c
int rm_rf(const char *path);

// run_cmd.c
int run_cmd(char **argv, char **envp, int *child_ret_p);
int run_cmd_output(char **argv, char **envp, char **output_p, int *child_ret_p);

// string_in_list.c
bool string_in_list(const char *name, size_t num_names, char **names);

// trim_string.c
char *trim_string(char *str, size_t max_len);
void strip_whitespace(char *str);

// ukify_os_release.c
int ukify_os_release(const char *name, const char *build_id, Dynamic_str *os_release);

#endif
