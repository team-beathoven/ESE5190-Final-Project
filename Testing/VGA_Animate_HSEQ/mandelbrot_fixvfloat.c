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
#include "hardware/adc.h"

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


uint act_adc() {
    adc_select_input(0);
    uint adc_x_raw = adc_read();
    uint adc_x = 0;

    if (adc_x_raw > 2000 && adc_x_raw < 2060) {
        adc_x = 2048;
    } else {
        adc_x = adc_x_raw;
    }

    adc_x = (adc_x * 100) / 4095;

    printf("%d, %d\n", adc_x, adc_x_raw);
    
    if (adc_x == 50){
        fillRect(290,460,60,20,WHITE);
        fillRect(170,460,60,20,0);
        fillRect(410,460,60,20,0);
    }
    else if (adc_x > 50) {
        fillRect(410,460,60,20,WHITE);
        fillRect(170,460,60,20,0);
        fillRect(290,460,60,20,0);
    }else if (adc_x < 50) {
        fillRect(170,460,60,20,WHITE);
        fillRect(290,460,60,20,0);
        fillRect(410,460,60,20,0);
    }
    sleep_ms(10);
    return adc_x;
}

void draw_fill_rect(short x, short y, short w, short h, char color, short inc_dec){
    fillRect(x,y,w,h,color);
    fillRect(x,y,w,inc_dec,0);
    fillRect(x,y+h,w,inc_dec,color);
    sleep_ms(10);
}

int main() {

    // Initialize stdio
    stdio_init_all();

    // Initialize VGA
    initVGA() ;

    /* int pattern_array[6] = {20, 80, 20, 120, 60, 20} */
    
    adc_init();
    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(26);

    uint blue_indx = 20, green_indx = 80, cyan_indx = 60, joystick_pos = 0;

    while(true){
        if (cyan_indx > 71) {
            cyan_indx = 0;
            fillRect(420,360,40,100,0);
        }

        if (green_indx > 71) {
            green_indx = 0;
            fillRect(300,360,40,100,0);
        }

        if (blue_indx > 71) {
            blue_indx = 0;
            fillRect(180,360,40,100,0);
        }
        
        joystick_pos = act_adc();

        draw_fill_rect(180,(blue_indx*5),40,100,BLUE,5);
        draw_fill_rect(300,(green_indx*5),40,100,GREEN,5);
        draw_fill_rect(420,(cyan_indx*5),40,100,CYAN,5);

        cyan_indx++;
        green_indx++;
        blue_indx++;

        sleep_ms(100);
    }
    return 0;
}
