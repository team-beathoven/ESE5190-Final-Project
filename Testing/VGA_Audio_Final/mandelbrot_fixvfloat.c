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
 *  - GPIO 16 ---> VGA Hsync
 *  - GPIO 17 ---> VGA Vsync
 *  - GPIO 18 ---> 330 ohm resistor ---> VGA Red
 *  - GPIO 19 ---> 330 ohm resistor ---> VGA Green
 *  - GPIO 20 ---> 330 ohm resistor ---> VGA Blue
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
#include "registers.h"
#include "hardware/irq.h"  // interrupts
#include "hardware/pwm.h"  // pwm 
#include "hardware/sync.h" // wait for interrupt 
 
// Audio PIN is to match some of the design guide shields. 
#define AUDIO_PIN_LEFT 28  // you can change this to whatever you like
#define AUDIO_PIN_RIGHT 15  // you can change this to whatever you like

/* 
 * This include brings in static arrays which contain audio samples. 
 * if you want to know how to make these please see the python code
 * for converting audio samples into static arrays. 
 */
/* #include "sample.h" */
/* #include "din.h" */
#include "A_major.h"
#include "E_major.h"
#include "B_major.h"
int wav_position = 0;
int flag_start = 0;
int audio_note_indx = 0;
int interception_side = 0;

/*
 * PWM Interrupt Handler which outputs PWM level and advances the 
 * current sample. 
 * 
 * We repeat the same value for 8 cycles this means sample rate etc
 * adjust by factor of 8   (this is what bitshifting <<3 is doing)
 * 
 */
void pwm_interrupt_handler() {
    pwm_clear_irq(pwm_gpio_to_slice_num(AUDIO_PIN_LEFT));    
    if (wav_position < (WAV_DATA_LENGTH<<3) - 1) { 
        // set pwm level 
        // allow the pwm value to repeat for 8 cycles this is >>3 
        if (audio_note_indx == 0) {
                pwm_set_gpio_level(AUDIO_PIN_LEFT, WAV_DATA_A[wav_position>>3]);
        } else if (audio_note_indx == 1) {
                pwm_set_gpio_level(AUDIO_PIN_LEFT, WAV_DATA_B[wav_position>>3]);
        } else {
                pwm_set_gpio_level(AUDIO_PIN_LEFT, WAV_DATA_E[wav_position>>3]);
        }
        wav_position++;
    } else {
        // reset to start
        pwm_set_gpio_level(AUDIO_PIN_LEFT, 0);  
        if (flag_start == 1) {
            wav_position = 0;
            flag_start = 0;
            if (audio_note_indx < 2) {
                audio_note_indx += 1;
            } else {
                audio_note_indx = 0;
            }
        }
    }
}

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

#define LEFT_VERT 150
#define MID_VERT 290
#define RIGHT_VERT 430

#define LEFT_VERT_TILES 160
#define MID_VERT_TILES 300
#define RIGHT_VERT_TILES 440

