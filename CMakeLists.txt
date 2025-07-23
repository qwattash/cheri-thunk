cmake_minimum_required(VERSION 3.10)
project(libcherithunk C ASM)

set(CMAKE_C_FLAGS_INIT "-Wall -Werror -O3")
add_compile_options(-std=c11)

include_directories("${CMAKE_SOURCE_DIR}")
include_directories(arch/${CMAKE_SYSTEM_PROCESSOR})

add_library(${PROJECT_NAME} SHARED thunk.c)
target_sources(${PROJECT_NAME} PRIVATE
  arch/${CMAKE_SYSTEM_PROCESSOR}/thunk_machdep.c)

enable_testing()
add_subdirectory(test)
