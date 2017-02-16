/*
 * global.h
 *
 * Created: 2017/2/7 8:06:46
 *  Author: chenjiawei
 */ 


#ifndef GLOBAL_H_
#define GLOBAL_H_

#ifdef OS_GLOBALS
#define OS_EXT
#else
#define OS_EXT extern
#endif

typedef union
{
	struct
	{
		uint16_t afe_connect_flag								: 1;
		uint16_t timewait_250ms                         		: 1;
		uint16_t afe_set_hard_protect_end                       : 1;
		uint16_t afe_adc_done_flag                              : 1;
		uint16_t afe_power_on_flag                              : 1;
		uint16_t advance_done_flag                              : 1;
		uint16_t cell_low_alarm_flag                            : 1;
		uint16_t cell_high_alarm_flag							: 1;

		uint16_t afe_adirq2_flag                                : 1;
		uint16_t re_cap_update_flag								: 1;
		uint16_t full_cap_update_flag                           : 1;
		uint16_t afe_volt_protect_flag                          : 1;
		uint16_t cap_update_end_flag                            : 1;
		uint16_t                                                : 1;
		uint16_t                                                : 1;
		uint16_t                                                : 1;

	}val;
	uint16_t VAL;
}SystemFlags;

typedef union
{
	struct
	{
		uint16_t sys_dch_on								: 1;
		uint16_t sys_chg_on                         	: 1;
		uint16_t sys_chg_state                          : 1;
		uint16_t sys_dch_state                          : 1;
		uint16_t sys_software_occ                       : 1;
		uint16_t sys_software_odc     					: 1;
		uint16_t sys_advance_flag        				: 1;
		uint16_t adapter_connect_flag					: 1;

		uint16_t soft_chg_protect						: 1;
		uint16_t soft_dch_protect						: 1;
		uint16_t chg_temp_protect                       : 1;
		uint16_t dch_temp_protect                       : 1;
		uint16_t sys_sw_lowpower_flag                   : 1;//nu zzy20161028 0Ϊ����ģʽ 1Ϊ�͹���ģʽnu                                     : 1;
		uint16_t sys_pcb_ot_flag                        : 1;
		uint16_t sys_sw_nconnect_flag                   : 1;
		uint16_t sys_err_flag                           : 1;
		
	}val;
	uint16_t VAL;
}SystemStates;

typedef union
{
	struct
	{
		uint16_t afe_chgmos_flag							: 1;
		uint16_t afe_dchmos_flag                         	: 1;
		uint16_t afe_fdr_rcv_flag                           : 1;
		uint16_t afe_ov_protect                             : 1;
		uint16_t afe_occ_flag                               : 1;
		uint16_t afe_ocd_flag                               : 1;
		uint16_t afe_scd_flag                               : 1;
		uint16_t afe_set_volt_protect_err_flag				: 1;

		uint16_t afe_uv_flag                                : 1;
		uint16_t afe_ov_flag								: 1;
		uint16_t afe_read_reg_err_flag                      : 1;
		uint16_t afe_fdr_protect_flag                       : 1;
		uint16_t afe_uv_lock_flag                           : 1;
		uint16_t afe_dfrv_autoprotect_flag                  : 1;
		uint16_t afe_CellBalance                            : 1;        //afe_2
		uint16_t afe_FET_CLR_BIT                            : 1;
		
	}val;
	uint16_t VAL;
}AFEProtectFlags;

typedef union
{
	struct
	{
		uint16_t re_cap_update_flag								: 1;
		uint16_t timewait_250ms                         		: 1;
		uint16_t afe_set_hard_protect_end                       : 1;
		uint16_t afe_adc_done_flag                              : 1;
		uint16_t afe_power_on_flag                              : 1;
		uint16_t temp_dch_low_protect_flag     					: 1;
		uint16_t temp_dch_high_protect_flag     				: 1;
		uint16_t en_comm_flag								    : 1;

		uint16_t afe_adirq2_flag								: 1;
		uint16_t power_first_flag								: 1;
		uint16_t deep_low_v_flag                           		: 1;
		uint16_t flash_err_flag                                 : 1;
		uint16_t en_deep_sleep_flag                             : 1;
		uint16_t                                                : 1;
		uint16_t                                                : 1;
		uint16_t                                                : 1;
		
	}val;
	uint16_t VAL;
}FLASH_FLAGS;

typedef union
{
	struct
	{
		uint16_t fcc_update_err_flag						: 1;
		uint16_t cell_err_flag                              : 1;
		uint16_t dch_mos_err_flag                         	: 1;
		uint16_t chg_mos_err_flag                           : 1;
		uint16_t sys_advance_error_flag                     : 1;
		uint16_t                                            : 1;
		uint16_t                                            : 1;
		uint16_t                                            : 1;

		unsigned                                            : 8;
	}val;
	uint16_t VAL;
}SystemErrFlags;

typedef union
{
	struct{
		uint16_t  	vcell_min;
		uint16_t   	vcell_max;
		
		int8_t      bat_temp_min;
		int8_t      bat_temp_max;
		
		int16_t     dch_cur_max;
		int16_t  	chg_cur_max;
		
		uint16_t    soc_max;   //�����洢��ѹ���ó���ʣ�������ٷֱ�
		int8_t      pcb_temp_max;

	}val;
}__SYS_HISTORY;