#define RESTART_PIN 4
#define RESTART_PIN_REG ((volatile uint32_t *)(IO_BANK0_BASE + 0x010))

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

    /* printf("%d, %d\n", adc_x, adc_x_raw); */
    
    if (adc_x == 50){
        fillRect(MID_VERT,460,60,20,WHITE);
        fillRect(LEFT_VERT,460,60,20,0);
        fillRect(RIGHT_VERT,460,60,20,0);
    }
    else if (adc_x > 50) {
        fillRect(RIGHT_VERT,460,60,20,WHITE);
        fillRect(LEFT_VERT,460,60,20,0);
        fillRect(MID_VERT,460,60,20,0);
    }else if (adc_x < 50) {
        fillRect(LEFT_VERT,460,60,20,WHITE);
        fillRect(MID_VERT,460,60,20,0);
        fillRect(RIGHT_VERT,460,60,20,0);
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

void update_score(uint score){
    fillRect(30,60,240,20,0);
    /* setCursor(30, 30); */
    /* setTextSize(3); */
    char str_score[3] = {'0', '0', '0'};
    str_score[2] = (score % 10) + '0';
    str_score[1] = ((score/10) % 10) + '0';
    str_score[0] = (((score/10)/10) % 10) + '0';
    drawChar(30, 60, str_score[0], WHITE, 0, 2);
    drawChar(45, 60, str_score[1], WHITE, 0, 2);
    drawChar(60, 60, str_score[2], WHITE, 0, 2);
}

int main() {

    /////////////////////////////////// AUDIO CODE //////////////////////////////////////////
    gpio_set_function(AUDIO_PIN_LEFT, GPIO_FUNC_PWM);

    int audio_pin_slice = pwm_gpio_to_slice_num(AUDIO_PIN_LEFT);

    // Setup PWM interrupt to fire when PWM cycle is complete
    pwm_clear_irq(audio_pin_slice);
    pwm_set_irq_enabled(audio_pin_slice, true);
    // set the handle function above
    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_interrupt_handler); 
    irq_set_enabled(PWM_IRQ_WRAP, true);

    // Setup PWM for audio output
    pwm_config config = pwm_get_default_config();
    /* Base clock 176,000,000 Hz divide by wrap 250 then the clock divider further divides
     * to set the interrupt rate. 
     * 
     * 11 KHz is fine for speech. Phone lines generally sample at 8 KHz
     * 
     * 
     * So clkdiv should be as follows for given sample rate
     *  8.0f for 11 KHz
     *  4.0f for 22 KHz
     *  2.0f for 44 KHz etc
     */
    pwm_config_set_clkdiv(&config, 8.0f); 
    pwm_config_set_wrap(&config, 250); 
    pwm_init(audio_pin_slice, &config, true);

    pwm_set_gpio_level(AUDIO_PIN_LEFT, 0);

    gpio_init(RESTART_PIN);
    gpio_set_dir(RESTART_PIN, GPIO_IN);

    // Initialize stdio
    stdio_init_all();

    // Initialize VGA
    initVGA() ;

    /* int pattern_array[6] = {20, 80, 20, 120, 60, 20} */
    
    adc_init();
    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(26);

    uint blue_indx = 20, green_indx = 40, cyan_indx = 60, joystick_pos = 0;
    uint curr_score = 0, buttons_status = 0;

    drawChar(30, 30, 'S', WHITE, 0, 2);
    drawChar(45, 30, 'c', WHITE, 0, 2);
    drawChar(60, 30, 'o', WHITE, 0, 2);
    drawChar(75, 30, 'r', WHITE, 0, 2);
    drawChar(90, 30, 'e', WHITE, 0, 2);
    drawChar(105, 30, ':', WHITE, 0, 2);
    update_score(curr_score);

    sleep_ms(5000);

    while(true) {
        while (true){
            joystick_pos = act_adc();
            
            if (cyan_indx > 89) {
                cyan_indx = 0;
                fillRect(RIGHT_VERT_TILES,360,40,100,0);
                if (joystick_pos > 50) {
                    sleep_ms(40);
                    fillRect(RIGHT_VERT_TILES,360,40,100,RED);
                    sleep_ms(40);
                    fillRect(RIGHT_VERT_TILES,360,40,100,0);
                    curr_score += 1;
                    update_score(curr_score);
                    flag_start = 1;
                    interception_side = 2;
                    __wfi();
                } else {
                    fillRect(RIGHT_VERT,460,60,20,0);
                    break;
                }
            }

            if (green_indx > 89) {
                green_indx = 0;
                fillRect(MID_VERT_TILES,360,40,100,0);
                if (joystick_pos == 50) {
                    sleep_ms(40);
                    fillRect(MID_VERT_TILES,360,40,100,RED);
                    sleep_ms(40);
                    fillRect(MID_VERT_TILES,360,40,100,0);
                    curr_score += 1;
                    update_score(curr_score);
                    flag_start = 1;
                    interception_side = 1;
                    __wfi();
                } else {
                    fillRect(MID_VERT,460,60,20,0);
                    break;
                }
            }

            if (blue_indx > 89) {
                blue_indx = 0;
                fillRect(LEFT_VERT_TILES,360,40,100,0);
                if (joystick_pos < 50) {
                    sleep_ms(40);
                    fillRect(LEFT_VERT_TILES,360,40,100,RED);
                    sleep_ms(40);
                    fillRect(LEFT_VERT_TILES,360,40,100,0);
                    curr_score += 1;
                    update_score(curr_score);
                    flag_start = 1;
                    interception_side = 0;
                    __wfi();
                } else {
                    fillRect(LEFT_VERT,460,60,20,0);
                    break;
                }
            }
            

            draw_fill_rect(LEFT_VERT_TILES,(blue_indx*4),40,100,BLUE,4);
            draw_fill_rect(MID_VERT_TILES,(green_indx*4),40,100,GREEN,4);
            draw_fill_rect(RIGHT_VERT_TILES,(cyan_indx*4),40,100,CYAN,4);

            cyan_indx++;
            green_indx++;
            blue_indx++;
            printf("Interception : %d", interception_side);
        }

        fillRect(LEFT_VERT_TILES,(blue_indx*4),40,100,0);
        fillRect(MID_VERT_TILES,(green_indx*4),40,100,0);
        fillRect(RIGHT_VERT_TILES,(cyan_indx*4),40,100,0);


        drawChar(180, 240, 'G', WHITE, 0, 5);
        drawChar(210, 240, 'A', WHITE, 0, 5);
        drawChar(240, 240, 'M', WHITE, 0, 5);
        drawChar(270, 240, 'E', WHITE, 0, 5);
        drawChar(300, 240, ' ', WHITE, 0, 5);
        drawChar(330, 240, 'O', WHITE, 0, 5);
        drawChar(360, 240, 'V', WHITE, 0, 5);
        drawChar(390, 240, 'E', WHITE, 0, 5);
        drawChar(420, 240, 'R', WHITE, 0, 5);
        drawChar(450, 240, '!', WHITE, 0, 5);
        drawChar(480, 240, '!', WHITE, 0, 5);
        
        buttons_status = register_read(RESTART_PIN_REG);
        printf("0x%08x\n", buttons_status);
        while (buttons_status == 0){
            buttons_status = register_read(RESTART_PIN_REG);
            printf("0x%08x\n", buttons_status);
            sleep_ms(10);
        }

        fillRect(180,240,400,100,0);
        curr_score = 0;
        update_score(curr_score);

        }
    return 0;
}
