# Toolchain for CHERI Morello platforms

# This file is used by CMake when cross-compiling.
# It defines the target system, processor, and paths to the cross-compilers.

set(CHERI_SDK_BINDIR ${CHERI_SDK}/morello-sdk/bin)
message(STATUS "CHERI_SDK ${CHERI_SDK}")

set(CMAKE_SYSTEM_NAME FreeBSD)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(triple aarch64-unknown-freebsd)
set(CMAKE_C_COMPILER ${CHERI_SDK_BINDIR}/clang)
set(CMAKE_C_COMPILER_TARGET ${triple})
set(CMAKE_CXX_COMPILER ${CHERI_SDK_BINDIR}/clang++)
set(CMAKE_CXX_COMPILER_TARGET ${triple})
set(CMAKE_ASM_COMPILER ${CHERI_SDK_BINDIR}/clang)
set(CMAKE_ASM_COMPILER_TARGET ${triple})

set(CMAKE_AR ${CHERI_SDK_BINDIR}/llvm-ar)
set(CMAKE_LINKER ${CHERI_SDK_BINDIR}/lld)
set(CMAKE_OBJCOPY ${CHERI_SDK_BINDIR}/llvm-objcopy)
set(CMAKE_OBJDUMP ${CHERI_SDK_BINDIR}/llvm-objdump)
set(CMAKE_SIZE ${CHERI_SDK_BINDIR}/llvm-size)
set(CMAKE_NM ${CHERI_SDK_BINDIR}/llvm-nm)
set(CMAKE_RANLIB ${CHERI_SDK_BINDIR}/llvm-ranlib)

# CheriSDK sysroot
# set(CMAKE_SYSROOT ${CHERI_SDK}/rootfs-morello-purecap)
set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Target-specific compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=morello -mabi=purecap")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -march=morello -mabi=purecap")

set(CMAKE_CROSSCOMPILING TRUE)
