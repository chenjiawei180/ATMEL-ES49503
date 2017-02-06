/*
 * flash.h
 *
 * Created: 2017/2/6 10:40:12
 *  Author: chenjiawei
 */ 


#ifndef FLASH_H_
#define FLASH_H_

#include "asf.h"

extern void Configure_Flash(void);
extern void Bsp_Erase_Row(uint32_t address);
extern void Bsp_Write_Buffer(uint32_t address,uint8_t *buff,uint16_t length);
extern void Bsp_Read_Buffer(uint32_t address,uint8_t *buff,uint16_t length);

#endif /* FLASH_H_ */