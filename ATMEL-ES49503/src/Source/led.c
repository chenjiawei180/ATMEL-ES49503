/*
 * led.c
 *
 * Created: 2017/2/6 10:30:41
 *  Author: chenjiawei
 */ 

#include "led.h"

/**
  * @brief  This function is Initialization LED0 and LED1. default state of led is turn off.
  * @param  None
  * @retval None
  */

void Configure_Led(void)
{
	struct port_config pin_conf;
	port_get_config_defaults(&pin_conf);

	pin_conf.direction  = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(LED0_PIN, &pin_conf);
	port_pin_set_output_level(LED0_PIN, LED0_INACTIVE);
	
	pin_conf.direction  = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(LED1_PIN, &pin_conf);
	port_pin_set_output_level(LED1_PIN, LED1_INACTIVE);

}
