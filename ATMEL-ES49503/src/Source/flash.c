/*
 * flash.c
 *
 * Created: 2017/2/6 10:40:57
 *  Author: chenjiawei
 */ 

#include "flash.h"

/**
  * @brief  This function is Initialization GPIO. default state is High.
  * @param  None
  * @retval None
  */

void Configure_Flash(void)
{
	struct nvm_config config_nvm;
	nvm_get_config_defaults(&config_nvm);
	config_nvm.manual_page_write = false;
	nvm_set_config(&config_nvm);
}

/**
  * @brief  This function is erase the flash.
  * @param  address
  * @retval None
  */

void Bsp_Erase_Row(uint32_t address)
{
	enum status_code error_code;
	do
	{
		error_code = nvm_erase_row(address);
	} while (error_code == STATUS_BUSY);
}

/**
  * @brief  This function is write buffer to flash.
  * @param  address,point of buffer,length
  * @retval None
  */

void Bsp_Write_Buffer(uint32_t address,uint8_t *buff,uint16_t length)
{
	enum status_code error_code;
	do
	{
		error_code = nvm_write_buffer(address,buff, length);
	} while (error_code == STATUS_BUSY);
}

/**
  * @brief  This function is read buffer to flash.
  * @param  address,point of buffer,length
  * @retval None
  */

void Bsp_Read_Buffer(uint32_t address,uint8_t *buff,uint16_t length)
{
	enum status_code error_code;
	do
	{
		error_code = nvm_read_buffer(address,buff, length);
	} while (error_code == STATUS_BUSY);
}

/**
  * @brief  上电 EEPROM 读取与初始化
  * @param  None
  * @retval None
  */

void SYS_EEPROM_Init(void)
{
	uint8_t i;
	uint8_t block_crc;
   
	Bsp_Read_Buffer(EEPROM_SYS_BEGIN,flash_ram_buffer,EEPROM_SYS_DATA_LEN);
	//FLASH块读取
	if(flash_ram_buffer[0] == 0xB3)
	{
		//判断块1,不正常判断备份块4
		//BLOCK1分配：HEAD+FULLCAp*2+CYCLES*2+CRC
		block_crc =0;
		for(i=0;i<EEPROM_INDEX_CRC;i++)
		{
			block_crc += flash_ram_buffer[i];
		}
		if(block_crc == flash_ram_buffer[EEPROM_INDEX_CRC])
		{
			EEPROM_To_RAM();
			//iap_vision =  DATAEE_ReadByte(EEPROM_INDEX_VISION); //DATAEE_ReadByte
			//iap_vision <<=8;
			//iap_vision |=  DATAEE_ReadByte(EEPROM_INDEX_VISION+1); //DATAEE_ReadByte
		}
		else
		{
			EEPROM_BACKUP_READ();
		}

	}
	else
	{
		EEPROM_BACKUP_READ();
	}
}

/**
  * @brief  EEPROM读取成功，恢复变量
  * @param  None
  * @retval None
  */

