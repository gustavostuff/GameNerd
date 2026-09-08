cmake_minimum_required(VERSION 3.16)
project(retr01_nano_emu C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wpedantic)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)

# nano/apps/emu -> repo root is ../../..
set(R01_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." CACHE PATH "Retr01 repo root")
set(R01_PKG_ROOT "${R01_REPO_ROOT}/app")
set(R01_COMMON_DIR "${R01_PKG_ROOT}/common")
set(R01_OUTPUT_DIR "${R01_REPO_ROOT}/output/nano")
set(R01_TEST_CART "${R01_OUTPUT_DIR}/test.r01nano")

set(R01NE_CORE_SRCS
  src/cart.c
  src/video.c
  src/play.c
  src/machine.c
  src/custom_host.c
  ${R01_OUTPUT_DIR}/C/custom_logic.c
)

if(NOT TARGET r01_nano_emu_common)
  add_library(r01_nano_emu_common
    ${R01_COMMON_DIR}/r01_play_anim.c
    ${R01_COMMON_DIR}/r01_pad_keys.c
    ${R01_COMMON_DIR}/r01_custom_logic_scan.c
  )
  target_include_directories(r01_nano_emu_common PUBLIC ${R01_COMMON_DIR})
  # pad_keys needs SDL scancodes
  target_link_libraries(r01_nano_emu_common PUBLIC PkgConfig::SDL2)
endif()

add_library(retr01_nano_emu_core ${R01NE_CORE_SRCS})
target_include_directories(retr01_nano_emu_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${R01_OUTPUT_DIR}/C/include
  ${R01_OUTPUT_DIR}/C
)
target_link_libraries(retr01_nano_emu_core PUBLIC r01_nano_emu_common)
target_compile_definitions(retr01_nano_emu_core PUBLIC "R01NE_DEFAULT_CART=\"${R01_TEST_CART}\"")
# custom_logic includes "include/r01_engine.h" relative to C/
target_compile_options(retr01_nano_emu_core PRIVATE
  "-I${R01_OUTPUT_DIR}/C"
)
add_executable(retr01_nano_emu src/main.c)
target_link_libraries(retr01_nano_emu PRIVATE retr01_nano_emu_core PkgConfig::SDL2)

enable_testing()

add_executable(test_nano_cart tests/test_cart.c)
target_link_libraries(test_nano_cart PRIVATE retr01_nano_emu_core)
add_test(NAME test_nano_cart COMMAND test_nano_cart ${R01_TEST_CART})

add_executable(test_nano_boot tests/test_boot.c)
target_link_libraries(test_nano_boot PRIVATE retr01_nano_emu_core)
add_test(NAME test_nano_boot COMMAND test_nano_boot ${R01_TEST_CART})

add_executable(test_nano_play tests/test_play.c)
target_link_libraries(test_nano_play PRIVATE retr01_nano_emu_core)
add_test(NAME test_nano_play COMMAND test_nano_play ${R01_TEST_CART})
