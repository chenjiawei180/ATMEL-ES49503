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
	config_wdt.always_on            = false;
	config_wdt.timeout_period       = WDT_PERIOD_8192CLK;
	config_wdt.early_warning_period = WDT_PERIOD_4096CLK;
	wdt_set_config(&config_wdt);
	configure_wdt_callbacks();
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

void configure_wdt_callbacks(void)
{
	wdt_register_callback(watchdog_callback,	WDT_CALLBACK_EARLY_WARNING);

	wdt_enable_callback(WDT_CALLBACK_EARLY_WARNING);
}

void watchdog_callback(void)
{
	wdt_reset_count();
}
