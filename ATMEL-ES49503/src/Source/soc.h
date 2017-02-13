/*
 * soc.h
 *
 * Created: 2017/2/7 9:57:18
 *  Author: chenjiawei
 */ 


#ifndef SOC_H_
#define SOC_H_

#include "asf.h"
#include "global.h"

#define CAP_10S_DELAY 40  	//容量扫描时间
#define CAP_MS 250  	//容量扫描时间
#define CAP_CNT_VAL 14400   //1mah


#define BAT_NORMAL_CAPACITY_MIN_AD 6500
#define BAT_NORMAL_CAP 63000                               //zzy? zzy20161019    40000
#define BAT_LEARNNING_CAPACITY_MAX_AD 12000

//电流1A为182,所以判断是多少C的倍率,就是 BAT_NORMAL_CAP/
#define BAT_CAP_3PS                       ((int16_t)(1200))  // BAT_NORMAL_CAP*3/100
#define BAT_CAP_30PS                      ((int16_t)(12000))  // BAT_NORMAL_CAP*3/100
#define CUR_CHG_01C                       (int16_t)(182)  // 1A
#define CUR_CHG_02C                       (int16_t)(1456)  // 1A
#define CUR_DCH_01C                       (int16_t)(-182)  // 1A
#define CUR_DCH_02C                       (int16_t)(-1456)  // 8A

#define CURRENT_DCH_2A                    ((int16_t)-182)  // 2A
#define VCELL_DEEP_DCH                    9830         //2.9V
#define VCELL_DEEP_DCH_CLEAR                    11465         //3.5V
#define VCELL_DEEP_HIGH_CHG                 13433         //4.1V
#define VCELL_DEEP_CHG_CLEAR                 12124         //3.7V

#define VBAT_SOC_UPDATE      500         //150mV    zzy?

extern void NormalCapacityProc(void);
extern uint8_t VbatToSoc(uint16_t vbat_val);
extern void Cap_Update_Check(void);
extern void FullCap_Update(void);
extern void SOC_FLASH_Save(void);
extern void BatCycleProc(void);

#endif /* SOC_H_ */