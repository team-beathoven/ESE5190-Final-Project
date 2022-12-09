#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "sound_pwm.pio.h"

#include "/home/ruturajn/Git-Repos/pico/pico-sdk/src/boards/include/boards/adafruit_qtpy_rp2040.h"
#include "registers.h"
#define PWM_PIN 6
#define PWM_PIN_REG ((volatile uint32_t *)(IO_BANK0_BASE + 0x030))

void init_pwm_pio(){
    PIO pio = pio0;
    uint pwm_sm = 3;

    uint pwm_offset = pio_add_program(pio, &sound_pwm_program);

    sound_pwm_program_init(pio, pwm_sm, pwm_offset, PWM_PIN);

    pio_sm_set_enabled(pio, pwm_sm, false);
    pio_sm_put_blocking(pio, pwm_sm, 100);
    pio_sm_exec(pio, pwm_sm, pio_encode_pull(false, false));
    pio_sm_exec(pio, pwm_sm, pio_encode_pull(pio_isr, 32));
    pio_sm_set_enabled(pio, pwm_sm, true);
}

int main(){
    // Initialize the PIO for generating PWM
    const uint POWER_PIN_NAME = PICO_DEFAULT_WS2812_POWER_PIN;
    gpio_init(POWER_PIN_NAME);
    gpio_init(PWM_PIN);
    gpio_set_dir(PWM_PIN, GPIO_OUT);
    gpio_set_dir(POWER_PIN_NAME, GPIO_OUT);
    gpio_put(POWER_PIN_NAME, 1);
    stdio_init_all();
    init_pwm_pio();
    uint curr_level = 20;
    uint32_t reg_status = 0;

    while(1) {
        if (curr_level < 100) {
            curr_level += 1;
        } else {
            curr_level = 20;
        }
        pio_sm_put_blocking(pio0, 3, curr_level);
        reg_status = register_read(PWM_PIN_REG);
        printf("0x%08x\n", reg_status);
        sleep_ms(1000);
    }
    return 0;
}
