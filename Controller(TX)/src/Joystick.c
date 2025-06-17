#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "Joystick.h"

void adc_xy_init(uint xpin, uint ypin)
{
    adc_init();
    adc_gpio_init(xpin);
    adc_gpio_init(ypin);
}

uint16_t return_x(uint xpin)
{
    adc_select_input(xpin - 26);    //since we have ADC0-ADC2, starting from GPIo26 to GPIO28, we take gpio#, subtract by 26 to get which ADC we are using
    uint16_t raw_xadc = adc_read();
    return raw_xadc;
}

uint16_t return_y(uint ypin)
{
    adc_select_input(ypin - 26);
    uint16_t raw_yadc = adc_read();
    return raw_yadc;
}