/*
 * usart.c
 *
 * Created: 2017/2/6 10:16:16
 *  Author: chenjiawei
 */ 

#include "usart.h"

struct usart_module usart_instance;

/**
  * @brief  This function is Initialization Usart1. default setting is 115200-1-8-None .
  * @param  None
  * @retval None
  */

void Configure_Usart(void)
{
	struct usart_config config_usart;
	usart_get_config_defaults(&config_usart);
	config_usart.baudrate    = USART1_BAUD;
	config_usart.mux_setting = USART1_SERCOM_MUX_SETTING;
	config_usart.pinmux_pad0 = USART1_SERCOM_PINMUX_PAD0;
	config_usart.pinmux_pad1 = USART1_SERCOM_PINMUX_PAD1;
	config_usart.pinmux_pad2 = USART1_SERCOM_PINMUX_PAD2;
	config_usart.pinmux_pad3 = USART1_SERCOM_PINMUX_PAD3;
	
	stdio_serial_init(&usart_instance, USART1_MODULE, &config_usart);
	
	usart_enable(&usart_instance);
}