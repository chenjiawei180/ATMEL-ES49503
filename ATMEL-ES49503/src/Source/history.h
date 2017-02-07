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

#endif /* HISTORY_H_ */