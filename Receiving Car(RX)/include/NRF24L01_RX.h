#ifndef NRF24L01_H
#define NRF24L01_H

void nrf_spi_init(spi_inst_t *spi_port,
                  uint pin_miso,
                  uint pin_cs,
                  uint pin_sck,
                  uint pin_mosi,
                  uint pin_ce);
bool nrf_get_adc(uint16_t *xy_array);

#endif