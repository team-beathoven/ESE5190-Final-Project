#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "sound_pwm.pio.h"

#define PWM_PIN 6

void init_pwm_pio(){
    PIO pio = pio0;
    uint pwm_sm = 3;

    uint pwm_offset = pio_add_program(pio, &sound_pwm_program);

    sound_pwm_program_init(pio, pwm_sm, pwm_offset, PWM_PIN);

    pio_sm_set_enabled(pio, pwm_sm, false);
    pio_sm_put_blocking(pio, pwm_sm, 20);
    pio_sm_exec(pio, pwm_sm, pio_encode_pull(false, false));
    pio_sm_exec(pio, pwm_sm, pio_encode_pull(pio_isr, 32));
    pio_sm_set_enabled(pio, pwm_sm, true);
}

int main(){

    // Initialize the PIO for generating PWM
    init_pwm_pio();
    uint curr_level = 20;

    while(1) {
        if (curr_level < 100) {
            curr_level += 1;
        } else {
            curr_level = 20;
        }
        pio_sm_put_blocking(pio0, 3, curr_level);
        sleep_ms(1000);
    }
    return 0;
}
