//  https://cdn.sparkfun.com/assets/3/d/8/5/1/nRF24L01P_Product_Specification_1_0.pdf   datasheet

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "NRF24L01_RX.h"
#include "PWM_Wheels.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS 17
#define PIN_SCK 18
#define PIN_MOSI 19
#define PIN_CE 20 // Chip Enable Activates RX or TX mode

#define TLRedWheel 8
#define TLBlackWheel 9
#define BLRedWheel 10
#define BLBlackWheel 11
#define TRRedWheel 12
#define TRBlackWheel 13
#define BRRedWheel 14
#define BRBlackWheel 15

void nrf_init(void);
void nrf_write_address(uint8_t reg, const uint8_t *addr, uint len);
static void read_reg(uint8_t reg, uint8_t *data, uint len);

int main()
{
    stdio_init_all();
    sleep_ms(1000);

    uint16_t xy_values[2];

    nrf_spi_init(SPI_PORT, PIN_MISO, PIN_CS, PIN_SCK, PIN_MOSI, PIN_CE);
    pwm_xy_init(TLRedWheel, TLBlackWheel);
    pwm_xy_init(BLRedWheel, BLBlackWheel);
    pwm_xy_init(TRRedWheel, TRBlackWheel);
    pwm_xy_init(BRRedWheel, BRBlackWheel);

    while (true)
    {
        if (nrf_get_adc(xy_values))
        {
            printf("→ Received X=%4u, Y=%4u\n", xy_values[0], xy_values[1]);
            set_wheel_value(
                xy_values[0],
                xy_values[1],
                TLRedWheel, TLBlackWheel,
                BLRedWheel, BLBlackWheel,
                TRRedWheel, TRBlackWheel,
                BRRedWheel, BRBlackWheel);
        }
        sleep_ms(5);
    }
}

