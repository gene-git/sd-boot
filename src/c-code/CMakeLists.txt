#
# sd-boot tools
# - cmake -G Ninja -B build
# - cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .
# - cmake --build build
#

cmake_minimum_required(VERSION 4.3)

# ALWAYS LAZY NEVER
set(CMAKE_INSTALL_MESSAGE LAZY)

#project(sd-boot VERSION 3.9.9 LANGUAGES C)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/../version.txt" RAW_VERSION)
string(STRIP "${RAW_VERSION}" PROJ_VERSION)
project(sd-boot VERSION ${PROJ_VERSION} LANGUAGES C)


# GNU Extensions
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

# valgrind is much better than sanitizer.
#set(SANITIZER_FLAGS "-fsanitize=address,undefined")
#add_compile_options("$<$<CONFIG:Debug>:${SANITIZER_FLAGS}>")
#add_link_options("$<$<CONFIG:Debug>:${SANITIZER_FLAGS}>")

# ntfw() needs this
add_compile_definitions(_GNU_SOURCE)
add_compile_definitions(_XOPEN_SOURCE=500)

if(CMAKE_COMPILER_IS_GNUCC)
    add_compile_options(
        -march=x86-64-v3
        -mtune=generic
        -mshstk 
        -O2
        -pipe
        -fno-plt
        -fexceptions
        -Wp,-D_FORTIFY_SOURCE=3
        -Wformat=2
        -Werror=format-security
        -fstack-clash-protection
        -fcf-protection
        -fvisibility=default
        -Wall 
        -Wextra 
        -Wpedantic 
        -Wshadow 
    )
endif()

# new shared/static
add_library(sd_boot_objs 
    lib/array_str.c
    lib/config.c
    lib/copy_file.c
    lib/dev_info.c
    lib/efi_image.c
    lib/env.c
    lib/file_list_glob.c
    lib/find_boot_mounts.c
    lib/get_one_line.c
    lib/kernel_info_all.c
    lib/kernel_info.c
    lib/kernel_install_oper.c
    lib/kernel_install_plugin.c
    lib/kernel_install_run.c
    lib/kernel_triggers.c
    lib/keyval.c
    lib/ki_plugin_env.c
    lib/loaderentry_efi.c
    lib/loaderentry_kernel.c
    lib/loaderentry_modify_file.c
    lib/make_dir.c
    lib/make_symlink.c
    lib/managed_packages.c
    lib/misc.c
    lib/msg.c
    lib/package_version_installed.c
    lib/package_versions.c
    lib/read_file.c
    lib/read_one_line.c
    lib/read_triggers.c
    lib/remove_file.c
    lib/rm_rf.c
    lib/run_cmd.c
    lib/run_cmd_output.c
    lib/string_in_list.c
    lib/trim_string.c
)
set_property(TARGET sd_boot_objs PROPERTY POSITION_INDEPENDENT_CODE 1)

add_library(sd_boot_shared SHARED $<TARGET_OBJECTS:sd_boot_objs>)
add_library(sd_boot_static STATIC $<TARGET_OBJECTS:sd_boot_objs>)

set_target_properties(sd_boot_shared PROPERTIES OUTPUT_NAME sd_boot)
set_target_properties(sd_boot_static PROPERTIES OUTPUT_NAME sd_boot)

#target_compile_options(sd-boot PRIVATE -g)
#set_target_properties(sd-boot PROPERTIES
#    VERSION ${PROJECT_VERSION}
#    SOVERSION ${PROJECT_VERSION_MAJOR}
#)

#include_directories(include)
include_directories(lib/include)
target_include_directories(sd_boot_shared PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/include)
target_include_directories(sd_boot_static PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/include)

# link all against the sd-boot library
#link_libraries(sd-boot)
#^^^

#link_libraries(/usr/lib/sd-boot/lib/libsd-boot.so)
#set(CMAKE_INSTALL_RPATH "/usr/lib/sd-boot/lib")
#set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

# tools and plugins
add_executable(sd-boot-find-boot-mounts tools/find-boot-mounts.c)
add_executable(sd-boot-efifs-update tools/efifs-update.c)
add_executable(sd-boot-efi-tool-update tools/efi-tool-update.c)
add_executable(sd-boot-kernel-update tools/kernel-update.c)

add_executable(95-sd-boot-loaderentry-modify.install plugins/loaderentry-modify.c)
#add_executable(95-sd-boot-loaderentry-kernel.install plugins/loaderentry-kernel.c)

add_executable(sd-boot-set-alternative tools/set-alternative.c)

#
# Linking: 
# Debug use static libs (kernel-install does not pass env vars to plugins (LD_LIBRARY_PATH)
# At min plugins require static link for testing.
# Proudction builds use shared libs as we instll libsd_boot.so into /usr/lib
#

#
# tools
#
target_link_libraries(sd-boot-find-boot-mounts PRIVATE 
    $<$<CONFIG:Debug>:sd_boot_static>
    $<$<NOT:$<CONFIG:Debug>>:sd_boot_shared>
)

target_link_libraries(sd-boot-efifs-update PRIVATE 
    $<$<CONFIG:Debug>:sd_boot_static>
    $<$<NOT:$<CONFIG:Debug>>:sd_boot_shared>
)

target_link_libraries(sd-boot-efi-tool-update PRIVATE 
    $<$<CONFIG:Debug>:sd_boot_static>
    $<$<NOT:$<CONFIG:Debug>>:sd_boot_shared>
)

target_link_libraries(sd-boot-kernel-update PRIVATE 
    $<$<CONFIG:Debug>:sd_boot_static>
    $<$<NOT:$<CONFIG:Debug>>:sd_boot_shared>
)

target_link_libraries(sd-boot-set-alternative PRIVATE 
    $<$<CONFIG:Debug>:sd_boot_static>
    $<$<NOT:$<CONFIG:Debug>>:sd_boot_shared>
)

#
# plugins
#
target_link_libraries(95-sd-boot-loaderentry-modify.install PRIVATE 
    $<$<CONFIG:Debug>:sd_boot_static>
    $<$<NOT:$<CONFIG:Debug>>:sd_boot_shared>
)

#target_link_libraries(95-sd-boot-loaderentry-kernel.install PRIVATE 
#    $<$<CONFIG:Debug>:sd_boot_static>
#    $<$<NOT:$<CONFIG:Debug>>:sd_boot_shared>
#)

#
# install
#
install(TARGETS 
    sd-boot-find-boot-mounts
    sd-boot-efifs-update
    sd-boot-efi-tool-update
    sd-boot-kernel-update
    DESTINATION /usr/lib/sd-boot/binary
)
install(TARGETS 
    sd-boot-set-alternative
    DESTINATION /usr/lib/sd-boot
)

install(TARGETS 
    95-sd-boot-loaderentry-modify.install
    DESTINATION /etc/kernel/install.d/binary
)

install(TARGETS sd_boot_shared
    LIBRARY DESTINATION lib
)

install(TARGETS sd_boot_static
    LIBRARY DESTINATION lib
)


if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    add_compile_options(
        -march=x86-64-v3
        -mtune=generic
        -mshstk -O2
        -pipe
        -fno-plt
        -fexceptions
        -Wp,-D_FORTIFY_SOURCE=3
        -Wformat=2
        -Werror=format-security
        -fstack-clash-protection
        -fcf-protection
        -Wall 
        -Wextra 
        -Wpedantic 
        -Wshadow 
    )
endif()

# target_link_libraries(${PROJECT_NAME} PRIVATE m) # Example for math library

