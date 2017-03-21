/*
 * protect.h
 *
 * Created: 2017/2/7 14:06:51
 *  Author: chenjiawei
 */ 


#ifndef PROTECT_H_
#define PROTECT_H_

#include "asf.h"
#include "global.h"

#define PROTECT_DELAY_1S  4
#define PROTECT_DELAY_2S  8
#define PROTECT_DELAY_3S  12
#define PROTECT_DELAY_5S  20
#define PROTECT_DELAY_10S  40
#define PROTECT_DELAY_30S  120
#define PROTECT_DELAY_60S  240


#define AFE_OPEN_ADVANCE            0x0002
#define AFE_CLOSE_ADVANCE           0x0000

// VDD50公式=====7.5/2^14 = 7.5/16384     Vx = 7.5/16384*AD
#define VDD50_OA_VAL      12014       //5.5V
#define VDD50_LA_VAL      7645        //3.5V
// VPACK公式=====100/2^14 = 100/16384     Vx = 100/16384*AD
#define ADAPTER_CONNECT_VAL    11009         //67.2V
// VBAT(GPIO1)公式=====5/2^14 = 5/16384     Vx = 5/16384*AD
// 缩小倍数n  0.3/(3+3+3+0.3) = 0.032258     Vx = Vm*n
#define VBAT_FULL           8000         //67.2V  7102
#define VBAT_HIGH_ALARM     6870         //65V
#define VBAT_EMPTY          3000         //45V   4756
#define VBAT_LOW_ALARM      5073         //48V
#define VBAT_SUB_ADAPT      100         //300mV   zzy20161030 从55改为100
//#define VBAT_SUB_VPACK      150         //1.5V  //XXY
#define VBAT_SUB_VPACK      300         //3.0V？    //zzy20161020  300改成500  1.5V?

// VCHG(GPIO2)公式=====5/2^14 = 5/16384     Vx = 5/16384*AD
// 缩小倍数n  0.3/(3+3+3+0.3) = 0.032258     Vx = Vm*n
#define V_ADAPTER           5496         //52V  7102


//CURRENT公式(mV)=====±180/2^15 = 180/32768     Vx = 180/32768*AD
// I = V/R =Vx/0.001

// TMONT(温度)公式=====5/2^14 = 5/16384     Vx = 5/16384*AD
#define TEMP_DCH_LOW_PROTECT    -10         //67.2V
#define TEMP_DCH_LOW_ALARM      -10         //67.2V
#define TEMP_DCH_HIGH_PROTECT   45         //67.2V
#define TEMP_DCH_HIGH_ALARM     45         //67.2V

#define TEMP_PCB_PROTECT   110         //67.2V
#define TEMP_PCB_PROTECT_CLEAR   85         //67.2V

#define TEMP_CHG_LOW_PROTECT    -10         //67.2V
#define TEMP_CHG_LOW_ALARM      -10         //67.2V
#define TEMP_CHG_HIGH_PROTECT   45         //67.2V
#define TEMP_CHG_HIGH_ALARM     45         //67.2V

#define OVER_TEMP_P3 45
#define OVER_TEMP_P4 50
#define OVER_TEMP_P5 55
#define OVER_TEMP_D3 42
#define OVER_TEMP_D4 45

#define CHG_Inhibit_Temp_H 45
#define CHG_Inhibit_Temp_L -10
#define CHG_Inhibit_Temp_HR 40
#define CHG_Inhibit_Temp_LR -5

#define DCH_Inhibit_Temp_H 45
#define DCH_Inhibit_Temp_L -10
#define DCH_Inhibit_Temp_HR 40
#define DCH_Inhibit_Temp_LR -5

// VCELL公式=====5/2^14 = 5/16384     Vx = 5/16384*AD
#define VCELL_HIGH_ERR        14090         //4.3V
#define VCELL_HIGH_PROTECT    13760         //4.2V  无用？
#define VCELL_HIGH_ALARM      13860         //13433         //4.1V  保护值改为4.3V
//#define VCELL_BALANCE         164          //50mV
#define VCELL_LOW_ALARM       9830          //3.1V

#define OCHG_P3_VOL 13631  //4.16V 
#define OCHG_P4_VOL 13926  //4.25V
#define OCHG_P5_VOL 14090  //4.30V
#define OCHG_D3_VOL 13500  //4.12V
#define OCHG_D4_VOL 13763  //4.20V

#define ODCH_P3_VOL 9830   //3.0V
#define ODCH_P4_VOL 9175   //2.8V
#define ODCH_P5_VOL 8192   //2.5V
#define ODCH_D3_VOL 10486  //3.2V
#define ODCH_D4_VOL 10486  //3.2V

#define VCELL_LOW_PROTECT     9502         //2.9V
#define VCELL_SUB_0V5         1638         //0.5V
#define VCELL_ERR             4915         //1.5V
#define VCELL_SHDN            7536         //2.3V

// VCELL公式=====5/2^14 = 5/16384     Vx = 5/16384*AD
#define VCELL_BALANCE_START         164             //50mV
#define VCELL_BALANCE_END           82              //25mV
#define VCELL_BALANCE_Open          12779           //3.9V
#define VCELL_BALANCE_Close         12451           //3.8V

//CURRENT公式(mV)=====±180/2^15 = 180/32768     Vx = 180/32768*AD
// I = V/R =Vx/0.001/1000

//#define CURRENT_DCHOC_PROTECT        ((SHORT)-9100)  // 50A
//#define CURRENT_CHGOC_PROTECT        ((SHORT)6370)  // 35A
#define CURRENT_DCHOC_PROTECT        ((uint16_t)-7282)  //40 25A  zzy20161020
#define CURRENT_CHGOC_PROTECT        ((uint16_t)5097)  //28A zzy20161026 25A4550  zzy20161020
#define CURRENT_CHG1A                ((uint16_t)182)  // 1A
#define CURRENT_CHG2A                ((uint16_t)364)  // 2A
#define CURRENT_DCH_05A              ((uint16_t)-91)  // 0.5A
#define CURRENT_DCH1A                ((uint16_t)-182)  // 0.5A
#define CURRENT_DCH30A                ((uint16_t)-5460)  // 0.5A
#define CURRENT_DCH_STATE            ((uint16_t)-20)  // 0.1A
#define CURRENT_CHG_STATE            ((uint16_t)20)  // 0.1A

//#define OCC_P3_AM 5096 //28A
//#define OCC_P4_AM 5278 //29A
//#define OCC_P5_AM 5460 //30A
//
#define ODC_P3_AM (CURRENT_DCH30A*12/10)
#define ODC_P4_AM (CURRENT_DCH30A*13/10)
#define ODC_P5_AM (CURRENT_DCH30A*15/10)
#define ODC_D3_AM (CURRENT_DCH30A*11/10)
#define ODC_D4_AM (CURRENT_DCH30A*12/10)
#define ODC_D5_AM (CURRENT_DCH30A*12/10)

extern void AFE_Control(void);
extern void Sys_250ms_tick(void);
extern void HardwareProtection(void);
extern void SoftwareProtection(void);
extern void SoftMeansureControl(void);
extern void Cell_Balance(void);
extern void PCB_Protect(void);
extern void SOC(void);
extern void Flag_Process(void);

extern void SOC_Flag(void);
extern void Abnormal_Flag(void);
extern void OCHG_Flag(void);
extern void ODCH_Flag(void);
extern void OCC_Flag(void);
extern void ODC_Flag(void);
extern void OTEMP_Flag(void);
extern void Stop_Flag(void);
extern void Inhibit_Flag(void);

#endif /* PROTECT_H_ */