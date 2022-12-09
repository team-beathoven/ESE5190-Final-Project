/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "pwm.pio.h"
#include "sample.h"

#define GPIO_PIN 5

// Write `period` to the input shift register
void pio_pwm_set_period(PIO pio, uint sm, uint32_t period) {
    pio_sm_set_enabled(pio, sm, false);
    pio_sm_put_blocking(pio, sm, period);
    pio_sm_exec(pio, sm, pio_encode_pull(false, false));
    pio_sm_exec(pio, sm, pio_encode_out(pio_isr, 32));
    pio_sm_set_enabled(pio, sm, true);
}

// Write `level` to TX FIFO. State machine will copy this into X.
void pio_pwm_set_level(PIO pio, uint sm, uint32_t level) {
    pio_sm_put_blocking(pio, sm, level);
}

int main() {
    stdio_init_all();
#ifndef PICO_DEFAULT_LED_PIN
#warning pio/pwm example requires a board with a regular LED
    puts("Default LED pin was not defined");
#else

    // todo get free sm
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &pwm_program);
    printf("Loaded program at %d\n", offset);

    /* pwm_program_init(pio, sm, offset, PICO_DEFAULT_LED_PIN); */
    pwm_program_init(pio, sm, offset, GPIO_PIN);
    pio_pwm_set_period(pio, sm, 250);

    int level = 0;
    int curr_indx = 0;
    int rerun_ind = 0;
    while (true) {
        if (curr_indx > WAV_DATA_LENGTH) {
            curr_indx = 0;
        }
        printf("Level = %d\n", level);
        level = WAV_DATA[curr_indx];
        pio_pwm_set_level(pio, sm, level);
        /* level = (level + 1) % 256; */
        sleep_ms(10);
        if (rerun_ind == 8) {
            curr_indx += 1;
        } else if (rerun_ind > 8) {
            rerun_ind = 0;
        }
        rerun_ind += 1;
    }
#endif
}
