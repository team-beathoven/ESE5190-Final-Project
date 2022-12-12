cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(sound_pwm C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(sound_pwm)

# must match with pio filename and executable name from above
pico_generate_pio_header(sound_pwm ${CMAKE_CURRENT_LIST_DIR}/sound_pwm.pio)

pico_enable_stdio_usb(sound_pwm 1)
pico_enable_stdio_uart(sound_pwm 0)

# must match with executable name and source file names
target_sources(sound_pwm PRIVATE sound_pwm.c)

# must match with executable name
target_link_libraries(sound_pwm PRIVATE pico_stdlib hardware_pio hardware_dma hardware_adc)

# must match with executable name
pico_add_extra_outputs(sound_pwm)
