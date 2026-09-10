cmake_minimum_required(VERSION 3.16)
project(retr01_nano_emu C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wpedantic)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)

# nano/apps/emu -> repo root is ../../..
set(R01_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." CACHE PATH "Retr01 repo root")
set(R01_OUTPUT_DIR "${R01_REPO_ROOT}/output/nano")
set(R01_TEST_CART "${R01_OUTPUT_DIR}/test.r01nano")

set(R01NE_CORE_SRCS
  src/cart.c
  src/video.c
  src/machine.c
)

add_library(retr01_nano_emu_core ${R01NE_CORE_SRCS})
target_include_directories(retr01_nano_emu_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_definitions(retr01_nano_emu_core PUBLIC "R01NE_DEFAULT_CART=\"${R01_TEST_CART}\"")

# Standalone app + tests only when this is the top-level project.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  add_executable(retr01_nano_emu src/main.c)
  target_link_libraries(retr01_nano_emu PRIVATE retr01_nano_emu_core PkgConfig::SDL2)

  enable_testing()

  add_executable(test_nano_cart tests/test_cart.c)
  target_link_libraries(test_nano_cart PRIVATE retr01_nano_emu_core)
  add_test(NAME test_nano_cart COMMAND test_nano_cart ${R01_TEST_CART})

  add_executable(test_nano_boot tests/test_boot.c)
  target_link_libraries(test_nano_boot PRIVATE retr01_nano_emu_core)
  add_test(NAME test_nano_boot COMMAND test_nano_boot ${R01_TEST_CART})
endif()
