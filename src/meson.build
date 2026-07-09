#
# sd-boot
#
# release build:
#   meson setup --reconfigure build/release --buildtype=release -Ddefault_library=shared \
#     -Db_lto=true -Dhardened_export=true [-Dcpu_level=x86-64-v3 (baseline)]
#   meson compile -C build/release
#   meson install -C build/release --destdir <xxx>
#
# debug build
#   meson setup --reconfigure build/debug --buildtype=debug -Ddefault_library=static \
#     -Dstrip=false [-Dcpu_level=x86-64-v3 (baseline)]
#   meson compile -C build/debug
#   meson install -C build/debug --destdir <xxx>
#
# See do-test-build for debugoptimized and debugoptimized_nosan.
#
# Running Tests - standard or under valgrind.
# There are 4 test suites 1, 2, 3, 4:
#
# Initialize test tree (once).
#   ./tests/scripts/test-initialize
#
#   meson test test-set-1 -C build/release --setup standard 
#
# Repeat for other sets: test-set-2, ..., 
#
# Tests run under valgrind:
#
#   meson test test-set-1 -C build/release --setup valgrind 
# or
#   meson test test-set-1 -C build/debug --setup valgrind 
#
# Simplest is to use the build and test wrapper:
#
# ./do-test-build [release [valgrind]]
# ./do-test-build [debug [valgrind]]
#
#   -Db_lto=true                # link-time optimization + better dead code / symbol stripping
#   -Dcpu_level=x86-64-v3       # Supported by most reasonably hardware 2013+
#
project('sd-boot', 'c',
  version: files('version.txt'),
  meson_version: '>=1.10.0',
  default_options: [
    'c_std=c23', 
    'warning_level=3',
    'b_pie=true',                       # force PIE regardless of toolchain defaults
    'prefix=/usr', 
    'mandir=share/man',
    ],
)

#
# Install dirs - reused below for both build targets and data/config installs
# so every install_dir is consistent and respects cross-distro overrides.
#
d_etc = get_option('sysconfdir')
d_lib = get_option('libdir')
d_bin = get_option('bindir')
d_man = get_option('mandir')
d_share = get_option('datadir')

#
# CPU baseline
# - "baseline" (default) is safe on any x86-64 hardware and is what
#   packaged/distributed builds should use.
# - "x86-64-v3" requires AVX2/BMI2/FMA (Haswell, 2013+) and will SIGILL
#   on older or more conservative (e.g. some VM) CPUs. Since this tool
#   runs during kernel-install, a SIGILL here is a much worse failure
#   mode than slightly slower code on modern hardware - so this is
#   opt-in only, for people building for their own known machine.
#
cpu_level = get_option('cpu_level')
if cpu_level == 'x86-64-v3'
  cpu_cflags = ['-march=x86-64-v3', '-mtune=generic']
else
  cpu_cflags = ['-march=x86-64', '-mtune=generic']
endif

#
# Symbol visibility
# - default (current behaviour): every symbol in the shared lib is
#   exported, which is simple but exposes internal helpers as part of
#   the shared object's public ABI.
# - hardened_export=true: hide everything by default and only export
#   symbols explicitly marked in the headers. This is the better
#   long-term posture for a shared library, but requires the public
#   API to be annotated first (e.g. with an SD_BOOT_EXPORT macro) or
#   linking against sd_boot_dep will break.
#
if get_option('hardened_export')
  visibility_cflags = ['-fvisibility=hidden']
else
  visibility_cflags = ['-fvisibility=default']
endif


