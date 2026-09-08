cmake_minimum_required(VERSION 3.16)
project(retr01_nano_sim C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wpedantic)

set(R01_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." CACHE PATH "Retr01 repo root")
set(R01_PKG_ROOT "${R01_REPO_ROOT}/app")
set(R01_COMMON_DIR "${R01_PKG_ROOT}/common")
set(R01_SIM_DIR "${R01_PKG_ROOT}/sim")
set(R01_OUTPUT_DIR "${R01_REPO_ROOT}/output/nano")
set(R01_TEST_CART "${R01_OUTPUT_DIR}/test.r01nano")
set(R01_NANO_EMU_DIR "${R01_REPO_ROOT}/nano/apps/emu")

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)

if(NOT TARGET retr01_nano_emu_core)
  add_subdirectory(${R01_NANO_EMU_DIR} ${CMAKE_BINARY_DIR}/retr01_nano_emu EXCLUDE_FROM_ALL)
endif()

# Shared Sim netlist primitives + reusable IC shells (not the full 23-IC board).
add_library(retr01_nano_sim_net
  ${R01_SIM_DIR}/src/pin.c
  ${R01_SIM_DIR}/src/entity.c
  ${R01_SIM_DIR}/src/bus.c
  ${R01_SIM_DIR}/src/gamepad.c
  ${R01_SIM_DIR}/src/island.c
  ${R01_SIM_DIR}/src/island_group.c
  ${R01_SIM_DIR}/src/island_builder.c
  ${R01_SIM_DIR}/chips/pwr5v.c
  ${R01_SIM_DIR}/chips/osc8m.c
  ${R01_SIM_DIR}/chips/pads.c
  ${R01_SIM_DIR}/chips/i2c_eeprom.c
)
target_include_directories(retr01_nano_sim_net PUBLIC
  ${R01_SIM_DIR}/include
  ${R01_SIM_DIR}/chips
)

add_library(retr01_nano_sim_core
  chips/video_sink.c
  chips/sst25vf010a.c
  chips/pwm.c
  chips/atmega1284p_nano.c
  src/board.c
  src/ui.c
  src/app.c
)
target_include_directories(retr01_nano_sim_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/chips
)
target_link_libraries(retr01_nano_sim_core PUBLIC
  retr01_nano_sim_net
  retr01_nano_emu_core
  PkgConfig::SDL2
)
target_compile_definitions(retr01_nano_sim_core PUBLIC "R01NS_DEFAULT_CART=\"${R01_TEST_CART}\"")

add_executable(retr01_nano_sim src/main.c)
target_link_libraries(retr01_nano_sim PRIVATE retr01_nano_sim_core)

enable_testing()
add_executable(test_nano_sim_boot tests/test_boot.c)
target_link_libraries(test_nano_sim_boot PRIVATE retr01_nano_sim_core)
add_test(NAME test_nano_sim_boot COMMAND test_nano_sim_boot ${R01_TEST_CART})
