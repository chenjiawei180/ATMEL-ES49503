/*
 * spi.h
 *
 * Created: 2017/2/6 10:36:04
 *  Author: chenjiawei
 */ 


#ifndef SPI_H_
#define SPI_H_

#include "asf.h"

#define CONF_MASTER_SPI_MODULE  SERCOM1
#define CONF_MASTER_SS_PIN      PIN_PA17
#define CONF_MASTER_MUX_SETTING SPI_SIGNAL_MUX_SETTING_E
#define CONF_MASTER_PINMUX_PAD0 PINMUX_PA16C_SERCOM1_PAD0
#define CONF_MASTER_PINMUX_PAD1 PINMUX_UNUSED
#define CONF_MASTER_PINMUX_PAD2 PINMUX_PA18C_SERCOM1_PAD2
#define CONF_MASTER_PINMUX_PAD3 PINMUX_PA19C_SERCOM1_PAD3

extern void Configure_Spi_Master(void);
extern void SPI_Write_Buff(uint8_t *buff,uint16_t length);
extern void SPI_Read_Buff(uint8_t *buff,uint16_t length);
extern void SPI_Transceive_Buff(uint8_t *tx_buff,uint8_t *rx_buff,uint16_t length);
extern void SPI_Slave_Low(void);
extern void SPI_Slave_High(void);

#endif /* SPI_H_ */