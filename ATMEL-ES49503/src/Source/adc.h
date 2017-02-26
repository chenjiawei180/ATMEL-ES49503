/*
 * adc.h
 *
 * Created: 2017/2/23 9:14:58
 *  Author: chenjiawei
 */ 


#ifndef ADC_H_
#define ADC_H_

#include "asf.h"
#include "global.h"

extern void Configure_Adc(void);
extern void Adc_Read_AdcValue(uint16_t *result);

#endif /* ADC_H_ */