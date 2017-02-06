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


