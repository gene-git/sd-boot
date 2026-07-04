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
#  '-fno-tree-vectorize',       # Fixes the Valgrind size 32 AVX false positive
#
project('sd-boot', 'c',
  version: files('version.txt'),
  default_options: [
    'c_std=c23', 
    'warning_level=2',
    'prefix=/usr', 
    'mandir=share/man',
    ],
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
  '-Wpedantic',
  '-Wshadow'
]

buildtype = get_option('buildtype')
opt_level = get_option('optimization')
# if buildtype not in ['debug', 'plain']
if opt_level not in ['0', 'g', 'default']
  my_cflags += ['-Wp,-D_FORTIFY_SOURCE=3']
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
# Apply cflags/ldflags
#
add_project_arguments(my_cflags, language: 'c')
add_project_link_arguments(my_ldflags, language: 'c')


#
# Source code
#
#my_inc_dir = include_directories('include')

inc_dir = []
lib_sources = []

subdir('lib')
subdir('lib-cmd')
subdir('lib-config')
subdir('lib-efi')
subdir('lib-efifs')
subdir('lib-kernel')
subdir('lib-msg')
subdir('lib-mounts')
subdir('lib-package')
subdir('lib-tool')
subdir('lib-utils')
subdir('lib-keyval')

#
# Libs
#
mount_dep = dependency('mount', required: true)
#yaml_dep = dependency('yaml-0.1', required: true)
yaml_dep = dependency('libcyaml', required: true)
udev_dep = dependency('libudev', required: true)

sd_boot_lib = library('sd_boot',
  sources: lib_sources,
  include_directories: inc_dir,
  c_args: my_cflags,
  link_args: my_ldflags,
  dependencies: [mount_dep, udev_dep, yaml_dep],
  install: true,
  install_dir: '/usr/lib'
)

sd_boot_dep = declare_dependency(
  link_with: sd_boot_lib,
  include_directories: inc_dir
)

