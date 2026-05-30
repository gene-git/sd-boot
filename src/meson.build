#
# sd-boot
#
# meson setup --reconfigure build/release --buildtype=release -Ddefault_library=shared
# meson compile -C build/release
# meson install -C build/release --destdir <xxx>
#
# meson setup --reconfigure build/debug --buildtype=debug -Ddefault_library=static -Dstrip=false
# meson compile -C build/debug
# meson install -C build/debug --destdir <xxx>
# Alternative is: DESTDIR=<full-path> ninja -C build/debug install
# Note this strips debug symbols
#
# meson test -C build/release --setup=standard [test_x]
# meson test -C build/release --setup=valgrind [test_x]
#
project('sd-boot', 'c',
  version: files('version.txt'),
  default_options: ['c_std=c23', 'warning_level=2'],
)

my_cflags = [
  '-D_GNU_SOURCE',
  '-D_XOPEN_SOURCE=500',
  '-march=x86-64-v3',          # Optimized for modern (AVX2-capable) CPUs
  '-mtune=generic',
  '-mshstk',                   # Intel Shadow Stack protection
  '-pipe',                     # Speeds up compilation
  '-fno-plt',                  # Optimization for shared library calls
  '-fexceptions',
  '-Wformat=2',
  '-Werror=format-security',   # Rejects insecure printf format strings
  '-fstack-clash-protection',  # Prevents stack-clash exploits
  '-fcf-protection',           # Control-flow enforcement
  '-fvisibility=default',      # Ensures global functions remain exported
  '-Wall',
  '-Wextra',
  '-Wpedantic',
  '-Wshadow'
]

if get_option('buildtype') != 'debug'
  my_cflags += ['-O2', '-Wp,-D_FORTIFY_SOURCE=3']
endif

my_ldflags = [
  '-Wl,-O1',
  '-Wl,--sort-common',
  '-Wl,--as-needed',
  '-Wl,-z,relro',
  '-Wl,-z,now',
  '-Wl,-z,pack-relative-relocs',
]


#
# Source code
#
inc_dir = include_directories('lib/include')

lib_sources = files(
    'lib/array_str.c',
    'lib/array_str_move.c',
    'lib/config.c',
    'lib/copy_file.c',
    'lib/dev_info.c',
    'lib/dir_dup_links.c',
    'lib/dynamic_string.c',
    'lib/efi_image.c',
    'lib/env.c',
    'lib/file_list_glob.c',
    'lib/find_efi_current.c',
    'lib/get_one_line.c',
    'lib/is_kernel_image.c',
    'lib/kernel_info_all.c',
    'lib/kernel_info.c',
    'lib/kernel_triggers.c',
    'lib/keyval.c',
    'lib/ki_efi_conf_bls.c',
    'lib/ki_efi_env.c',
    'lib/ki_oper.c',
    'lib/ki_plugin.c',
    'lib/ki_plugin_efi_env.c',
    'lib/ki_plugin_test_env.c',
    'lib/ki_run.c',
    'lib/loaderentry_efi.c',
    'lib/loaderentry_kernel.c',
    'lib/loaderentry_modify_file.c',
    'lib/make_dir.c',
    'lib/make_symlink.c',
    'lib/managed_packages.c',
    'lib/misc.c',
    'lib/msg.c',
    'lib/package_version_installed.c',
    'lib/package_versions.c',
    'lib/parse_kernel_triggers.c',
    'lib/path_add_slash.c',
    'lib/read_file.c',
    'lib/read_one_line.c',
    'lib/read_triggers.c',
    'lib/remove_file.c',
    'lib/rm_rf.c',
    'lib/run_cmd.c',
    'lib/run_cmd_output.c',
    'lib/string_in_list.c',
    'lib/trim_string.c',
    'lib/ukify_os_release.c',
    )

#
# Libs
#
sd_boot_lib = library('sd_boot',
  sources: lib_sources,
  include_directories: inc_dir,
  c_args: my_cflags,
  link_args: my_ldflags,
  install: true,
  install_dir: '/usr/lib'
)

sd_boot_dep = declare_dependency(
  link_with: sd_boot_lib,
  include_directories: inc_dir
)

blkid_dep = dependency('blkid', required: true)