typedef union
{
	struct{
		uint16_t  	full_cap;
		uint16_t   	bat_cap;
		int32_t     cap_val;
		int32_t     cap_val2;
		int32_t     cap_val3;       //����socֵ20161010zzysoc4  ��������¼ ���val3 ͨ��Ϊ�����valΪ��

		uint16_t    cycle_cap_val;	//ѭ��������������������ѭ������

		uint8_t  	re_cap_rate;
		uint8_t     re_cap_rate2;   //�����洢��ѹ���ó���ʣ�������ٷֱ�
		uint8_t     re_cap_rate_old;
		uint8_t     re_cap_rate_old2;   //����socֵ20161010zzysoc3  ���ڼ�¼soc�仯ǰֵ����socÿ������
		uint8_t     re_cap_rate_record;
		int8_t      re_cap_rate_sum;
		
		uint8_t     cycle_record_flag;
		
		uint8_t     deep_cycle_rate_record;
		uint8_t     deep_rate_sum;
		uint8_t     deep_dch_volt_delay;

		uint8_t     deep_cycle_chg_record;
		uint8_t     deep_rate_chgsum;
		uint8_t     deep_chg_volt_delay;
		
		int32_t  	cap_cnt;
		uint16_t    bat_cycle_cnt;
		uint16_t    deep_dch_cycle_cnt;
		uint16_t    deep_chg_cycle_cnt;
		//        UCHAR   bat_cycle_record;
	}val;
}__SYS_CAP;

typedef union
{
	struct
	{
		unsigned en_c01_ban_flag								: 1;
		unsigned en_c02_ban_flag                         		: 1;
		unsigned en_c03_ban_flag                                : 1;
		unsigned en_c04_ban_flag                                : 1;
		unsigned en_c05_ban_flag                                : 1;
		unsigned en_c06_ban_flag                                : 1;
		unsigned en_c07_ban_flag                                : 1;
		unsigned en_c08_ban_flag                                : 1;

		unsigned en_c09_ban_flag								: 1;
		unsigned en_c10_ban_flag                         		: 1;
		unsigned en_c11_ban_flag                                : 1;
		unsigned en_c12_ban_flag                                : 1;
		unsigned en_c13_ban_flag                                : 1;
		unsigned en_c14_ban_flag                                : 1;
		unsigned en_c15_ban_flag                                : 1;
		unsigned en_c16_ban_flag                                : 1;
	}val;
	uint16_t VAL;
}Cell_Balanc_state;

#define		MAC_AN49503_READ_ADR		(uint8_t)0x01
#define 	MAC_AN49503_READ_CNT        (uint8_t)0x56		/* from 0x00 to 0x100 */
#define     MAC_SPI_DEV                 (uint8_t)0xE0

#include "usart.h"

OS_EXT SystemFlags sys_flags;
OS_EXT SystemStates sys_states;
//CellBalance cell_balance;
OS_EXT AFEProtectFlags afe_flags;
OS_EXT FLASH_FLAGS flash_flags;
OS_EXT SystemErrFlags sys_err_flags;
OS_EXT volatile __SYS_HISTORY g_sys_history;
OS_EXT volatile __SYS_CAP g_sys_cap;
OS_EXT Cell_Balanc_state g_bal_state;
OS_EXT Cell_Balanc_state g_bal_need;

#define EEPROM_SYS_DATA_LEN  26
OS_EXT uint8_t flash_ram_buffer[EEPROM_SYS_DATA_LEN];
OS_EXT volatile uint16_t cap_update;
OS_EXT volatile uint8_t sleep_delay_cycle;

OS_EXT uint8_t	 ucSPI_Conti_RecvData[MAC_AN49503_READ_CNT*2+3];	/* receive continuous data 174byte+CRC(1Byte)	 MAC_AN49503_READ_CNT*2+1 */
OS_EXT uint16_t  AVE_CURRENT[4];
OS_EXT uint16_t nADC_TMONI_PCB_MAX;
OS_EXT uint16_t nADC_TMONI_BAT_MAX;
OS_EXT uint16_t nADC_TMONI_BAT_MIN;
OS_EXT uint16_t nADC_CELL_MAX;
OS_EXT uint16_t nADC_CELL_MIN;
OS_EXT int16_t nADC_CURRENT;
#define RAM_P_CELL_SEREIES  16
OS_EXT uint16_t nADC_Cell_Value[RAM_P_CELL_SEREIES];
OS_EXT uint16_t PWR_VALUE;
OS_EXT uint16_t spi_read_value[2];
OS_EXT uint16_t Total_VBAT;
OS_EXT volatile uint8_t AFE_OC_DELAY_CNT;
OS_EXT volatile uint8_t AFE_SCD_DELAY_CNT;
OS_EXT volatile uint8_t AFE_OCC_DELAY_CNT;


// for debug
#define UartTxLength 68
OS_EXT uint8_t TxBuffer[UartTxLength];

#endif /* GLOBAL_H_ */