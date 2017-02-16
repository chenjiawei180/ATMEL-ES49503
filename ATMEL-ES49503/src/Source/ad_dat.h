/*
 * ad_dat.h
 *
 * Created: 2017/2/7 14:02:50
 *  Author: chenjiawei
 */ 


#ifndef AD_DAT_H_
#define AD_DAT_H_

#include "asf.h"
#include "global.h"

extern void vAPI_ADC_Read_Data_bal_1(void);
extern void vAPI_ADC_Read_Data_bal_2(void);
extern void vAPI_ADC_Read_Data(void);
extern int8_t vAPI_IndexNtcTemp(uint16_t temp_ad_val);
extern void vAPI_CalcCell(void);
extern void vAPI_CalcTempture(void);
extern uint8_t AFE_HardwareProtection_Write(void);

//for debug
extern void vAPI_Uart_Load(void);

#endif /* AD_DAT_H_ */