void EEPROM_To_RAM(void)
{
	g_sys_cap.val.re_cap_rate = flash_ram_buffer[EEPROM_INDEX_CAP_VAL];
	g_sys_cap.val.re_cap_rate_old = g_sys_cap.val.re_cap_rate;
	g_sys_cap.val.re_cap_rate_old2 = g_sys_cap.val.re_cap_rate;   //修正soc值20161010zzysoc3

	//cap_update =  flash_ram_buffer[EEPROM_INDEX_FULL_CAP];
	//cap_update <<=8;
	//cap_update |=  flash_ram_buffer[EEPROM_INDEX_FULL_CAP+1];

	g_sys_cap.val.bat_cycle_cnt =  flash_ram_buffer[EEPROM_INDEX_CYCLE];
	g_sys_cap.val.bat_cycle_cnt <<=8;
	g_sys_cap.val.bat_cycle_cnt |=  flash_ram_buffer[EEPROM_INDEX_CYCLE+1];
	    
	g_sys_cap.val.deep_dch_cycle_cnt =  flash_ram_buffer[EEPROM_INDEX_DEEP_DCH_CYCLE];
	g_sys_cap.val.deep_dch_cycle_cnt <<=8;
	g_sys_cap.val.deep_dch_cycle_cnt |=  flash_ram_buffer[EEPROM_INDEX_DEEP_DCH_CYCLE+1];
	    
	    
	g_sys_cap.val.deep_chg_cycle_cnt =  flash_ram_buffer[EEPROM_INDEX_DEEP_CHG_CYCLE];
	g_sys_cap.val.deep_chg_cycle_cnt <<=8;
	g_sys_cap.val.deep_chg_cycle_cnt |= flash_ram_buffer[EEPROM_INDEX_DEEP_CHG_CYCLE+1];
	    
	//0819
	sys_err_flags.VAL = flash_ram_buffer[EEPROM_INDEX_SYS_ERR];
	sys_err_flags.VAL <<=8;
	sys_err_flags.VAL = flash_ram_buffer[EEPROM_INDEX_SYS_ERR+1];

	g_sys_history.val.vcell_min = flash_ram_buffer[EEPROM_INDEX_HIS_VCELLMIN];
	g_sys_history.val.vcell_min <<=8;
	g_sys_history.val.vcell_min |=  flash_ram_buffer[EEPROM_INDEX_HIS_VCELLMIN+1];
	    
	g_sys_history.val.vcell_max = flash_ram_buffer[EEPROM_INDEX_HIS_VCELLMAX];
	g_sys_history.val.vcell_max <<=8;
	g_sys_history.val.vcell_max |=  flash_ram_buffer[EEPROM_INDEX_HIS_VCELLMAX+1];

	g_sys_history.val.bat_temp_min = flash_ram_buffer[EEPROM_INDEX_HIS_TEMPMIN];
	g_sys_history.val.bat_temp_max =  flash_ram_buffer[EEPROM_INDEX_HIS_TEMPMAX];

	g_sys_history.val.dch_cur_max = flash_ram_buffer[EEPROM_INDEX_HIS_DCHCMAX];
	g_sys_history.val.dch_cur_max <<=8;
	g_sys_history.val.dch_cur_max |=  flash_ram_buffer[EEPROM_INDEX_HIS_DCHCMAX+1];
	    
	g_sys_history.val.chg_cur_max = flash_ram_buffer[EEPROM_INDEX_HIS_CHGCMAX];
	g_sys_history.val.chg_cur_max <<=8;
	g_sys_history.val.chg_cur_max |=  flash_ram_buffer[EEPROM_INDEX_HIS_CHGCMAX+1];

	g_sys_history.val.soc_max = flash_ram_buffer[EEPROM_INDEX_HIS_SOC_MAX];
	g_sys_history.val.soc_max <<=8;
	g_sys_history.val.soc_max |=  flash_ram_buffer[EEPROM_INDEX_HIS_SOC_MAX+1];
	    
	g_sys_history.val.pcb_temp_max = flash_ram_buffer[EEPROM_INDEX_HIS_PCBTEMPMAX];
}

/**
  * @brief  主EEPROM读取失败，读取备用块
  * @param  None
  * @retval None
  */

void EEPROM_BACKUP_READ(void)
{
   uint8_t i;
   uint8_t block_crc;
   Bsp_Read_Buffer(EEPROM_SYS_BK_BEGIN,flash_ram_buffer,EEPROM_SYS_DATA_LEN);
   if(flash_ram_buffer[0] == 0xB3)
   {
	   block_crc =0;
	   for(i=0;i<EEPROM_INDEX_CRC;i++)
	   {
		   block_crc += flash_ram_buffer[i];
	   }
	   if(block_crc == flash_ram_buffer[EEPROM_INDEX_CRC])
	   {
		   EEPROM_To_RAM();
		   //iap_vision =  DATAEE_BK_ReadByte(EEPROM_INDEX_VISION); //DATAEE_ReadByte
		   //iap_vision <<=8;
		   //iap_vision |=  DATAEE_BK_ReadByte(EEPROM_INDEX_VISION+1); //DATAEE_ReadByte
	   }
	   else
	   {
		   EEPROM_Init();
	   }
   }
   else
   {
	   EEPROM_Init();
   }
}

/**
  * @brief  主EEPROM读取失败，读取备用块
  * @param  None
  * @retval None
  */

