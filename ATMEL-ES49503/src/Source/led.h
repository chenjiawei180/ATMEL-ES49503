/*
 * led.h
 *
 * Created: 2017/2/6 10:29:56
 *  Author: chenjiawei
 */ 


#ifndef LED_H_
#define LED_H_

#include "asf.h"
#include "global.h"

/********************************* LED0 **********************************************/
#define LED0_PIN                  PIN_PA27
#define LED0_ACTIVE               true
#define LED0_INACTIVE             !LED0_ACTIVE
#define Bsp_LED0_On()             port_pin_set_output_level(LED0_PIN, LED0_ACTIVE)
#define Bsp_LED0_Off()            port_pin_set_output_level(LED0_PIN, LED0_INACTIVE)
#define Bsp_LED0_Toggle()         port_pin_toggle_output_level(LED0_PIN)

/********************************* LED1 **********************************************/
#define LED1_PIN                  PIN_PA28
#define LED1_ACTIVE               true
#define LED1_INACTIVE             !LED1_ACTIVE
#define Bsp_LED1_On()             port_pin_set_output_level(LED1_PIN, LED1_ACTIVE)
#define Bsp_LED1_Off()            port_pin_set_output_level(LED1_PIN, LED1_INACTIVE)
#define Bsp_LED1_Toggle()         port_pin_toggle_output_level(LED1_PIN)

extern void Configure_Led(void);
extern void SysLED_Display(void);
extern void SysLED_RunProtect(void);
extern void SysLED_ChgNormal(void);
extern void SysLED_RunNormal(void);

#endif /* LED_H_ */