/*
 * usart.h
 *
 * Created: 2017/2/6 10:15:19
 *  Author: chenjiawei
 */ 


#ifndef USART_H_
#define USART_H_

#include "asf.h"
#include "stdio_serial.h"
#include "global.h"

//#define OS_DEBUG

#define USART1_MODULE              SERCOM3
#define USART1_BAUD                19200
#define USART1_SERCOM_MUX_SETTING  USART_RX_1_TX_0_XCK_1
#define USART1_SERCOM_PINMUX_PAD0  PINMUX_PA22C_SERCOM3_PAD0
#define USART1_SERCOM_PINMUX_PAD1  PINMUX_PA23C_SERCOM3_PAD1
#define USART1_SERCOM_PINMUX_PAD2  PINMUX_UNUSED
#define USART1_SERCOM_PINMUX_PAD3  PINMUX_UNUSED

extern void Configure_Usart(void);
extern void Usart_send_buff(uint8_t *data,uint16_t length);
extern uint16_t Crc16Calc(uint8_t *data_arr, uint8_t data_len);
extern void Usart_process(void);

#endif /* USART_H_ */