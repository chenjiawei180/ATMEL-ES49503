/*
 * usart.c
 *
 * Created: 2017/2/6 10:16:16
 *  Author: chenjiawei
 */ 

#include "usart.h"
#include "ad_dat.h"

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

void Usart_send_buff(uint8_t *data,uint16_t length)
{
	usart_write_buffer_wait(&usart_instance, data,length);
}

uint16_t Crc16Calc(uint8_t *data_arr, uint8_t data_len)
{
	uint16_t crc16 = 0;
	uint8_t i;
	for(i = 2; i < (data_len+2); i++)
	{
		crc16 = (uint16_t)(( crc16 >> 8) | (crc16 << 8));
		crc16 ^= data_arr[i];
		crc16 ^= (uint16_t)(( crc16 & 0xFF) >> 4);
		crc16 ^= (uint16_t)(( crc16 << 8) << 4);
		crc16 ^= (uint16_t)((( crc16 & 0xFF) << 4) << 1);
	}
	return crc16;
}

void Usart_process(void)
{
	uint16_t crc16 = 0;
	vAPI_Uart_Load();
	TxBuffer[0] = 0xfe;
	TxBuffer[1] = 0xfe;
	TxBuffer[2] = 0x31;
	TxBuffer[3] = UartTxLength-7;
	TxBuffer[4] = afe_flags.VAL>>8;
	TxBuffer[5] = afe_flags.VAL;
	TxBuffer[6] = sys_states.VAL>>8;
	TxBuffer[7] = sys_states.VAL;
	//                TxBuffer[8] = cell_balance.VAL>>8; g_sys_cap.val.bat_cycle_cnt
	//                TxBuffer[9] = cell_balance.VAL;
	TxBuffer[8] = g_sys_cap.val.bat_cycle_cnt>>8;
	TxBuffer[9] = g_sys_cap.val.bat_cycle_cnt;
	/* 百分比 */
	TxBuffer[55] = g_sys_cap.val.re_cap_rate>>8;
	TxBuffer[56] = g_sys_cap.val.re_cap_rate;
	/* 剩余电量 */
	TxBuffer[57] = g_sys_cap.val.cap_val>>8;
	TxBuffer[58] = g_sys_cap.val.cap_val;
	/* 总电量 */
	TxBuffer[59] = g_sys_cap.val.full_cap>>8;
	TxBuffer[60] = g_sys_cap.val.full_cap;
	//新增
	TxBuffer[61] = g_sys_cap.val.deep_dch_cycle_cnt;
	TxBuffer[62] = g_sys_cap.val.deep_chg_cycle_cnt;
	TxBuffer[63] = sys_err_flags.VAL>>8;
	TxBuffer[64] = sys_err_flags.VAL;

	crc16 =0;
	crc16 = Crc16Calc(TxBuffer,UartTxLength-5);
	TxBuffer[UartTxLength-3] = crc16>>8;
	TxBuffer[UartTxLength-2] = crc16;
	TxBuffer[UartTxLength-1] = 0xbb;
	
	Usart_send_buff(TxBuffer,UartTxLength);
}