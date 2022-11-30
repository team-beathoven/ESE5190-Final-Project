/**
 * Hunter Adams (vha3@cornell.edu)
 * 
 * Mandelbrot set calculation and visualization
 * Uses PIO-assembly VGA driver.
 * 
 * Core 1 draws the bottom half of the set using floating point.
 * Core 0 draws the top half of the set using fixed point.
 * This illustrates the speed improvement of fixed point over floating point.
 * 
 * https://vanhunteradams.com/FixedPoint/FixedPoint.html
 * https://vanhunteradams.com/Pico/VGA/VGA.html
 *
 * HARDWARE CONNECTIONS
 *  - GPIO 24 ---> VGA Hsync
 *  - GPIO 25 ---> VGA Vsync
 *  - GPIO 26 ---> 330 ohm resistor ---> VGA Red
 *  - GPIO 27 ---> 330 ohm resistor ---> VGA Green
 *  - GPIO 28 ---> 330 ohm resistor ---> VGA Blue
 *  - RP2040 GND ---> VGA GND
 *
 * RESOURCES USED
 *  - PIO state machines 0, 1, and 2 on PIO instance 0
 *  - DMA channels 0 and 1
 *  - 153.6 kBytes of RAM (for pixel color data)
 *
 */
#include "vga_graphics.h"
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/pio.h"
#include "hardware/dma.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////// Stuff for Mandelbrot ///////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// Fixed point data type
typedef signed int fix28 ;
#define multfix28(a,b) ((fix28)(((( signed long long)(a))*(( signed long long)(b)))>>28)) 
#define float2fix28(a) ((fix28)((a)*268435456.0f)) // 2^28
#define fix2float28(a) ((float)(a)/268435456.0f) 
#define int2fix28(a) ((a)<<28)
// the fixed point value 4
#define FOURfix28 0x40000000 
#define SIXTEENTHfix28 0x01000000
#define ONEfix28 0x10000000

// Maximum number of iterations
#define max_count 1000

void draw_left_square(){
    drawHLine(170,290,100,BLUE);
    drawVLine(170,190,100,WHITE);
    drawHLine(170,190,100,BLUE);
    drawVLine(270,190,100,WHITE);
    sleep_ms(300);

    drawHLine(170,290,100,0);
    drawVLine(170,190,100,0);
    drawHLine(170,190,100,0);
    drawVLine(270,190,100,0);
    sleep_ms(10);
}

void draw_middle_square(){
    drawHLine(270,290,100,BLUE);
    drawVLine(270,190,100,WHITE);
    drawHLine(270,190,100,BLUE);
    drawVLine(370,190,100,WHITE);
    sleep_ms(300);

    drawHLine(270,290,100,0);
    drawVLine(270,190,100,0);
    drawHLine(270,190,100,0);
    drawVLine(370,190,100,0);
    sleep_ms(10);
}

void draw_right_square(){
    drawHLine(370,290,100,BLUE);
    drawVLine(370,190,100,WHITE);
    drawHLine(370,190,100,BLUE);
    drawVLine(470,190,100,WHITE);
    sleep_ms(300);

    drawHLine(370,290,100,0);
    drawVLine(370,190,100,0);
    drawHLine(370,190,100,0);
    drawVLine(470,190,100,0);
    sleep_ms(10);
}

int main() {

    // Initialize stdio
    stdio_init_all();

    // Initialize VGA
    initVGA() ;

    while(true){
        
        for (int i=0;i<160;i++){
            
            fillRect(170,(i*3),20,100,BLUE);
            fillRect(170,(i*3),20,5,0);
            fillRect(170,(i*3)+100,20,5,BLUE);
            sleep_ms(10);
            
            fillRect(270,(i*3),20,100,GREEN);
            fillRect(270,(i*3),20,5,0);
            fillRect(270,(i*3)+100,20,5,GREEN);
            sleep_ms(10);
            
            fillRect(370,i*3,20,100,CYAN);
            fillRect(370,i*3,20,5,0);
            fillRect(370,(i*3)+100,20,5,CYAN);
            sleep_ms(10);

            fillRect(170,480,20,1,0);
            fillRect(270,480,20,1,0);
            fillRect(370,480,20,1,0);
        }
        sleep_ms(100);
    }
}