my_cflags = [
  ## General
  '-D_GNU_SOURCE',
  '-pipe',                              # Speeds up compilation
  '-fno-plt',                           # Optimization for shared library calls
  '-Wmissing-prototypes',               # Warn for global function missing prototype decl
  ## Memory Protections
  '-fstack-protector-strong',           # Protect function calls with stack canaries
  '-fstack-clash-protection',           # Prevents stack-clash exploits
  '-ftrivial-auto-var-init=zero',       # Eliminate uninitialized stack memory leaks
  '-fzero-call-used-regs=used-gpr',     # Wipe general registers before returning
  ## Control Flow & Runtime Integrity
  '-mshstk',                            # Intel Shadow Stack protection
  '-fcf-protection',                    # Control-flow enforcement
  ## Aggressive Diagnostic Hardening
  '-Wshadow',                           # Warn if local variable/decl shadows another 
  '-Wconversion',                       # Warn about type conversions that may alter value
  '-Wformat=2',                         # Warn about format/args mismatch printf, scanf etc
  '-Werror=format-security',            # Also warn about insecure printf format strings
] + cpu_cflags + visibility_cflags

#
# Note: warning_level=3 (above) already gives -Wall -Wextra -Wpedantic,
#

buildtype = get_option('buildtype')
opt_level = get_option('optimization')
# if buildtype not in ['debug', 'plain']
if opt_level not in ['0', 'g', 'default']
  my_cflags += ['-Wp,-D_FORTIFY_SOURCE=3']
endif

#
# Development / CI 
#  catch use-after-free/double-free/null-deref. Slow and can be noisy,
#
if get_option('analyzer')
  my_cflags += ['-fanalyzer']
endif

# link-time hardening
my_ldflags = [
  '-Wl,--as-needed',                    # Limit shared to 'as needed'
  '-Wl,-z,relro',                       # Mark relocation tables read-only
  '-Wl,-z,now',                         # Force immediate binding (Full RELRO)
  '-Wl,-z,noexecstack',                 # Strictly enforce non-executable stack
  '-Wl,-z,pack-relative-relocs',        # Optimize layout with DT_RELR packing
  '-Wl,-z,defs',                        # Catch unresolved symbols at link time, not at runtime
]

#
# Note: '-Wl,--sort-common' was dropped - it only helps the legacy
# .hash symbol table layout, and modern binutils/distros default to
# --hash-style=gnu (or both), where it has little to no effect.

#
# Apply cflags/ldflags
#
add_project_arguments(my_cflags, language: 'c')
add_project_link_arguments(my_ldflags, language: 'c')


#
# Source code
#

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
yaml_dep = dependency('libcyaml', required: true)
udev_dep = dependency('libudev', required: true)

sd_boot_lib = library('sd_boot',
  sources: lib_sources,
  include_directories: inc_dir,
  c_args: my_cflags,
  link_args: my_ldflags,
  dependencies: [mount_dep, udev_dep, yaml_dep],
  install: true,
  install_dir: d_lib
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
  install_dir: d_bin
)

efifs_update = executable('sd-boot-efifs-update',
  sources: 'tools/efifs-update.c',
  dependencies: [sd_boot_dep, yaml_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: true,
  install_dir: d_lib / 'sd-boot'
)

efi_tool_update_triggers = executable('sd-boot-efi-tool-update-triggers',
  sources: 'tools-efi/efi-tool-update-triggers.c',
  dependencies: [sd_boot_dep, yaml_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: true,
  install_dir: d_lib / 'sd-boot'
)

efi_tool_update = executable('sd-boot-efi-tool-update',
  sources: 'tools-efi/efi-tool-update.c',
  dependencies: [sd_boot_dep, yaml_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: true,
  install_dir: d_bin
)

kernel_update_triggers = executable('sd-boot-kernel-update-triggers',
  sources: 'tools-kernel/kernel-update-triggers.c',
  dependencies: [sd_boot_dep, yaml_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: true,
  install_dir: d_lib / 'sd-boot'
)

kernel_update = executable('sd-boot-kernel-update',
  sources: 'tools-kernel/kernel-update.c',
  dependencies: [sd_boot_dep, yaml_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: true,
  install_dir: d_bin
)

loaderentry_modify = executable('95-sd-boot-loaderentry-modify.install',
  sources: 'tools-plugins/loaderentry-modify.c',
  dependencies: [sd_boot_dep, yaml_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: true,
  install_dir: d_lib / 'kernel/install.d'
)

#
# Configs / Hooks
#
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
    s_etc / 'sd-boot' / 'config.yaml',       # may removed in future release
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
