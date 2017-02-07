/*
 * flash.h
 *
 * Created: 2017/2/6 10:40:12
 *  Author: chenjiawei
 */ 


#ifndef FLASH_H_
#define FLASH_H_

#include "asf.h"
#include "global.h"

#define EEPROM_SYS_BEGIN    0x3fe00
#define EEPROM_SYS_BK_BEGIN  0x3ff00

#define EEPROM_INDEX_BEGIN   0
#define EEPROM_INDEX_CAP_VAL   1   //容量百分比
#define EEPROM_INDEX_FULL_CAP   2
#define EEPROM_INDEX_CYCLE   4
#define EEPROM_INDEX_DEEP_DCH_CYCLE   6
#define EEPROM_INDEX_DEEP_CHG_CYCLE   8
#define EEPROM_INDEX_SYS_ERR   10  //未加入
#define EEPROM_INDEX_HIS_VCELLMIN   12
#define EEPROM_INDEX_HIS_VCELLMAX   14
#define EEPROM_INDEX_HIS_TEMPMIN   16
#define EEPROM_INDEX_HIS_TEMPMAX   17
#define EEPROM_INDEX_HIS_DCHCMAX   18
#define EEPROM_INDEX_HIS_CHGCMAX   20
#define EEPROM_INDEX_HIS_SOC_MAX   22
#define EEPROM_INDEX_HIS_PCBTEMPMAX   24
#define EEPROM_INDEX_CRC   25

extern void Configure_Flash(void);
extern void Bsp_Erase_Row(uint32_t address);
extern void Bsp_Write_Buffer(uint32_t address,uint8_t *buff,uint16_t length);
extern void Bsp_Read_Buffer(uint32_t address,uint8_t *buff,uint16_t length);
extern void SYS_EEPROM_Init(void);
extern void EEPROM_To_RAM(void);
extern void EEPROM_BACKUP_READ(void);
extern void EEPROM_Init(void);
#endif /* FLASH_H_ */