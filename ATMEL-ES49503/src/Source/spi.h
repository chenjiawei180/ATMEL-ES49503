/*
 * spi.h
 *
 * Created: 2017/2/6 10:36:04
 *  Author: chenjiawei
 */ 


#ifndef SPI_H_
#define SPI_H_

#include "asf.h"
#include "global.h"

#define CONF_MASTER_SPI_MODULE  SERCOM1
#define CONF_MASTER_SS_PIN      PIN_PA17
#define CONF_MASTER_MUX_SETTING SPI_SIGNAL_MUX_SETTING_E
#define CONF_MASTER_PINMUX_PAD0 PINMUX_PA16C_SERCOM1_PAD0
#define CONF_MASTER_PINMUX_PAD1 PINMUX_UNUSED
#define CONF_MASTER_PINMUX_PAD2 PINMUX_PA18C_SERCOM1_PAD2
#define CONF_MASTER_PINMUX_PAD3 PINMUX_PA19C_SERCOM1_PAD3

#define SDI_PIN					PIN_PB16
#define SDI1_GetValue()         port_pin_get_input_level(SDI_PIN)

/********************************* SHDN **********************************************/
#define SHDN               PIN_PA07
#define SHDN_Low()         port_pin_set_output_level(SHDN, false)
#define SHDN_High()        port_pin_set_output_level(SHDN, true)
#define SHDN_Toggle()      port_pin_toggle_output_level(SHDN)

/********************************* STB **********************************************/
#define STB               PIN_PA10
#define STB_Low()         port_pin_set_output_level(STB, false)
#define STB_High()        port_pin_set_output_level(STB, true)
#define STB_Toggle()      port_pin_toggle_output_level(STB)

/********************************* VPC **********************************************/
#define VPC               PIN_PB17
#define VPC_Low()         port_pin_set_output_level(VPC, false)
#define VPC_High()        port_pin_set_output_level(VPC, true)
#define VPC_Toggle()      port_pin_toggle_output_level(VPC)

/********************************* ADIRQ2 **********************************************/
#define ADIRQ2_PIN               PIN_PA13
#define ADIRQ2_ACTIVE            false
#define ADIRQ2_INACTIVE          true
#define ADIRQ2_EIC_PIN           PIN_PA13A_EIC_EXTINT13
#define ADIRQ2_EIC_MUX           MUX_PA13A_EIC_EXTINT13
#define ADIRQ2_EIC_PINMUX        PINMUX_PA13A_EIC_EXTINT13
#define ADIRQ2_EIC_LINE          13
#define ADIRQ2_EIC_PULL_UP       EXTINT_PULL_NONE
#define ADIRQ2_EIC_DETECT        EXTINT_DETECT_RISING

#define	MAC_SPI_RW_WRITE		(uint8_t)0x00
#define	MAC_SPI_RW_READ			(uint8_t)0x01
#define	MAC_SPI_TRANS_SINGLE	(uint8_t)0x00
#define	MAC_SPI_TRANS_CONTINUE	(uint8_t)0x01
#define	MAC_SPI_WRITE_RETRY		(uint8_t)0x02
#define	MAC_SPI_READ_RETRY		(uint8_t)0x02

extern void Configure_Spi_Master(void);
extern void SPI_Write_Buff(uint8_t *buff,uint16_t length);
extern void SPI_Read_Buff(uint8_t *buff,uint16_t length);
extern void SPI_Transceive_Buff(uint8_t *tx_buff,uint8_t *rx_buff,uint16_t length);
extern void SPI_Slave_Low(void);
extern void SPI_Slave_High(void);

extern void vSPI_Wait(void);
extern uint8_t ucSPI_Write(uint8_t ucdev, uint8_t ucreg, uint16_t undata);
extern uint8_t ucSPI_Read(uint8_t ucdev, uint8_t ucreg, uint16_t* pundata);
extern uint8_t ucSPI_Continue_Read(uint8_t ucdev, uint8_t ucreg, uint8_t uctime);
extern uint8_t ucCRC_Calc(uint8_t ucsize,uint8_t* pucdt);

extern void ADIRQ2_Extint_Callback(void);

#endif /* SPI_H_ */