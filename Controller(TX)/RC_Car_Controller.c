//  https://cdn.sparkfun.com/assets/3/d/8/5/1/nRF24L01P_Product_Specification_1_0.pdf   datasheet

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/adc.h"
#include "Joystick.h"
#include "NRF24L01_TX.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS 17
#define PIN_SCK 18
#define PIN_MOSI 19
#define PIN_CE 20 // Chip Enable Activates RX or TX mode

// Joystick ADC inputs
#define ADC_X_PIN 26 // GP26 = ADC0
#define ADC_Y_PIN 27 // GP27 = ADC1

int main()
{
    stdio_init_all();
    sleep_ms(1000);

    adc_xy_init(ADC_X_PIN, ADC_Y_PIN);
    spi_nrf_init(SPI_PORT, PIN_MISO, PIN_CS, PIN_SCK, PIN_MOSI, PIN_CE);

    while (true)
    {
        nrf_send_adc();
        sleep_ms(5);
    }
}