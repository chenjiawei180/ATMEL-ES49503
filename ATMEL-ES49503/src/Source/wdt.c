/*
 * wdt.c
 *
 * Created: 2017/2/6 10:33:44
 *  Author: chenjiawei
 */ 

#include "wdt.h"

/**
  * @brief  This function is Initialization WDT. 
  * @param  None
  * @retval None
  */

void Configure_Wdt(void)
{
	struct wdt_conf config_wdt;
	wdt_get_config_defaults(&config_wdt);
	wdt_set_config(&config_wdt);
}

/**
  * @brief  This function is claer WDT count. 
  * @param  None
  * @retval None
  */

void Wdt_Clear(void)
{
	wdt_reset_count();
}
