cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(blink_new C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(falling_tiles)

# must match with pio filename and executable name from above
pico_generate_pio_header(falling_tiles ${CMAKE_CURRENT_LIST_DIR}/hsync.pio)
pico_generate_pio_header(falling_tiles ${CMAKE_CURRENT_LIST_DIR}/vsync.pio)
pico_generate_pio_header(falling_tiles ${CMAKE_CURRENT_LIST_DIR}/rgb.pio)

pico_enable_stdio_usb(falling_tiles 1)
pico_enable_stdio_uart(falling_tiles 0)

# must match with executable name and source file names
target_sources(falling_tiles PRIVATE falling_tiles.c vga_graphics.c registers.h ./audio_notes/A_major.h ./audio_notes/B_major.h ./audio_notes/E_major.h)

# must match with executable name
target_link_libraries(falling_tiles PRIVATE pico_stdlib pico_multicore hardware_pio hardware_dma hardware_adc hardware_pwm)

# must match with executable name
pico_add_extra_outputs(falling_tiles)
