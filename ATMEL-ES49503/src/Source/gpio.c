/*
 * gpio.c
 *
 * Created: 2017/2/23 9:53:10
 *  Author: chenjiawei
 */ 

#include "gpio.h"

void Configure_GPIO(void)
{
	struct port_config pin_conf;
	port_get_config_defaults(&pin_conf);
	
	pin_conf.direction  = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(MCU_STOP_PIN, &pin_conf);
	port_pin_set_output_level(MCU_STOP_PIN, true);
	
	pin_conf.direction  = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(ID_OUT_PIN, &pin_conf);
	port_pin_set_output_level(ID_OUT_PIN, false);
	
	pin_conf.direction  = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(COM_RES_PIN, &pin_conf);
	port_pin_set_output_level(COM_RES_PIN, false);
	
	pin_conf.direction  = PORT_PIN_DIR_INPUT;
	port_pin_set_config(ID_IN_PIN, &pin_conf);
	
	pin_conf.direction  = PORT_PIN_DIR_INPUT;
	port_pin_set_config(ID_END_PIN, &pin_conf);
	
	pin_conf.direction  = PORT_PIN_DIR_INPUT;
	port_pin_set_config(SOV_PIN, &pin_conf);
	
}