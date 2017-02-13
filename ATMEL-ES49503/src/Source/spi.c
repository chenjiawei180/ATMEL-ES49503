/*
 * spi.c
 *
 * Created: 2017/2/6 10:36:43
 *  Author: chenjiawei
 */ 

#include "spi.h"

struct spi_module spi_master_instance;
struct spi_slave_inst slave;

static uint8_t	ucSPI_SendData[5];						/* send data 5byte	*/
static uint8_t	ucSPI_RecvData[3];						/* receive data 3byte	*/
uint8_t  afe_lost_cnt =0;

static void Configure_Extint_ADIRQ2(void);
static void Configure_Extint_Callbacks_ADIRQ2(void);

void ADIRQ2_Extint_Callback(void)
{
	sys_flags.val.afe_adirq2_flag =1;
}

const uint8_t ucCRC_tCalc[] ={
	0x00,0xd5,0x7f,0xaa,0xfe,0x2b,0x81,0x54,0x29,0xfc,0x56,0x83,0xd7,0x02,0xa8,0x7d,
	0x52,0x87,0x2d,0xf8,0xac,0x79,0xd3,0x06,0x7b,0xae,0x04,0xd1,0x85,0x50,0xfa,0x2f,
	0xa4,0x71,0xdb,0x0e,0x5a,0x8f,0x25,0xf0,0x8d,0x58,0xf2,0x27,0x73,0xa6,0x0c,0xd9,
	0xf6,0x23,0x89,0x5c,0x08,0xdd,0x77,0xa2,0xdf,0x0a,0xa0,0x75,0x21,0xf4,0x5e,0x8b,
	0x9d,0x48,0xe2,0x37,0x63,0xb6,0x1c,0xc9,0xb4,0x61,0xcb,0x1e,0x4a,0x9f,0x35,0xe0,
	0xcf,0x1a,0xb0,0x65,0x31,0xe4,0x4e,0x9b,0xe6,0x33,0x99,0x4c,0x18,0xcd,0x67,0xb2,
	0x39,0xec,0x46,0x93,0xc7,0x12,0xb8,0x6d,0x10,0xc5,0x6f,0xba,0xee,0x3b,0x91,0x44,
	0x6b,0xbe,0x14,0xc1,0x95,0x40,0xea,0x3f,0x42,0x97,0x3d,0xe8,0xbc,0x69,0xc3,0x16,
	0xef,0x3a,0x90,0x45,0x11,0xc4,0x6e,0xbb,0xc6,0x13,0xb9,0x6c,0x38,0xed,0x47,0x92,
	0xbd,0x68,0xc2,0x17,0x43,0x96,0x3c,0xe9,0x94,0x41,0xeb,0x3e,0x6a,0xbf,0x15,0xc0,
	0x4b,0x9e,0x34,0xe1,0xb5,0x60,0xca,0x1f,0x62,0xb7,0x1d,0xc8,0x9c,0x49,0xe3,0x36,
	0x19,0xcc,0x66,0xb3,0xe7,0x32,0x98,0x4d,0x30,0xe5,0x4f,0x9a,0xce,0x1b,0xb1,0x64,
	0x72,0xa7,0x0d,0xd8,0x8c,0x59,0xf3,0x26,0x5b,0x8e,0x24,0xf1,0xa5,0x70,0xda,0x0f,
	0x20,0xf5,0x5f,0x8a,0xde,0x0b,0xa1,0x74,0x09,0xdc,0x76,0xa3,0xf7,0x22,0x88,0x5d,
	0xd6,0x03,0xa9,0x7c,0x28,0xfd,0x57,0x82,0xff,0x2a,0x80,0x55,0x01,0xd4,0x7e,0xab,
	0x84,0x51,0xfb,0x2e,0x7a,0xaf,0x05,0xd0,0xad,0x78,0xd2,0x07,0x53,0x86,0x2c,0xf9,
};

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
	
	struct port_config pin_conf;
	port_get_config_defaults(&pin_conf);
	pin_conf.input_pull = PORT_PIN_PULL_NONE;
	port_pin_set_config(SDI_PIN, &pin_conf);
	
	pin_conf.direction  = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(SHDN, &pin_conf);
	port_pin_set_output_level(SHDN, true);

	pin_conf.direction  = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(STB, &pin_conf);
	port_pin_set_output_level(STB, true);
	
	Configure_Extint_ADIRQ2();
	Configure_Extint_Callbacks_ADIRQ2();
	
}

