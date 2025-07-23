cmake_minimum_required(VERSION 3.10)
project(libcherithunk C ASM)

option(AUTH_WITH_SW_PERM "Authenticate thunk provenance with a software permission bit" ON)
option(LARGE_TOKEN_SPACE "Do not assume 48bit virtual address space" OFF)

set(CMAKE_C_FLAGS_INIT "-Wall -Werror -O3")
add_compile_options(-std=c11)

if (AUTH_WITH_SW_PERM)
  add_definitions(-DTHUNK_AUTH_MODE_PERMS)
endif ()

if (LARGE_TOKEN_SPACE)
  add_definitions(-DTHUNK_LARGE_TOKEN_SPACE)
endif ()

include_directories("${CMAKE_SOURCE_DIR}/src")
include_directories(arch/${CMAKE_SYSTEM_PROCESSOR})

add_library(${PROJECT_NAME} SHARED src/thunk.c src/thunk_gate.c)
target_sources(${PROJECT_NAME} PRIVATE
  arch/${CMAKE_SYSTEM_PROCESSOR}/thunk_machdep.c
  arch/${CMAKE_SYSTEM_PROCESSOR}/gate_thunk.S
  arch/${CMAKE_SYSTEM_PROCESSOR}/gate_machdep.c)

add_library(thunk_preload SHARED src/thunk_preload.c)

enable_testing()
add_subdirectory(test)
