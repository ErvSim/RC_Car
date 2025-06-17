#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "NRF24L01_RX.h"

// These are Address values from datasheet 51/78 and 57/78 to 61/78
#define R_REGISTER 0x00
#define W_REGISTER 0x20
#define R_RX_PAYLOAD 0x61
#define FLUSH_RX 0xE2
#define NOP 0xFF

#define CONFIG 0x00
#define EN_AA 0x01
#define EN_RXADDR 0x02
#define SETUP_AW 0x03
#define SETUP_RETR 0x04
#define RF_CH 0x05
#define RF_SETUP 0x06
#define STATUS 0x07
#define TX_ADDR 0x10
#define RX_ADDR_P0 0x0A
#define RX_PW_P0 0x11
#define FIFO_STATUS 0x17

// We mark them 'static' so they are private to this .c file.
static spi_inst_t *SPI_PORT;
static uint PIN_MISO;
static uint PIN_CS;
static uint PIN_SCK;
static uint PIN_MOSI;
static uint PIN_CE;

void nrf_init(void);
void nrf_write_address(uint8_t reg, const uint8_t *addr, uint len);
static void read_reg(uint8_t reg, uint8_t *data, uint len);
void nrf_spi_init(spi_inst_t *spi_port,
                  uint pin_miso,
                  uint pin_cs,
                  uint pin_sck,
                  uint pin_mosi,
                  uint pin_ce);
bool nrf_get_adc(uint16_t *xy_array);

void nrf_spi_init(spi_inst_t *spi_port,
                  uint pin_miso,
                  uint pin_cs,
                  uint pin_sck,
                  uint pin_mosi,
                  uint pin_ce)
{
    SPI_PORT = spi_port;
    PIN_MISO = pin_miso;
    PIN_CS = pin_cs;
    PIN_SCK = pin_sck;
    PIN_MOSI = pin_mosi;
    PIN_CE = pin_ce;

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000 * 1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS, GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    gpio_init(PIN_CE);
    gpio_set_dir(PIN_CE, GPIO_OUT);
    gpio_put(PIN_CE, 0);

    nrf_init();
}

bool nrf_get_adc(uint16_t *xy_array) {
    uint8_t status;
    read_reg(STATUS, &status, 1);

    // If RX_DR (bit 6) is set, there's at least one payload in RX FIFO
    if (status & (1 << 6)) {
        uint16_t last_x = 0, last_y = 0;

        while (true) {
            uint8_t data[4];

            gpio_put(PIN_CS, 0);
            uint8_t cmd = R_RX_PAYLOAD;
            spi_write_blocking(SPI_PORT, &cmd, 1);
            spi_read_blocking( SPI_PORT, NOP, data, 4 );
            gpio_put(PIN_CS, 1);

            last_x = (uint16_t)data[0] | ((uint16_t)(data[1] & 0x0F) << 8);
            last_y = (uint16_t)data[2] | ((uint16_t)(data[3] & 0x0F) << 8);

            // Check if RX FIFO is now empty (bit 0 of FIFO_STATUS)
            uint8_t fifo_stat;
            read_reg(FIFO_STATUS, &fifo_stat, 1);
            if (fifo_stat & 0x01) 
            {
                // RX_EMPTY=1 → no more data
                break;
            }
            // else, loop and read the next payload
        }

        // Pass the newest data back to the caller
        xy_array[0] = last_x;
        xy_array[1] = last_y;

        // Clear all IRQ flags: RX_DR (bit 6), TX_DS (bit 5), MAX_RT (bit 4)
        nrf_write_address(STATUS, (uint8_t[]){ 0x70 }, 1);

        return true;
    }

    return false;
}

void nrf_write_address(uint8_t reg, const uint8_t *addr, uint len)
{
    uint8_t tx_buf[len + 1];
    tx_buf[0] = W_REGISTER | reg; // Write command (0x20 OR reg address) (0x20 comes from datasheet says 0b001A AAAA)
    for (int i = 0; i < len; i++)
        tx_buf[i + 1] = addr[i];

    gpio_put(PIN_CS, 0);
    spi_write_blocking(SPI_PORT, tx_buf, len + 1);
    gpio_put(PIN_CS, 1);
}

void nrf_init(void)
{
    // Chosen 5-byte address (LSB first)
    const uint8_t addr[5] = {0xEE, 0xDD, 0xCC, 0xBB, 0xAA};

    // Write to RX_ADDR_P0 (0x0A)
    nrf_write_address(RX_ADDR_P0, addr, 5);

    // Write to TX_ADDR (0x10)
    nrf_write_address(TX_ADDR, addr, 5);

    // Enable auto acknowledgement data pipe 0
    nrf_write_address(EN_AA, (uint8_t[]){0x01}, 1);

    // Enable data pipe 0
    nrf_write_address(EN_RXADDR, (uint8_t[]){0x01}, 1);

    // 5 byte Address width
    nrf_write_address(SETUP_AW, (uint8_t[]){0x03}, 1);

    // Auto-Retransmit 1.25ms, 3 times
    nrf_write_address(SETUP_RETR, (uint8_t[]){0x43}, 1);

    // RF Channel 2 (chosen randomly)
    nrf_write_address(RF_CH, (uint8_t[]){0x02}, 1);

    // RF setup. 1 Mbps, 0dBm
    nrf_write_address(RF_SETUP, (uint8_t[]){0x06}, 1);

    // Set payload width
    nrf_write_address(RX_PW_P0, (uint8_t[]){0x04}, 1);

    // Enable  CRC, 2 bytes of CRC, Power up, PRX mode
    nrf_write_address(CONFIG, (uint8_t[]){0x0F}, 1);
    sleep_ms(2);

    // Flush RX FIFO (0xE2 command)
    uint8_t flush_rx = FLUSH_RX;
    gpio_put(PIN_CS, 0);
    spi_write_blocking(spi0, &flush_rx, 1);
    gpio_put(PIN_CS, 1);

    // Clear any pending STATUS bits (RX_DR, TX_DS, MAX_RT)
    nrf_write_address(STATUS, (uint8_t[]){0x70}, 1);

    // Put CE high once, and leave it high so the chip stays in PRX
    gpio_put(PIN_CE, 1);
}

// Read `len` bytes from register `reg` into data[]
static void read_reg(uint8_t reg, uint8_t *data, uint len)
{
    uint8_t cmd = R_REGISTER | reg;
    gpio_put(PIN_CS, 0);
    spi_write_blocking(SPI_PORT, &cmd, 1);
    spi_read_blocking(SPI_PORT, NOP, data, len);
    gpio_put(PIN_CS, 1);
}