/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# Minimal main function that starts with a call to system_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <asf.h>

#define OS_GLOBALS
#include "Source/global.h"

/* User Header */
#include "Source/usart.h"
#include "Source/led.h"
#include "Source/wdt.h"
#include "Source/spi.h"
#include "Source/flash.h"
#include "Source/power.h"
#include "Source/afe_wr.h"
#include "Source/adc.h"
#include "Source/gpio.h"
#include "Source/can.h"
#include "Source/history.h"

static volatile unsigned char flash_data[] __attribute__((section(".physicalsection")))={"const data"};


int main (void)
{
	uint16_t temp = flash_data[0] ;
	temp = temp;
	
	system_init();
	
	/* Initialize the delay driver */
	delay_init();
	
	/* Initialize the USART */
	#ifdef OS_DEBUG
	/*  Init Usart */
	Configure_Usart();
	//uint8_t string[] = "Hello World!\r\n";
	//Usart_send_buff(string,12);
	#endif // DEBUG
	
	/*  Init LED  */
	Configure_Led();
	
	/*  Init WDT  */
	//Configure_Wdt();
	
	/*  Init SPI  */	
	Configure_Spi_Master();
	
	/*  Init flash  */
	Configure_Flash();
	
	/* Init ADC0 */
	Configure_Adc();
	
	/* Init GPIO */
	Configure_GPIO();
	
	/* Init CAN */
	configure_can();
	can_set_standard_filter_1();
	buff_init();
	
	/* 上电变量初始化 */
	PowerOn_Init();
	
	VPC_High();
	delay_ms(50);
	VPC_Low();

	system_interrupt_enable_global();

	/* 从EEPROM恢复各变量的值 */
	SYS_EEPROM_Init();
	g_sys_cap.val.full_cap = cap_update;
	
	/* 初始化AFE */
    AFE_Init();
	delay_ms(300);
	
    sleep_delay_cycle = 0;
    sys_states.val.sys_sw_lowpower_flag = 0 ;          //低功耗模式 1关闭所有功能zzy20161101
	
	Bsp_LED0_Off();
	Bsp_LED1_On();

	Address_Init(); //初始化设备地址
	//printf("address is %d. \r\n",ID_address);
	
	/* Insert application code here, after the board has been initialized. */
	while (1)
	{
		can_process();
		if (AFE_disconnect)
		{
			AFE_Init();
		}
		else
		{
			AFE_Reg_Read();	
		}
		//printf("Cell 0 is %d. \r\n",nADC_Cell_Value[0]);
	}
}
