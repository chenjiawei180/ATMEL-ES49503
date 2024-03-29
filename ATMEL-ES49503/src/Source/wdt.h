/*
 * wdt.h
 *
 * Created: 2017/2/6 10:32:42
 *  Author: chenjiawei
 */ 


#ifndef WDT_H_
#define WDT_H_

#include "asf.h"
#include "global.h"

extern void Configure_Wdt(void);
extern void Wdt_Clear(void);
extern void configure_wdt_callbacks(void);
extern void watchdog_callback(void);

#endif /* WDT_H_ */