void EEPROM_Init(void)
{
    uint8_t i;
    uint8_t block_crc;
    block_crc =0;
    flash_ram_buffer[EEPROM_INDEX_BEGIN] = 0xB3;
    flash_ram_buffer[EEPROM_INDEX_FULL_CAP] = (uint8_t)(cap_update>>8);
    flash_ram_buffer[EEPROM_INDEX_FULL_CAP+1] = (uint8_t)(cap_update);
    
    flash_ram_buffer[EEPROM_INDEX_CYCLE] = (uint8_t)(g_sys_cap.val.bat_cycle_cnt>>8);
    flash_ram_buffer[EEPROM_INDEX_CYCLE+1] = (uint8_t)(g_sys_cap.val.bat_cycle_cnt);
    
    flash_ram_buffer[EEPROM_INDEX_DEEP_DCH_CYCLE] = (uint8_t)(g_sys_cap.val.deep_dch_cycle_cnt>>8);
    flash_ram_buffer[EEPROM_INDEX_DEEP_DCH_CYCLE+1] = (uint8_t)(g_sys_cap.val.deep_dch_cycle_cnt);
    
    flash_ram_buffer[EEPROM_INDEX_DEEP_CHG_CYCLE] = (uint8_t)(g_sys_cap.val.deep_chg_cycle_cnt>>8);
    flash_ram_buffer[EEPROM_INDEX_DEEP_CHG_CYCLE+1] = (uint8_t)(g_sys_cap.val.deep_chg_cycle_cnt);
    
    //0819
    flash_ram_buffer[EEPROM_INDEX_SYS_ERR] = (uint8_t)(sys_err_flags.VAL>>8);
    flash_ram_buffer[EEPROM_INDEX_SYS_ERR+1] = (uint8_t)(sys_err_flags.VAL);
    //20160810新增历史信息
    flash_ram_buffer[EEPROM_INDEX_HIS_VCELLMIN] = (uint8_t)(g_sys_history.val.vcell_min>>8);
    flash_ram_buffer[EEPROM_INDEX_HIS_VCELLMIN+1] = (uint8_t)(g_sys_history.val.vcell_min);
    flash_ram_buffer[EEPROM_INDEX_HIS_VCELLMAX] = (uint8_t)(g_sys_history.val.vcell_max>>8);
    flash_ram_buffer[EEPROM_INDEX_HIS_VCELLMAX+1] = (uint8_t)(g_sys_history.val.vcell_max);

    flash_ram_buffer[EEPROM_INDEX_HIS_TEMPMIN] = (uint8_t)(g_sys_history.val.bat_temp_min);
    flash_ram_buffer[EEPROM_INDEX_HIS_TEMPMAX] = (uint8_t)(g_sys_history.val.bat_temp_max);
    
    flash_ram_buffer[EEPROM_INDEX_HIS_DCHCMAX] = (uint8_t)(g_sys_history.val.dch_cur_max>>8);
    flash_ram_buffer[EEPROM_INDEX_HIS_DCHCMAX+1] = (uint8_t)(g_sys_history.val.dch_cur_max);
    flash_ram_buffer[EEPROM_INDEX_HIS_CHGCMAX] = (uint8_t)(g_sys_history.val.chg_cur_max>>8);
    flash_ram_buffer[EEPROM_INDEX_HIS_CHGCMAX+1] = (uint8_t)(g_sys_history.val.chg_cur_max);
    
    flash_ram_buffer[EEPROM_INDEX_HIS_SOC_MAX] = (uint8_t)(g_sys_history.val.soc_max>>8);
    flash_ram_buffer[EEPROM_INDEX_HIS_SOC_MAX+1] = (uint8_t)(g_sys_history.val.soc_max);
    flash_ram_buffer[EEPROM_INDEX_HIS_PCBTEMPMAX] = (uint8_t)(g_sys_history.val.pcb_temp_max>>8);
    
    for(i=0;i<EEPROM_INDEX_CRC;i++)
    {
	    block_crc += flash_ram_buffer[i];
    }
    flash_ram_buffer[EEPROM_INDEX_CRC] = block_crc;
	
	Bsp_Erase_Row(EEPROM_SYS_BEGIN);
	Bsp_Erase_Row(EEPROM_SYS_BK_BEGIN);
	
	Bsp_Write_Buffer(EEPROM_SYS_BEGIN,flash_ram_buffer,EEPROM_SYS_DATA_LEN);
	Bsp_Write_Buffer(EEPROM_SYS_BK_BEGIN,flash_ram_buffer,EEPROM_SYS_DATA_LEN);

}