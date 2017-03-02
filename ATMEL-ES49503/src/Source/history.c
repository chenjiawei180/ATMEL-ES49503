/*
 * history.c
 *
 * Created: 2017/2/7 14:56:05
 *  Author: chenjiawei
 */ 

#include "history.h"
#include "flash.h"

uint8_t his_vcell_min_delay=0;
uint8_t his_vcell_max_delay=0;
uint8_t his_bat_temp_min_delay=0;
uint8_t his_bat_temp_max_delay=0;
uint8_t his_dch_cur_max_delay=0;
uint8_t his_chg_cur_max_delay=0;
uint8_t his_soc_delay=0;
uint8_t his_pcb_temp_max_delay=0;
uint8_t his_sys_err_flags_delay=0;   //xxy0819

/******************************************************************************
FUNCTION		: His_Data_Save
DESCRIPTION		: 历史记录判断存储
INPUT			: none

OUTPUT			: none
NOTICE			:
DATE			: 2016/08/10
******************************************************************************/
void His_Data_Save(void)
{
	static uint16_t last_SYS_ERR_FLAGS_VAL=0;
	//CELL_MIN
	if(nADC_CELL_MIN<g_sys_history.val.vcell_min)
	{
		his_vcell_min_delay++;
		if(his_vcell_min_delay >10)
		{
			his_vcell_min_delay =0;
			EEPROM_Write_DATA(EEPROM_INDEX_HIS_VCELLMIN,nADC_CELL_MIN,1);
		}
	}
	else
	{
		his_vcell_min_delay =0;
	}
	//CELL_MAX
	if(nADC_CELL_MAX>g_sys_history.val.vcell_max)
	{
		his_vcell_max_delay++;
		if(his_vcell_max_delay >10)
		{
			his_vcell_max_delay =0;
			EEPROM_Write_DATA(EEPROM_INDEX_HIS_VCELLMAX,nADC_CELL_MAX,1);
		}
	}
	else
	{
		his_vcell_max_delay =0;
	}
	//BAT_TEMPERTURE_MIN
	if(nADC_TMONI_BAT_MIN<g_sys_history.val.bat_temp_min)
	{
		his_bat_temp_min_delay++;
		if(his_bat_temp_min_delay >10)
		{
			his_bat_temp_min_delay =0;
			EEPROM_Write_DATA(EEPROM_INDEX_HIS_TEMPMIN,nADC_TMONI_BAT_MIN,0);
		}
	}
	else
	{
		his_bat_temp_min_delay =0;
	}
	//BAT_TEMPERTURE_MAX
	if(nADC_TMONI_BAT_MAX>g_sys_history.val.bat_temp_max)
	{
		his_bat_temp_max_delay++;
		if(his_bat_temp_max_delay >10)
		{
			his_bat_temp_max_delay =0;
			EEPROM_Write_DATA(EEPROM_INDEX_HIS_TEMPMAX,nADC_TMONI_BAT_MAX,0);
		}
	}
	else
	{
		his_bat_temp_max_delay =0;
	}
	
	//DCH_CURRENT_MAX
	if((nADC_CURRENT<g_sys_history.val.dch_cur_max)&&(nADC_CURRENT<0))
	{
		his_dch_cur_max_delay++;
		if(his_dch_cur_max_delay >10)
		{
			his_dch_cur_max_delay =0;
			EEPROM_Write_DATA(EEPROM_INDEX_HIS_DCHCMAX,nADC_CURRENT,1);
		}
	}
	else
	{
		his_dch_cur_max_delay =0;
	}
	
	//CHG_CURRENT_MAX
	if((nADC_CURRENT>g_sys_history.val.chg_cur_max)&&(nADC_CURRENT>0))
	{
		his_chg_cur_max_delay++;
		if(his_chg_cur_max_delay >10)
		{
			his_chg_cur_max_delay =0;
			EEPROM_Write_DATA(EEPROM_INDEX_HIS_CHGCMAX,nADC_CURRENT,1);
		}
	}
	else
	{
		his_chg_cur_max_delay =0;
	}
	
	//SOC
	if(g_sys_cap.val.full_cap>g_sys_history.val.soc_max)
	{
		his_soc_delay++;
		if(his_soc_delay >10)
		{
			his_soc_delay =0;
			EEPROM_Write_DATA(EEPROM_INDEX_HIS_SOC_MAX,g_sys_cap.val.full_cap,1);
		}
	}
	else
	{
		his_soc_delay =0;
	}
	
	//PCB_TEMPERTURE_MAX
	if(nADC_TMONI_PCB_MAX>g_sys_history.val.pcb_temp_max)
	{
		his_pcb_temp_max_delay++;
		if(his_pcb_temp_max_delay >10)
		{
			his_pcb_temp_max_delay =0;
			EEPROM_Write_DATA(EEPROM_INDEX_HIS_PCBTEMPMAX,nADC_TMONI_PCB_MAX,1);
		}
	}
	else
	{
		his_pcb_temp_max_delay =0;
	}
	
	//SYS_ERR_FLAGS   xxy0819
	if(last_SYS_ERR_FLAGS_VAL!=sys_err_flags.VAL)
	{
		his_sys_err_flags_delay++;
		if(his_sys_err_flags_delay>10)
		{
			his_sys_err_flags_delay=0;
			last_SYS_ERR_FLAGS_VAL=sys_err_flags.VAL;
			EEPROM_Write_DATA(EEPROM_INDEX_SYS_ERR,sys_err_flags.VAL,1);
			
		}
	}
	else
	{
		his_sys_err_flags_delay=0;
	}
}

