/*
 * afe_wr.h
 *
 * Created: 2017/2/7 13:32:07
 *  Author: chenjiawei
 */ 


#ifndef AFE_WR_H_
#define AFE_WR_H_

#include "asf.h"
#include "global.h"
#include "AN49503.h"

#define AFE_UNLOCK                 0xE3B5
#define AFE_LOCK                   0x0000

#define AFE_OTHER_AD_ALL_ON        0x0FFF
#define AFE_GPIO456_GPIO1_EN       0x0703
#define AFE_OV45_UV30              0x3400
#define AFE_ALARM_3V5              0x3400
#define AFE_TM_PULLUP              0x1F00
#define AFE_ADC_EN_CONT            0x0348
#define AFE_ADC_EN_TRG             0x4010
#define AFE_ADIH_EN                0x2031
#define AFE_ADIL_EN                0x1032

#define AFE_ADIQ1_AMARM2_EN        0x2502
#define AFE_ADIQ2_AMARM2_EN        0x2442
#define AFE_VAD_DONE               0x0007

#define AFE_Balance_EN             0x0000   // 开启均衡
#define AFE_Balance_Dis            0x0001   // 关闭均衡
#define AFE_AVD_LATCH              0x4007
#define AFE_HARDWARE_ALARM_EN      0x000F
#define AFE_200A_75A_40A           0x0C43 //0x0000//zzy20161026 测试        AFE_100A_50A_40A           0x0423   // 00 0001 00001 00011 zzy20161020 //AFE_200A_75A_50A           0x0C44   // 00 0011 00010 00100
#define AFE_50us_1ms_1ms           0x01EF

#define AFE_H420V_L275V            0x2e2d //#define AFE_H425V_L275V            0x2f2d//  取消zzy改为4.25v减0.02为4.23V

#define AFE_UV_350mV_1S            0x0300 //以取消#define AFE_OV_200mV_UV_350mV_1S            0x2300//  zzy??原应该为欠压延后250，其他默认，先改为高压提前200mV
#define AFE_FET_AUTO_CONTROL       0x8000
//20160909
#define AFE_FET_CLR_BIT            0x2000
//20160907 xxy zzy
//#define AFE_FET_CLR                0xE000   //OV/uv条件恢复时还需要其他报警也恢复才恢复
#define AFE_FET_CLR                0xA000      //OV/uv条件恢复时立即复位

#define AFE_SPI_NM50               0x0701
#define AFE_SPI_LP50               0x0711
#define AFE_SCD_OCD_OCC            0x0070

#define ST_PROTECT    0x0370
#define ST_OV         0x0200
#define ST_UV         0x0100
#define ST_SCD        0x0040
#define ST_OCD        0x0020
#define ST_CCD        0x0010

#define  AFE_CELL13S                0xFE3F//e00f zzy20161020 AFE_CELL14S                0xFF3F
#define  AFE_CELL13S_P              0x01C0//1ff0  zzy20161020 AFE_CELL14S_P              0x00C0

extern void AFE_Init(void);
extern void AFE_Reg_Read(void);
extern void SPI_AllReg_WR(void);
extern void Cells_Bal_Close(void);
extern uint16_t AFE_ONE_VPC_ADC(void);
extern void AFE_HardwareProtection_Read(void);
extern void Cells_Bal_Judge(void);
extern void Cells_Bal_Open(void);


#endif /* AFE_WR_H_ */