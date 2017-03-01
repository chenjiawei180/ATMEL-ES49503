/*
 * history.h
 *
 * Created: 2017/2/7 14:55:12
 *  Author: chenjiawei
 */ 


#ifndef HISTORY_H_
#define HISTORY_H_

#include "asf.h"
#include "global.h"

extern void His_Data_Save(void);
extern void EEPROM_Write_DATA(uint16_t index,uint16_t val,uint8_t mode);
extern void Write_Time_or_mAh(uint32_t value,uint8_t type);
extern uint32_t Read_Time_or_mAh(uint8_t type);

#endif /* HISTORY_H_ */