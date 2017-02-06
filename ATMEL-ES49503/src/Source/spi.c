/*
 * spi.c
 *
 * Created: 2017/2/6 10:36:43
 *  Author: chenjiawei
 */ 

#include "spi.h"

struct spi_module spi_master_instance;
struct spi_slave_inst slave;

/**
  * @brief  This function is Initialization SPI. 
  * @param  None
  * @retval None
  */

void Configure_Spi_Master(void)
{
	struct spi_config config_spi_master;
	struct spi_slave_inst_config slave_dev_config;
	
	spi_slave_inst_get_config_defaults(&slave_dev_config);
	slave_dev_config.ss_pin = CONF_MASTER_SS_PIN;
	spi_attach_slave(&slave, &slave_dev_config);
	
	spi_get_config_defaults(&config_spi_master);
	config_spi_master.mux_setting = CONF_MASTER_MUX_SETTING;
	config_spi_master.pinmux_pad0 = CONF_MASTER_PINMUX_PAD0;
	config_spi_master.pinmux_pad1 = CONF_MASTER_PINMUX_PAD1;
	config_spi_master.pinmux_pad2 = CONF_MASTER_PINMUX_PAD2;
	config_spi_master.pinmux_pad3 = CONF_MASTER_PINMUX_PAD3;
	
	spi_init(&spi_master_instance, CONF_MASTER_SPI_MODULE, &config_spi_master);
	spi_enable(&spi_master_instance);
	SPI_Slave_Low();
}

/**
  * @brief  This function is SPI write buffer. 
  * @param  uint8_t *buff,uint16_t length
  * @retval None
  */

void SPI_Write_Buff(uint8_t *buff,uint16_t length)
{
	spi_write_buffer_wait(&spi_master_instance, buff, length);
}

/**
  * @brief  This function is SPI read buffer. 
  * @param  uint8_t *buff,uint16_t length
  * @retval None
  */

void SPI_Read_Buff(uint8_t *buff,uint16_t length)
{
	spi_read_buffer_wait(&spi_master_instance, buff, length,0x00);
}

/**
  * @brief  This function is SPI transceive buffer. 
  * @param  uint8_t *tx_buff,uint8_t *rx_buff,uint16_t length
  * @retval None
  */

void SPI_Transceive_Buff(uint8_t *tx_buff,uint8_t *rx_buff,uint16_t length)
{
	spi_transceive_buffer_wait(&spi_master_instance,tx_buff,rx_buff,length);
}

/**
  * @brief  This function is Set SPI_SS Low. 
  * @param  None
  * @retval None
  */

void SPI_Slave_Low(void)
{
	spi_select_slave(&spi_master_instance, &slave, true);
}

/**
  * @brief  This function is Set SPI_SS High. 
  * @param  None
  * @retval None
  */

void SPI_Slave_High(void)
{
	spi_select_slave(&spi_master_instance, &slave, false);
}

