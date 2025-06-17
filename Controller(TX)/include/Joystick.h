#ifndef JOYSTICK_H
#define JOYSTICK_H
#include <stdint.h>

// Joystick ADC inputs
#define ADC_X_PIN 26 // GP26 = ADC0
#define ADC_Y_PIN 27 // GP27 = ADC1

void adc_xy_init(uint xpin, uint ypin);
uint16_t return_x(uint xpin);
uint16_t return_y(uint ypin);

#endif