/****************************************************************************
FUNCTION		: EEPROM_Write_DATA
DESCRIPTION		: 将数据写入EEPROM
INPUT			: index 数据位置,input_val 数据内容,mode写入的数据是8位还是16位的
OUTPUT			: None
NOTICE			:
DATE			: 2016/08/10
*****************************************************************************/

void EEPROM_Write_DATA(uint16_t index,uint16_t val,uint8_t mode)
{
	uint8_t i,block_crc;
	block_crc = 0;
	if(mode == 1)
	{
		flash_ram_buffer[index] = (uint8_t)(val>>8);
		flash_ram_buffer[index+1] = (uint8_t)(val);
		for(i=0;i<EEPROM_INDEX_CRC;i++)
		{
			block_crc += flash_ram_buffer[i];
		}
		Bsp_Erase_Row(EEPROM_SYS_BEGIN);
		Bsp_Erase_Row(EEPROM_SYS_BK_BEGIN);
		Bsp_Write_Buffer(EEPROM_SYS_BEGIN,flash_ram_buffer,EEPROM_SYS_DATA_LEN);
		Bsp_Write_Buffer(EEPROM_SYS_BK_BEGIN,flash_ram_buffer,EEPROM_SYS_DATA_LEN);
	}
	else
	{
		flash_ram_buffer[index] = (uint8_t)(val);
		for(i=0;i<EEPROM_INDEX_CRC;i++)
		{
			block_crc += flash_ram_buffer[i];
		}
		Bsp_Erase_Row(EEPROM_SYS_BEGIN);
		Bsp_Erase_Row(EEPROM_SYS_BK_BEGIN);
		Bsp_Write_Buffer(EEPROM_SYS_BEGIN,flash_ram_buffer,EEPROM_SYS_DATA_LEN);
		Bsp_Write_Buffer(EEPROM_SYS_BK_BEGIN,flash_ram_buffer,EEPROM_SYS_DATA_LEN);
	}
}

