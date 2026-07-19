cmake_minimum_required(VERSION 3.13)


include(../../pico-sdk/pico_sdk_init.cmake)

project(music_box_controller_fw C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(music_box_controller_fw
	../src/main.cpp
)
target_link_libraries(music_box_controller_fw pico_stdlib)

pico_enable_stdio_usb(music_box_controller_fw 1)
pico_enable_stdio_uart(music_box_controller_fw 0)
