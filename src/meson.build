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
  '-Wpedantic',
  '-Wshadow'
]

if get_option('buildtype') != 'debug'
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
subdir('lib-kernel')
subdir('lib-msg')
subdir('lib-mounts')
subdir('lib-package')
subdir('lib-utils')

#
# Libs
#
mount_dep = dependency('mount', required: true)
yaml_dep = dependency('yaml-0.1', required: true)
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
install_data('data/etc/dracut.conf.d/010-dracut.conf', install_dir: '/etc/dracut.conf.d')

install_data('data/etc/kernel/install.conf', install_dir: '/etc/kernel')
install_data('data/etc/kernel/ukify.conf', install_dir: '/etc/kernel')

# to be removed in the future release
install_data('data/etc/sd-boot/config.yaml', install_dir: '/etc/sd-boot')

install_data('data/etc/sd-boot/edk2-shell.image', install_dir: '/etc/sd-boot')
install_data('data/etc/sd-boot/efi-tool.packages', install_dir: '/etc/sd-boot')
install_data('data/etc/sd-boot/kernel.packages', install_dir: '/etc/sd-boot')
install_data('data/etc/sd-boot/memtest86_64-git.image', install_dir: '/etc/sd-boot')

install_data('data/hooks/60-sd-boot-kernel-remove.hook', install_dir: '/usr/share/libalpm/hooks')
install_data('data/hooks/69-sd-boot-efifs-remove.hook', install_dir: '/usr/share/libalpm/hooks')
install_data('data/hooks/70-sd-boot-efi-tool-remove.hook', install_dir: '/usr/share/libalpm/hooks')
install_data('data/hooks/90-sd-boot-kernel-install.hook', install_dir: '/usr/share/libalpm/hooks')
install_data('data/hooks/99-sd-boot-efifs-install.hook', install_dir: '/usr/share/libalpm/hooks')
install_data('data/hooks/99-sd-boot-efi-tool-install.hook', install_dir: '/usr/share/libalpm/hooks')


install_data('data/share/config.yaml.sample', install_dir: '/usr/share/sd-boot')
install_data('data/share/Changelog.rst', install_dir: '/usr/share/sd-boot')
install_data('data/share/loader.conf', install_dir: '/usr/share/sd-boot')
install_data('data/share/Readme.rst', install_dir: '/usr/share/sd-boot')
install_data('data/share/License/License-gpl-2.0.txt', install_dir: '/usr/share/sd-boot/License')
install_data('data/share/License/License.rst', install_dir: '/usr/share/sd-boot/License')

install_data('data/share/License/License.rst', install_dir: '/usr/share/licenses/sd-boot')

install_data('data/man/man8/sd-boot-efifs-update.8.gz', install_dir: '/usr/share/man/man8')
install_data('data/man/man8/sd-boot-efi-tool-update.8.gz', install_dir: '/usr/share/man/man8')
install_data('data/man/man8/sd-boot-find-boot-mounts.8.gz', install_dir: '/usr/share/man/man8')
install_data('data/man/man8/sd-boot-kernel-update.8.gz', install_dir: '/usr/share/man/man8')

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
  args: [efi_tool_update_triggers, t3_input_file, 'add'],
  workdir: testing_dir,
  )

test('test_t4', run_one_test,
  args: [efi_tool_update, '-none-', 'add', 'edk2-shell'],
  workdir: testing_dir,
  )

test('test_t4a', run_one_test,
  args: [efi_tool_update, '-none-', 'inspect', 'edk2-shell'],
  workdir: testing_dir,
  )

t5_input_file = files(testing_dir / 'Triggers/linux')
test('test_t5', run_one_test,
  args: [kernel_update_triggers, t5_input_file, 'add'],
  workdir: testing_dir,
  )

test('test_t6', run_one_test,
  args: [kernel_update, '-none-', 'add', 'linux'],
  workdir: testing_dir,
  )

test('test_t6a', run_one_test,
  args: [kernel_update, '-none-', 'inspect', 'linux'],
  workdir: testing_dir,
  )

