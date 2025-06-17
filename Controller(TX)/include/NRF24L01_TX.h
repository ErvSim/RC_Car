#ifndef NRF24L01_TX_H
#define NRF24L01_TX_H

void spi_nrf_init(spi_inst_t *spi_port,
                  uint pin_miso,
                  uint pin_cs,
                  uint pin_sck,
                  uint pin_mosi,
                  uint pin_ce);

void nrf_send_adc(void);

#endif