/*
 * simulation.c
 *
 * Created: 2017/3/23 9:30:55
 *  Author: chenjiawei
 */ 

#include "simulation.h"
#include "protect.h"
#include "led.h"

#ifdef SIMULATION_AFE

struct tc_module tc_instance;

void Configure_Tc(void)
{
	struct tc_config config_tc;
	tc_get_config_defaults(&config_tc);
	
	config_tc.counter_size = TC_COUNTER_SIZE_8BIT;
	config_tc.clock_source = GCLK_GENERATOR_1;
	config_tc.clock_prescaler = TC_CLOCK_PRESCALER_DIV256;
	config_tc.counter_8_bit.period = 31;
	//config_tc.counter_8_bit.compare_capture_channel[0] = 50;
	//config_tc.counter_8_bit.compare_capture_channel[1] = 54;
	
	tc_init(&tc_instance, TC0, &config_tc);
	tc_enable(&tc_instance);
	Configure_Tc_Callbacks();
}

void Configure_Tc_Callbacks(void)
{
	tc_register_callback(&tc_instance, tc_callback_to_toggle_led,TC_CALLBACK_OVERFLOW);
	tc_enable_callback(&tc_instance, TC_CALLBACK_OVERFLOW);
}

void tc_callback_to_toggle_led(
struct tc_module *const module_inst)
{
	Sys_250ms_tick();
	TC_250ms_flag = 1;
	Bsp_LED0_Toggle();
	Bsp_LED1_Toggle();
}

void Sim_process(void)
{
	Abnormal_Flag();
	Flag_Process();
}

void Can_var(uint8_t* buff)
{
	nADC_CELL_MAX = (uint16_t)( (buff[2]<< 8 | buff[3]) /5 *16384 /1000 ) ;
	nADC_CELL_MIN = (uint16_t)( (buff[4]<< 8 | buff[5]) /5 *16384 /1000 ) ;
	nADC_CURRENT  = (int16_t) ( ((int16_t)(buff[6]<< 8 | buff[7])) *182 /100);	
	nADC_TMONI_BAT_MAX = (int8_t)buff[8];
	nADC_TMONI_BAT_MIN = (int8_t)buff[9];
	g_sys_cap.val.re_cap_rate = (uint8_t)buff[10];	
}

#endif