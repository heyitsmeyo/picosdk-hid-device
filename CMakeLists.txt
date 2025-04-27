cmake_minimum_required(VERSION 3.20)

include(pico_sdk_import.cmake)

project(hid_composite C CXX ASM)

pico_sdk_init()

add_executable(hid_composite
    src/main.c
    src/usb_descriptors.c
)

target_include_directories(hid_composite PUBLIC
    src
    tinyusb/src
)

target_compile_definitions(hid_composite PUBLIC
    CFG_TUSB_MCU=OPT_MCU_RP2040
    CFG_TUSB_RHPORT0_MODE=OPT_MODE_DEVICE
)

target_link_libraries(hid_composite
    pico_stdlib
    tinyusb_device
    tinyusb_board
)

pico_add_extra_outputs(hid_composite)

