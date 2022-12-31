# ESE5190-Final-Project

The directory structure is as follows:

```
.
├── Falling_Piano_Tiles
│   ├── CMakeLists.txt
│   ├── audio_notes
│   ├── falling_tiles.c
│   ├── glcdfont.c
│   ├── hsync.pio
│   ├── pico_sdk_import.cmake
│   ├── re_build.sh
│   ├── registers.h
│   ├── rgb.pio
│   ├── vga_graphics.c
│   ├── vga_graphics.h
│   └── vsync.pio
├── README.md
└── Testing
    ├── Joystick
    ├── VGA_Animate_HSEQ
    ├── VGA_Audio_Final
    ├── VGA_Audio_Integrate
    ├── VGA_Audio_Integrate_2
    ├── VGA_Intercept_Tiles
    ├── VGA_Joystick
    ├── VGA_Mandelbrot_Set
    ├── VGA_Test_Draw
    ├── pico_pwm_example
    └── sound_test

14 directories, 12 files
```

Here all the folders in the `Testing` directory show our progress from animating a basic block on the screen to a complete game. Whereas the `Falling_Piano_Tiles`
directory contains the final source code for our project.

<br>
</br>

## Block Diagram

![block_diagram](https://user-images.githubusercontent.com/56625259/210120526-0d660312-8178-4246-ae59-7a8a795c566c.png)

<br>
</br>

## Circuit Connections

Following are diagrams illustrating the connections for each of the components:

### VGA 
![VGA_Connections](https://user-images.githubusercontent.com/56625259/210120542-493dc54d-5762-4910-bd44-da62e299a2ef.jpg)

### Complete Circuit
![Circuit_Connections](https://user-images.githubusercontent.com/56625259/210121905-0ba90e56-1425-43ed-bf8a-166c5939c087.jpg)
