/*
 * gpio.h
 *
 * Created: 2017/2/23 9:26:42
 *  Author: chenjiawei
 */ 


#ifndef GPIO_H_
#define GPIO_H_

#include "asf.h"
#include "global.h"

/********************************* MCU_STOP **********************************************/
#define MCU_STOP_PIN                     PIN_PB09
#define MCU_STOP_Low()					 port_pin_set_output_level(MCU_STOP_PIN, false)
#define MCU_STOP_High()					 port_pin_set_output_level(MCU_STOP_PIN, true)
#define MCU_STOP_Toggle()				 port_pin_toggle_output_level(MCU_STOP_PIN)

/********************************* ID_OUT  **********************************************/
#define ID_OUT_PIN                       PIN_PA05
#define ID_OUT_Low()				     port_pin_set_output_level(ID_OUT_PIN, false)
#define ID_OUT_High()				     port_pin_set_output_level(ID_OUT_PIN, true)
#define ID_OUT_Toggle()				     port_pin_toggle_output_level(ID_OUT_PIN)

/********************************* COM_RES **********************************************/
#define COM_RES_PIN                      PIN_PA11
#define COM_RES_Low()				     port_pin_set_output_level(COM_RES_PIN, false)
#define COM_RES_High()				     port_pin_set_output_level(COM_RES_PIN, true)
#define COM_RES_Toggle()			     port_pin_toggle_output_level(COM_RES_PIN)

/********************************* ID_IN **********************************************/
#define ID_IN_PIN					     PIN_PA04
#define ID_IN_Read()				     port_pin_get_input_level(ID_IN_PIN)

/********************************* ID_END **********************************************/
#define ID_END_PIN					     PIN_PA06
#define ID_END_Read()				     port_pin_get_input_level(ID_END_PIN)

/********************************* SOV  **********************************************/
#define SOV_PIN							 PIN_PB04
#define SOV_Read()						 port_pin_get_input_level(SOV_PIN)

extern void Configure_GPIO(void);

#endif /* GPIO_H_ */