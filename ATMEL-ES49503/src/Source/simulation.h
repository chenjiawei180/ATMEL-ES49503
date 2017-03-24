/*
 * simulation.h
 *
 * Created: 2017/3/23 9:26:54
 *  Author: chenjiawei
 */ 


#ifndef SIMULATION_H_
#define SIMULATION_H_

#include "asf.h"
#include "global.h"

extern void Configure_Tc(void);
extern void Configure_Tc_Callbacks(void);
extern void tc_callback_to_toggle_led(struct tc_module *const module_inst);
extern void Can_var(uint8_t* buff);
extern void Sim_process(void);
extern void tc_switch(void);


#endif /* SIMULATION_H_ */