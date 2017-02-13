/*
 * power.c
 *
 * Created: 2017/2/7 8:57:56
 *  Author: chenjiawei
 */ 

#include "power.h"
#include "soc.h"
#include "led.h"
#include "spi.h"

/**
  * @brief  上电初始化. 
  * @param  None
  * @retval None
  */

void PowerOn_Init(void)
{
	SHDN_Low();
	Bsp_LED0_On();
	Bsp_LED1_On();
    delay_ms(500);
	Bsp_LED0_Off();
	Bsp_LED1_Off();
	
    sys_flags.VAL =0;
    sys_states.VAL =0;
    afe_flags.VAL =0;
    flash_flags.VAL =0;
    sys_err_flags.VAL =0;
	cap_update = BAT_NORMAL_CAP;
	nADC_CURRENT = 0;
	g_bal_state.VAL = 0;
	g_bal_need.VAL = 0;
	PWR_VALUE = 0;
	Total_VBAT = 0;
	AFE_OC_DELAY_CNT = 0;
	AFE_SCD_DELAY_CNT = 0;
	AFE_OCC_DELAY_CNT = 0;
	
    sys_states.val.sys_sw_nconnect_flag=1;//zzy20161021 初始值为断开连接
    g_sys_cap.val.cycle_record_flag =0;
    sys_flags.val.afe_connect_flag =1;
    sys_states.val.sys_dch_on =0;
    sys_states.val.sys_chg_on =0;
    sys_flags.val.afe_adirq2_flag = 1;
    g_sys_cap.val.cap_cnt =0;
    g_sys_cap.val.bat_cycle_cnt =0;
    g_sys_cap.val.re_cap_rate_sum =0;
    g_sys_cap.val.deep_rate_sum =0;
	
	//历史信息初始化,为了方便判断,初始化全部配置极限值
	g_sys_history.val.bat_temp_max =0;
	g_sys_history.val.bat_temp_min =100;
	g_sys_history.val.chg_cur_max =0;
	g_sys_history.val.dch_cur_max =0;
	g_sys_history.val.pcb_temp_max =0;
	g_sys_history.val.soc_max =0;
	g_sys_history.val.vcell_max =0;
	g_sys_history.val.vcell_min =0xffff;
}