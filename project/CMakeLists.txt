cmake_minimum_required(VERSION 3.13)


include(../../pico-sdk/pico_sdk_init.cmake)

project(music_box_controller_fw C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(music_box_controller_fw
	../src/led_mode_request.cpp
	../src/main.cpp
	../src/osc_commands.cpp
	../src/osc_message.cpp
	../src/usb_descriptors.c
	../src/usb_vendor_device.cpp
)
target_include_directories(music_box_controller_fw PRIVATE ../src)
target_link_libraries(music_box_controller_fw pico_stdlib hardware_pio pico_unique_id tinyusb_device)
pico_generate_pio_header(music_box_controller_fw ../src/ws2812.pio)
pico_add_extra_outputs(music_box_controller_fw)

# USB is now owned by the custom vendor (WinUSB) device in usb_descriptors.c /
# usb_vendor_device.cpp, so stdio moves to UART for debug output.
pico_enable_stdio_usb(music_box_controller_fw 0)
pico_enable_stdio_uart(music_box_controller_fw 1)
