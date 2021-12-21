cmake_minimum_required(VERSION 3.16)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/stm32-cmake/cmake/stm32_gcc.cmake)

project(esc C ASM)

find_package(CMSIS COMPONENTS STM32H753ZI_M7 REQUIRED)

add_executable(firmware main.c)
target_link_libraries(firmware CMSIS::STM32::H753ZI::M7 STM32::NoSys)
target_compile_options(firmware PRIVATE -O3 -g)