void Configure_Extint_ADIRQ2(void)
{
	struct extint_chan_conf config_extint_chan;
	extint_chan_get_config_defaults(&config_extint_chan);
	config_extint_chan.gpio_pin           = ADIRQ2_EIC_PIN;
	config_extint_chan.gpio_pin_mux       = ADIRQ2_EIC_MUX;
	config_extint_chan.gpio_pin_pull      = ADIRQ2_EIC_PULL_UP;
	config_extint_chan.detection_criteria = ADIRQ2_EIC_DETECT;
	extint_chan_set_config(ADIRQ2_EIC_LINE, &config_extint_chan);
}

void Configure_Extint_Callbacks_ADIRQ2(void)
{
	extint_register_callback(ADIRQ2_Extint_Callback,	ADIRQ2_EIC_LINE,	EXTINT_CALLBACK_TYPE_DETECT);
	extint_chan_enable_callback(ADIRQ2_EIC_LINE,	EXTINT_CALLBACK_TYPE_DETECT);
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

/**
  * @brief  This function is vSPI_Wait. 
  * @param  None
  * @retval None
  */

void vSPI_Wait(void)
{
	volatile uint8_t ucdummy;
	for (ucdummy = 0; ucdummy < 8; ucdummy++);
}

/**
  * @brief  计算AFE通信的CRC校验值. 
  * @param  None
  * @retval None
  */

uint8_t ucCRC_Calc( uint8_t ucsize,uint8_t* pucdt )
{
	uint8_t	uccrc,uci;
	uccrc = 0;

	for( uci=0; uci<ucsize; uci++) 
	{
		uccrc = ucCRC_tCalc[uccrc ^ pucdt[uci]];
	}

	return uccrc;
}


/**
  * @brief  AFE SPI写入
  * @param  ucdev设备地址,ucreg设备寄存器地址,undata需要写入的数据
  * @retval ok/err
  */

uint8_t ucSPI_Write(uint8_t ucdev, uint8_t ucreg, uint16_t undata)
{
	uint8_t	ucerrorcount = 0;
	uint16_t	wk_reg;
	uint16_t	wk_dev;
	uint16_t	wk_dat;

	wk_dev	= ((uint16_t)ucdev << 1) & 0x001E;
	wk_reg	= ((uint16_t)ucreg << 1) & 0x00FE;
	wk_dat	= undata;
	uint16_t i;
	uint8_t SDI_VAL;
	while (ucerrorcount <= MAC_SPI_WRITE_RETRY) 
	{
		SPI_Slave_High();

		ucSPI_SendData[0] = (uint8_t)0xE0 | (uint8_t)wk_dev | MAC_SPI_RW_WRITE;	/* set send data	*/
		ucSPI_SendData[1] = (uint8_t)(wk_reg | MAC_SPI_TRANS_SINGLE);
		ucSPI_SendData[2] = (uint8_t)(wk_dat >> 8);
		ucSPI_SendData[3] = (uint8_t)(wk_dat & 0xFF);
		ucSPI_SendData[4] = ucCRC_Calc(4,&ucSPI_SendData[0]);
		
		SPI_Write_Buff(ucSPI_SendData,5);
		
		for(i=0;i<10000;i++)
		{
			SDI_VAL = SDI1_GetValue();
			if(SDI1_GetValue() ==1)
			{
				break;
			}
		}
		delay_us(100);
		if(SDI_VAL ==1)
		{
			SPI_Slave_Low();
			return 0;
		}
		delay_us(100);
		
		SPI_Slave_Low();
		vSPI_Wait();						/* Wait so as not to High SEM immediately */
		ucerrorcount++;
	}

	return 1;								/* error */
}

/**
  * @brief  AFE SPI读取
  * @param  ucdev设备地址,ucreg设备寄存器地址,pundata 存放数组
  * @retval ok/err
  */

uint8_t ucSPI_Read(uint8_t ucdev, uint8_t ucreg, uint16_t* pundata)
{
	uint8_t	ucerrorcount = 0;
	uint8_t	crc;
	uint16_t	wk_reg;
	uint16_t	wk_dev;
	uint16_t i;
	uint8_t SDI_VAL;
	wk_dev	= ((uint16_t)ucdev << 1) & 0x001E;
	wk_reg	= ((uint16_t)ucreg << 1) & 0x00FE;

	while (ucerrorcount <= MAC_SPI_READ_RETRY) 
	{
		SPI_Slave_High();

		ucSPI_SendData[0] = 0xE0 | (uint8_t)wk_dev | MAC_SPI_RW_READ;
		ucSPI_SendData[1] = (uint8_t)wk_reg | MAC_SPI_TRANS_SINGLE;
		ucSPI_SendData[2] = ucCRC_Calc(2,&ucSPI_SendData[0]);
	
		SPI_Write_Buff(ucSPI_SendData,3);

		for(i=0;i<10000;i++)
		{
			SDI_VAL = SDI1_GetValue();
			if(SDI1_GetValue() ==1)
			{
				break;
			}
		}
		delay_us(100);
		if(SDI_VAL ==1)
		{
			SPI_Read_Buff(ucSPI_RecvData,3);

			SPI_Slave_Low();
			crc = ucCRC_Calc(2,&ucSPI_RecvData[0]);
			if( crc != ucSPI_RecvData[2] ){
				return 2;
			}
			pundata[0] = ((uint16_t)ucSPI_RecvData[0] << 8 ) + ucSPI_RecvData[1];	/* Bp15-8 => left 8bit shift + Bp7-0 */
			return 0;						/* Successful complete */
		}
		SPI_Slave_Low();
		vSPI_Wait();						/* Wait so as not to High SEM immediately */
		ucerrorcount++;
	}
	return 1;								/* error */
}

/**
  * @brief  AFE SPI读取
  * @param  ucdev设备地址,ucreg设备寄存器地址,uctime 连续长度
  * @retval ok/err
  */

uint8_t ucSPI_Continue_Read(uint8_t ucdev, uint8_t ucreg, uint8_t uctime)
{
	uint8_t	ucerrorcount = 0;
	uint16_t	wk_reg;
	uint16_t	wk_dev;
	uint8_t	wk_time;
	uint16_t i;
	uint8_t SDI_VAL;
	wk_dev	= ((uint16_t)ucdev << 1) & 0x001E;
	wk_reg	= ((uint16_t)ucreg << 1) & 0x00FE;
	wk_time	= uctime;

	while (ucerrorcount <= MAC_SPI_READ_RETRY) 
	{
		SPI_Slave_High();

		ucSPI_SendData[0] = 0xE0 | (uint8_t)wk_dev | MAC_SPI_RW_READ;
		ucSPI_SendData[1] = (uint8_t)wk_reg | MAC_SPI_TRANS_CONTINUE;
		ucSPI_SendData[2] = wk_time & 0x7F;
		ucSPI_SendData[3] = ucCRC_Calc(3,&ucSPI_SendData[0]);

		SPI_Write_Buff(ucSPI_SendData,4);
		
		for(i=0;i<10000;i++)
		{
			SDI_VAL = SDI1_GetValue();
			if(SDI1_GetValue() ==1)
			{
				break;
			}
		}
		delay_us(100);
		if(SDI_VAL ==1)
		{
			SPI_Read_Buff( ucSPI_RecvData,( (MAC_AN49503_READ_CNT*2) + 1 ) );
			
			SPI_Slave_Low();
			vSPI_Wait();						/* Wait so as not to High SEM immediately */
			if(ucSPI_Conti_RecvData[5] == 0x3b)
			{
				afe_lost_cnt =0;
				sys_flags.val.afe_connect_flag = 1;
			}
			else
			{
				afe_lost_cnt++;
				if(afe_lost_cnt ==8)
				{
					afe_lost_cnt =0;
					sys_flags.val.afe_connect_flag = 0;
				}
			}
			return 0;						/* Successful complete		*/
		}
		SPI_Slave_Low();
		ucerrorcount++;
	}

	return 1;								/* error */
}