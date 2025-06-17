//  https://cdn.sparkfun.com/assets/3/d/8/5/1/nRF24L01P_Product_Specification_1_0.pdf   datasheet

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "Joystick.h"

// These are Address values from datasheet 51/78 and 57/78 to 61/78
#define R_REGISTER 0x00
#define W_REGISTER 0x20
#define W_TX_PAYLOAD 0xA0
#define FLUSH_TX 0xE1
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

static spi_inst_t *SPI_PORT;
static uint PIN_MISO;
static uint PIN_CS;
static uint PIN_SCK;
static uint PIN_MOSI;
static uint PIN_CE;

void nrf_init(void);
void nrf_write_address(uint8_t reg, const uint8_t *addr, uint len);
static void read_reg(uint8_t reg, uint8_t *data, uint len);
static void flush_tx(void);
static void clear_ints(void);
void spi_nrf_init(spi_inst_t *spi_port,
                  uint pin_miso,
                  uint pin_cs,
                  uint pin_sck,
                  uint pin_mosi,
                  uint pin_ce);

void spi_nrf_init(spi_inst_t *spi_port,
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

    // initilize and set PIN_CE to 0 (PIN_CE sets radio to RX or TX mode)
    gpio_init(PIN_CE);
    gpio_set_dir(PIN_CE, GPIO_OUT);
    gpio_put(PIN_CE, 0);

    nrf_init();
}
void nrf_send_adc(void)
{
    uint8_t payload[4]; // Make space for joystick data
    uint8_t Write_TX_Payload = W_TX_PAYLOAD;

    // X = ADC0 (GP26)
    uint16_t raw_x = return_x(ADC_X_PIN); // 0 to 4095

    // Y = ADC1 (GP27)
    uint16_t raw_y = return_y(ADC_Y_PIN); // 0 to 4095

    // Pack into 4-byte payload (little-endian): [X_low, X_high, Y_low, Y_high]
    payload[0] = raw_x & 0xFF;
    payload[1] = (raw_x >> 8) & 0x0F; // upper nibble (only 12 bits used)
    payload[2] = raw_y & 0xFF;
    payload[3] = (raw_y >> 8) & 0x0F;

    printf("TX → X = %4u, Y = %4u   (0x%03X, 0x%03X)  Sending…\n",
           raw_x, raw_y, raw_x, raw_y);

    // Flush TX FIFO (0xE1 command)
    flush_tx();

    // Load payload
    gpio_put(PIN_CS, 0);
    spi_write_blocking(spi0, &Write_TX_Payload, 1);     // Send W_TX_PAYLOAD command
    spi_write_blocking(spi0, payload, sizeof(payload)); // Send payload
    gpio_put(PIN_CS, 1);

    // Pulse CE to transmit
    gpio_put(PIN_CE, 1);
    sleep_us(15);
    gpio_put(PIN_CE, 0);

    sleep_ms(10);

    // Read STATUS and FIFO_STATUS
    uint8_t status, fifo;
    read_reg(STATUS, &status, 1);
    read_reg(FIFO_STATUS, &fifo, 1);
    // f) Print them and check whether TX_DS or MAX_RT
    if (status & (1 << 5))
    {
        // TX_DS bit set → ACK received
        printf("    TX_DS = 1 (ACK received)\n");
        clear_ints();
    }
    else if (status & (1 << 4))
    {
        // MAX_RT bit set → no ACK after max retries
        printf("    MAX_RT = 1 (no ACK; flushing TX FIFO)\n");
        nrf_write_address(STATUS, (uint8_t[]){0x10}, 1); // clear only MAX_RT
        flush_tx();
    }
    else
    {
        printf("    Waiting for TX_DS or MAX_RT...\n");
    }

    sleep_ms(200);
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
    nrf_write_address(RF_CH, (uint8_t[]){0x2}, 1);

    // RF setup. 1 Mbps, 0dBm
    nrf_write_address(RF_SETUP, (uint8_t[]){0x06}, 1);

    // Set payload width
    nrf_write_address(RX_PW_P0, (uint8_t[]){0x04}, 1);

    // Enable  CRC, 2 bytes of CRC, Power up, PTX mode
    nrf_write_address(CONFIG, (uint8_t[]){0x0E}, 1);
    sleep_ms(2);

    // 9) Flush TX FIFO & clear any leftover STATUS
    flush_tx();
    clear_ints();

    // Pulse CE >10 µs to enter Standby-II (TX) mode
    gpio_put(PIN_CE, 1);
    sleep_us(15);
    gpio_put(PIN_CE, 0);
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

// Flush the TX FIFO
static void flush_tx(void)
{
    uint8_t cmd = FLUSH_TX;
    gpio_put(PIN_CS, 0);
    spi_write_blocking(SPI_PORT, &cmd, 1);
    gpio_put(PIN_CS, 1);
}

// Clear any (RX_DR, TX_DS, MAX_RT) bits in STATUS by writing 1 to bits 4,5,6
static void clear_ints(void)
{
    nrf_write_address(STATUS, (uint8_t[]){0x70}, 1);
}