#
# Executables
#
find_boot_mounts = executable('sd-boot-find-boot-mounts',
  sources: ['tools/find-boot-mounts.c', 'lib/find_boot_mounts.c'],
  dependencies: [sd_boot_dep, blkid_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: true,
  install_dir: '/usr/lib/sd-boot'
)

efifs_update = executable('sd-boot-efifs-update',
  sources: 'tools/efifs-update.c',
  dependencies: sd_boot_dep,
  c_args: my_cflags,
  link_args: my_ldflags,
  install: true,
  install_dir: '/usr/lib/sd-boot'
)

efi_tool_update = executable('sd-boot-efi-tool-update',
  sources: 'tools/efi-tool-update.c',
  dependencies: sd_boot_dep,
  c_args: my_cflags,
  link_args: my_ldflags,
  install: true,
  install_dir: '/usr/lib/sd-boot'
)

kernel_update = executable('sd-boot-kernel-update',
  sources: 'tools/kernel-update.c',
  dependencies: sd_boot_dep,
  c_args: my_cflags,
  link_args: my_ldflags,
  install: true,
  install_dir: '/usr/lib/sd-boot'
)

loaderentry_modify = executable('95-sd-boot-loaderentry-modify.install',
  sources: 'plugins/loaderentry-modify.c',
  dependencies: sd_boot_dep,
  c_args: my_cflags,
  link_args: my_ldflags,
  install: true,
  install_dir: '/etc/kernel/install.d'
)

#
# Configs / Hooks
#
install_data('etc/dracut.conf.d/010-dracut.conf', install_dir: '/etc/dracut.conf.d')

install_data('etc/kernel/install.conf', install_dir: '/etc/kernel')
install_data('etc/kernel/ukify.conf', install_dir: '/etc/kernel')

install_data('etc/sd-boot/config', install_dir: '/etc/sd-boot')
install_data('etc/sd-boot/edk2-shell.image', install_dir: '/etc/sd-boot')
install_data('etc/sd-boot/efi-tool.packages', install_dir: '/etc/sd-boot')
install_data('etc/sd-boot/kernel.packages', install_dir: '/etc/sd-boot')
install_data('etc/sd-boot/memtest86_64-git.image', install_dir: '/etc/sd-boot')

install_data('hooks/60-sd-boot-kernel-remove.hook', install_dir: '/usr/share/libalpm/hooks')
install_data('hooks/69-sd-boot-efifs-remove.hook', install_dir: '/usr/share/libalpm/hooks')
install_data('hooks/70-sd-boot-efi-tool-remove.hook', install_dir: '/usr/share/libalpm/hooks')
install_data('hooks/90-sd-boot-kernel-install.hook', install_dir: '/usr/share/libalpm/hooks')
install_data('hooks/99-sd-boot-efifs-install.hook', install_dir: '/usr/share/libalpm/hooks')
install_data('hooks/99-sd-boot-efi-tool-install.hook', install_dir: '/usr/share/libalpm/hooks')

install_data('share/Changelog.rst', install_dir: '/usr/share/sd-boot')
install_data('share/loader.conf', install_dir: '/usr/share/sd-boot')
install_data('share/Readme.rst', install_dir: '/usr/share/sd-boot')
install_data('share/License/License-gpl-2.0.txt', install_dir: '/usr/share/sd-boot/License')
install_data('share/License/License.rst', install_dir: '/usr/share/sd-boot/License')

install_data('share/License/License.rst', install_dir: '/usr/share/licenses/sd-boot')

#
# Tests: set up
# - standard test and tests run under valgrind
# - all tests use the sd-boot test env 
# - see scripts: init-test-setup, run_one_test
# --errors-for-leak-kinds=all
#

source_dir = meson.current_source_dir()
build_dir = source_dir / 'build'
testing_dir = source_dir / 'tests/Testing'

run_one_test = find_program(source_dir / 'tests/scripts/run-one-test')

add_test_setup('standard',
  exe_wrapper: ['../scripts/init-test-setup', source_dir, build_dir],
  env: { 'SDB_DEV_TEST': 'true', },
  is_default: true # apply to all tests
)

add_test_setup('valgrind',
  exe_wrapper: ['../scripts/init-test-setup', source_dir, build_dir, 'valgrind',],
  env: { 'SDB_DEV_TEST': 'true', },
  timeout_multiplier: 10 # Valgrind is slow - prevent false timeout failures
)


#
# Standard tests
#

test('test_t1', run_one_test,
  args:[find_boot_mounts, '-none-', 'add'],
  workdir: testing_dir,
  )

test('test_t2', run_one_test,
  args:[efifs_update, '-none-', 'add'],
  workdir: testing_dir,
  )

t3_input_file = files(testing_dir / 'Triggers/edk2-shell')
test('test_t3', run_one_test,
  args: [efi_tool_update, t3_input_file, 'add'],
  workdir: testing_dir,
  )

t4_input_file = files(testing_dir / 'Triggers/linux')
test('test_t4', run_one_test,
  args: [kernel_update, t4_input_file, 'add'],
  workdir: testing_dir,
  )

