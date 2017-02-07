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


int main (void)
{
	system_init();
	
	/* Initialize the delay driver */
	delay_init();
	
	/* Initialize the USART */
	#ifdef OS_DEBUG
	/*  Init Usart */
	Configure_Usart();
	#endif // DEBUG
	
	/*  Init LED  */
	Configure_Led();
	
	/*  Init WDT  */
	//Configure_Wdt();
	
	/*  Init SPI  */	
	Configure_Spi_Master();
	
	/*  Init flash  */
	Configure_Flash();
	
	/* 上电变量初始化 */
	PowerOn_Init();
	
	/* 从EEPROM恢复各变量的值 */
	SYS_EEPROM_Init();
	g_sys_cap.val.full_cap = cap_update;
	
	/* Insert application code here, after the board has been initialized. */
	while (1)
	{
		
	}
}