void Write_Time_or_mAh(uint32_t value,uint8_t type)
{
	uint32_t address = DCH_FLAG_START;
	uint8_t i = 0;
	uint8_t buff[8];
	switch(type)    //根据类型区分地址
	{
		case DCH_FLAG: address = DCH_FLAG_START;break;
		case CHG_FLAG: address = CHG_FLAG_START;break;
		case TIME_FLAG: address = TIME_FLAG_START;break;
		default:address = DCH_FLAG_START;break;
	}
	Bsp_Read_Buffer(address+256,buff,8);
	if (buff[0] == 0xff) //第二扇区没有数据 直接从第一扇区开始找
	{
		for (i=0;i<64;i++)
		{
			Bsp_Read_Buffer(address+(i<<3),buff,8);
			if (buff[0] == 0xff)
			{
				buff[0] = 0x00;
				buff[1] = 0x00;
				buff[2] = value>>24;
				buff[3] = value>>16;
				buff[4] = value>>8;
				buff[5] = value;
				buff[6] = 0x00;
				buff[7] = 0x00;
				Bsp_Write_Buffer(address+(i<<3),buff,8);
				if (i == 16)
				{
					Bsp_Erase_Row(address+256);
				}
				break;
			}
		}
	}
	else //第二商区有数据，先找第二扇区
	{
		for (i=0;i<32;i++)
		{
			Bsp_Read_Buffer(address+256+(i<<3),buff,8);
			if (buff[0] == 0xff)
			{
				buff[0] = 0x00;
				buff[1] = 0x00;
				buff[2] = value>>24;
				buff[3] = value>>16;
				buff[4] = value>>8;
				buff[5] = value;
				buff[6] = 0x00;
				buff[7] = 0x00;
				Bsp_Write_Buffer(address+256+(i<<3),buff,8);
				if (i == 16)
				{
					Bsp_Erase_Row(address);
				}
				break;
			}
		}
		if (i == 32)
		{
			for (i=0;i<32;i++)
			{
				Bsp_Read_Buffer(address+(i<<3),buff,8);
				if (buff[0] == 0xff)
				{
					buff[0] = 0x00;
					buff[1] = 0x00;
					buff[2] = value>>24;
					buff[3] = value>>16;
					buff[4] = value>>8;
					buff[5] = value;
					buff[6] = 0x00;
					buff[7] = 0x00;
					Bsp_Write_Buffer(address+(i<<3),buff,8);
					if (i == 16)
					{
						Bsp_Erase_Row(address+256);
					}
					break;
				}
			}
		}
	}
}

uint32_t Read_Time_or_mAh(uint8_t type) 
{
	uint32_t address = DCH_FLAG_START;
	uint32_t val_temp = 0;
	uint8_t i = 0;
	uint8_t buff[8] = {0};
	switch(type)    //根据类型区分地址
	{
		case DCH_FLAG: address = DCH_FLAG_START;break;
		case CHG_FLAG: address = CHG_FLAG_START;break;
		case TIME_FLAG: address = TIME_FLAG_START;break;
		default:address = DCH_FLAG_START;break;
	}
	Bsp_Read_Buffer(address,buff,8);// 第一字节为FF表示无数据  第二字节无效  第 3 4 5 6 为32位数据 7 8 字节无效
	if (buff[0] == 0xff)
	{
		address = address + 256;
		Bsp_Read_Buffer(address,buff,8);
		if (buff[0] == 0xff)
		{
			//flash没数据 重新初始化
		}
		else
		{
			for (i=0;i<32;i++)
			{
				Bsp_Read_Buffer(address+(i<<3),buff,8);
				if (buff[0] == 0xff)
				{
					Bsp_Read_Buffer(address+((i-1)<<3),buff,8);
					break;
				}
			}
			if (i == 32)
			{
				Bsp_Read_Buffer(address+((i-1)<<3),buff,8);
			}
		}
	}
	else
	{
		for (i=0;i<64;i++)
		{
			Bsp_Read_Buffer(address+(i<<3),buff,8);
			if (buff[0] == 0xff)
			{
				Bsp_Read_Buffer(address+((i-1)<<3),buff,8);
				break;
			}
		}
	}
	
	val_temp = buff[2]<<24 | buff[3]<<16 | buff[4]<<8 | buff[5];
	if(val_temp == 0XFFFFFFFF) val_temp = 0;
	
	return val_temp;
}

void Time_update(void)
{
	static cal_ms = 0;
	cal_ms++;
	if (cal_ms >= 4)
	{
		cal_ms = 0;
		Time_Val++;
		if (Time_Val - Time_Val_Bak >= 60)
		{
			Write_Time_or_mAh(Time_Val,TIME_FLAG);
			Time_Val_Bak = Time_Val;
		}
	}
}