#
# Executables
#
find_boot_mounts = executable('sd-boot-find-boot-mounts',
  sources: ['tools/find-boot-mounts.c'],
  dependencies: [sd_boot_dep, mount_dep, udev_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: true,
  install_dir: '/usr/bin'
)

efifs_update = executable('sd-boot-efifs-update',
  sources: 'tools/efifs-update.c',
  dependencies: [sd_boot_dep, yaml_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: true,
  install_dir: '/usr/lib/sd-boot'
)

efi_tool_update_triggers = executable('sd-boot-efi-tool-update-triggers',
  sources: 'tools-efi/efi-tool-update-triggers.c',
  dependencies: [sd_boot_dep, yaml_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: true,
  install_dir: '/usr/lib/sd-boot'
)

efi_tool_update = executable('sd-boot-efi-tool-update',
  sources: 'tools-efi/efi-tool-update.c',
  dependencies: [sd_boot_dep, yaml_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: true,
  install_dir: '/usr/bin'
)

kernel_update_triggers = executable('sd-boot-kernel-update-triggers',
  sources: 'tools-kernel/kernel-update-triggers.c',
  dependencies: [sd_boot_dep, yaml_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: true,
  install_dir: '/usr/lib/sd-boot'
)

kernel_update = executable('sd-boot-kernel-update',
  sources: 'tools-kernel/kernel-update.c',
  dependencies: [sd_boot_dep, yaml_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: true,
  install_dir: '/usr/bin'
)

loaderentry_modify = executable('95-sd-boot-loaderentry-modify.install',
  sources: 'tools-plugins/loaderentry-modify.c',
  dependencies: [sd_boot_dep, yaml_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: true,
  install_dir: '/usr/lib/kernel/install.d'
)

#
# Configs / Hooks
#
d_etc = get_option('sysconfdir')
d_lib = get_option('libdir')
d_man = get_option('mandir')
d_share = get_option('datadir')

s_etc = 'data/etc'
s_lib = 'data/usr/lib'
s_man = 'data/man'
s_share = 'data/share'
s_hook = 'data/hooks'

p_man8 = 'man8'

p_dracut = 'dracut/dracut.conf.d'
p_kernel = 'kernel/install.conf.d'
p_hook = 'libalpm/hooks'

install_data(
  [
    s_lib / p_dracut / '010-sd-boot-dracut.conf'
  ], 
  install_dir: d_lib / p_dracut
)

install_data(
  [
    s_lib / p_kernel / '010-sd-boot-install.conf', 
    s_lib / p_kernel / '010-sd-boot-ukify.conf', 
  ],
  install_dir: d_lib / p_kernel
)

install_data(
  [
    s_etc / 'sd-boot' / 'config.yaml',       # to be removed in future release
    s_etc / 'sd-boot' / 'edk2-shell.image',
    s_etc / 'sd-boot' / 'efi-tool.packages',
    s_etc / 'sd-boot' / 'kernel.packages',
    s_etc / 'sd-boot' / 'memtest86_64-git.image',
  ], 
  install_dir: d_etc / 'sd-boot'
)

install_data(
  [
    s_hook / '69-sd-boot-efifs-remove.hook', 
    s_hook / '99-sd-boot-efifs-install.hook', 
    s_hook / '60-sd-boot-kernel-remove.hook', 
    s_hook / '90-sd-boot-kernel-install.hook', 
    s_hook / '70-sd-boot-efi-tool-remove.hook', 
    s_hook / '99-sd-boot-efi-tool-install.hook', 
  ],
  install_dir: d_share / p_hook
)

install_data(
  [
    s_share / 'config.yaml.sample',
    s_share / 'Changelog.rst',
    s_share / 'loader.conf',
    s_share / 'Readme.rst',
  ],
  install_dir: d_share / 'sd-boot'
)

install_data(
  [
    s_share / 'License/License-gpl-2.0.txt',
    s_share / 'License/License.rst',
  ],
  install_dir: d_share / 'sd-boot/License'
)

install_data(
  [
    s_share / 'License/License.rst',
  ],
  install_dir: d_share / 'licenses/sd-boot'
)

install_data(
  [
    s_man / 'man8' / 'sd-boot-efifs-update.8.gz',
    s_man / 'man8' / 'sd-boot-efi-tool-update.8.gz',
    s_man / 'man8' / 'sd-boot-find-boot-mounts.8.gz',
    s_man / 'man8' / 'sd-boot-kernel-update.8.gz',
  ],
  install_dir: d_man / 'man8'
)

#
# build man pages
# - if want meson to build them.
#   Requires python-docutils
#
#man_dir = meson.current_source_dir() / 'data/man'
#meson.add_install_script(man_dir / 'build-man-pages.sh')

#
# Tests: set up
# - standard test and tests run under valgrind
# - all tests use the sd-boot test env 
# - see scripts: init-test-setup, run_one_test
# --errors-for-leak-kinds=all
#

source_dir = meson.current_source_dir()
build_dir = source_dir / 'build' / buildtype
testing_dir = source_dir / 'tests/Testing'

run_one_test = find_program(source_dir / 'tests/scripts/run-one-test')

#
# Before running meson tests initialize the test root dir
# ../scripts/test-initialize <source-dir> <build-dir>
#
add_test_setup('standard',
  exe_wrapper: ['../scripts/run-one-wrapper'],
  env: { 'SDB_DEV_TEST': 'true', },
  is_default: true # apply to all tests if setup not specified
)

add_test_setup('valgrind',
  exe_wrapper: ['../scripts/run-one-wrapper', 'valgrind',],
  env: { 'SDB_DEV_TEST': 'true', },
  timeout_multiplier: 10 # Valgrind is slow - prevent false timeout failures
)


#
# Standard tests
#

#
# find_boot_mounts
#
test('boot_mounts_t1', run_one_test,
  args:[find_boot_mounts, '-none-', 'add'],
  workdir: testing_dir,
  suite: 'boot_mounts',
  priority: 50,
  is_parallel: false,
  )

#
# efifs_update
#
test('efifs_t1', run_one_test,
  args:[efifs_update, '-none-', 'add'],
  workdir: testing_dir,
  suite: 'efifs',
  priority: 50,
  is_parallel: false,
  )

test('efifs_t2', run_one_test,
  args:[efifs_update, '-none-', 'remove'],
  workdir: testing_dir,
  suite: 'efifs',
  priority: 30,
  is_parallel: false,
  )

#
# edk2-shell
#
edk2_shell_t1_input_file = files(testing_dir / 'Triggers/edk2-shell')
test('edk2-shell_t1', run_one_test,
  args: [efi_tool_update_triggers, edk2_shell_t1_input_file, 'add'],
  workdir: testing_dir,
  suite: 'edk2-shell',
  priority: 50,
  is_parallel: false,
  )

test('edk2-shell_t2', run_one_test,
  args: [efi_tool_update, '-none-', 'remove', 'edk2-shell'],
  workdir: testing_dir,
  suite: 'edk2-shell',
  priority: 40,
  is_parallel: false,
  )

test('edk2-shell_t3', run_one_test,
  args: [efi_tool_update, '-none-', 'add', 'edk2-shell'],
  workdir: testing_dir,
  suite: 'edk2-shell',
  priority: 30,
  is_parallel: false,
  )

test('edk2-shell_t4', run_one_test,
  args: [efi_tool_update, '-none-', 'inspect', 'edk2-shell'],
  workdir: testing_dir,
  suite: 'edk2-shell',
  priority: 20,
  is_parallel: false,
  )


#
# linux update
#
linux_t1_input_file = files(testing_dir / 'Triggers/linux')
test('linux_t1', run_one_test,
  args: [kernel_update_triggers, linux_t1_input_file, 'add'],
  workdir: testing_dir,
  suite: 'linux',
  priority: 50,
  is_parallel: false,
  )

test('linux_t2', run_one_test,
  args: [kernel_update, '-none-', 'remove', 'linux'],
  workdir: testing_dir,
  suite: 'linux',
  priority: 40,
  is_parallel: false,
  )

test('linux_t3', run_one_test,
  args: [kernel_update, '-none-', 'add', 'linux'],
  workdir: testing_dir,
  suite: 'linux',
  priority: 30,
  is_parallel: false,
  )

test('linux_t4', run_one_test,
  args: [kernel_update, '-none-', 'inspect', 'linux'],
  workdir: testing_dir,
  suite: 'linux',
  priority: 20,
  is_parallel: false,
  )

