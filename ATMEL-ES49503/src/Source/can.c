/*
 * can.c
 *
 * Created: 2017/2/27 9:20:48
 *  Author: chenjiawei
 */ 

#include "can.h"
#include "gpio.h"
#include "simulation.h"

static struct can_module can_instance;

//! [can_transfer_message_setting]
#define CAN_TX_BUFFER_INDEX    0
//static uint8_t tx_message_0[CONF_CAN_ELEMENT_DATA_SIZE];
//static uint8_t tx_message_1[CONF_CAN_ELEMENT_DATA_SIZE];
//! [can_transfer_message_setting]

//! [can_receive_message_setting]
static volatile uint32_t standard_receive_index = 0;
//static volatile uint32_t extended_receive_index = 0;
static struct can_rx_element_fifo_0 rx_element_fifo_0;
//static struct can_rx_element_fifo_1 rx_element_fifo_1;
//static struct can_rx_element_buffer rx_element_buffer;
//! [can_receive_message_setting]

static uint8_t callback_buffer[XMODEM_BUFLEN];
volatile static uint32_t readOffset, writeOffset;

uint8_t battery_data[64] = {0};
uint8_t profile_data[128] = {0};
	
uint8_t address_assign_flag = 1;
uint8_t address_conflict = 0;

void configure_can(void)
{
	/* Set up the CAN TX/RX pins */
	struct system_pinmux_config pin_config;
	system_pinmux_get_config_defaults(&pin_config);
	pin_config.mux_position = CAN_TX_MUX_SETTING;
	system_pinmux_pin_set_config(CAN_TX_PIN, &pin_config);
	pin_config.mux_position = CAN_RX_MUX_SETTING;
	system_pinmux_pin_set_config(CAN_RX_PIN, &pin_config);

	/* Initialize the module. */
	struct can_config config_can;
	can_get_config_defaults(&config_can);
	can_init(&can_instance, CAN_MODULE, &config_can);

	can_start(&can_instance);

	/* Enable interrupts for this CAN module */
	system_interrupt_enable(SYSTEM_INTERRUPT_MODULE_CAN0);
	can_enable_interrupt(&can_instance, CAN_PROTOCOL_ERROR_ARBITRATION
	| CAN_PROTOCOL_ERROR_DATA);
}

void can_set_standard_filter_1(void)
{
	struct can_standard_message_filter_element sd_filter;

	can_get_standard_message_filter_element_default(&sd_filter);
	sd_filter.S0.bit.SFID1 = CAN_RX_STANDARD_FILTER_ID_1;

	can_set_rx_standard_filter(&can_instance, &sd_filter,
	CAN_RX_STANDARD_FILTER_INDEX_1);
	can_enable_interrupt(&can_instance, CAN_RX_FIFO_0_NEW_MESSAGE);
}

void can_send_standard_message(uint32_t id_value, uint8_t *data,uint32_t data_length)
{
	uint32_t i;
	struct can_tx_element tx_element;

	can_get_tx_buffer_element_defaults(&tx_element);
	tx_element.T0.reg |= CAN_TX_ELEMENT_T0_STANDARD_ID(id_value);
	tx_element.T1.bit.DLC = data_length;
	for (i = 0; i < data_length; i++) {
		tx_element.data[i] = *data;
		data++;
	}

	can_set_tx_buffer_element(&can_instance, &tx_element,
	CAN_TX_BUFFER_INDEX);
	can_tx_transfer_request(&can_instance, 1 << CAN_TX_BUFFER_INDEX);
}

void CAN0_Handler(void)
{
	volatile uint32_t status, i;
	status = can_read_interrupt_status(&can_instance);

	//if (status & CAN_RX_BUFFER_NEW_MESSAGE) {
	//can_clear_interrupt_status(&can_instance, CAN_RX_BUFFER_NEW_MESSAGE);
	//for (i = 0; i < CONF_CAN0_RX_BUFFER_NUM; i++) {
	//if (can_rx_get_buffer_status(&can_instance, i)) {
	//rx_buffer_index = i;
	//can_rx_clear_buffer_status(&can_instance, i);
	//can_get_rx_buffer_element(&can_instance, &rx_element_buffer,
	//rx_buffer_index);
	//if (rx_element_buffer.R0.bit.XTD) {
	//printf("\n\r Extended message received in Rx buffer. The received data is: \r\n");
	//} else {
	//printf("\n\r Standard message received in Rx buffer. The received data is: \r\n");
	//}
	//for (i = 0; i < rx_element_buffer.R1.bit.DLC; i++) {
	//printf("  %d",rx_element_buffer.data[i]);
	//}
	//printf("\r\n\r\n");
	//}
	//}
	//}

	if (status & CAN_RX_FIFO_0_NEW_MESSAGE) {
		can_clear_interrupt_status(&can_instance, CAN_RX_FIFO_0_NEW_MESSAGE);
		can_get_rx_fifo_0_element(&can_instance, &rx_element_fifo_0,
		standard_receive_index);
		can_rx_fifo_acknowledge(&can_instance, 0,
		standard_receive_index);
		standard_receive_index++;
		if (standard_receive_index == CONF_CAN0_RX_FIFO_0_NUM) {
			standard_receive_index = 0;
		}
		//download_address = ( rx_element_fifo_0.data[0] << 24 ) | ( rx_element_fifo_0.data[1] << 16 ) | ( rx_element_fifo_0.data[2] << 8 ) | ( rx_element_fifo_0.data[3] );
		//program_memory(download_address+APP_START_ADDRESS, &rx_element_fifo_0.data[4], 4);
		//can_send_standard_message(CAN_RX_STANDARD_FILTER_ID_1,&rx_element_fifo_0.data[0],4);
		//write_buffer[0] = rx_element_fifo_0.data[0];
		//write_buffer[1] = rx_element_fifo_0.data[1];
		//write_buffer[2] = rx_element_fifo_0.data[2];
		//write_buffer[3] = rx_element_fifo_0.data[3];
		
		
		//printf("\n\r Standard message received in FIFO 0. The received data is: \r\n");
		for (i = 0; i < rx_element_fifo_0.R1.bit.DLC; i++) {
			write_byte(rx_element_fifo_0.data[i]);
			//printf("  %d",rx_element_fifo_0.data[i]);
		}
		//can_send_standard_message(CAN_RX_STANDARD_FILTER_ID_1,&rx_element_fifo_0.data[0],8);
		//printf("\r\n\r\n");
	}

	//if (status & CAN_RX_FIFO_1_NEW_MESSAGE) {
	//can_clear_interrupt_status(&can_instance, CAN_RX_FIFO_1_NEW_MESSAGE);
	//can_get_rx_fifo_1_element(&can_instance, &rx_element_fifo_1,
	//extended_receive_index);
	//can_rx_fifo_acknowledge(&can_instance, 0,
	//extended_receive_index);
	//extended_receive_index++;
	//if (extended_receive_index == CONF_CAN0_RX_FIFO_1_NUM) {
	//extended_receive_index = 0;
	//}
	//download_address = ( rx_element_fifo_1.data[0] << 24 ) | ( rx_element_fifo_1.data[1] << 16 ) | ( rx_element_fifo_1.data[2] << 8 ) | ( rx_element_fifo_1.data[3] );
	//program_memory(download_address+APP_START_ADDRESS, &rx_element_fifo_1.data[4], 4);
	//can_send_extended_message(CAN_RX_EXTENDED_FILTER_ID_1,&rx_element_fifo_1.data[0],4);
	////printf("\n\r Extended message received in FIFO 1. The received data is: \r\n");
	////for (i = 0; i < rx_element_fifo_1.R1.bit.DLC; i++) {
	////printf("  %d",rx_element_fifo_1.data[i]);
	////}
	////printf("\r\n\r\n");
	//}

	if ((status & CAN_PROTOCOL_ERROR_ARBITRATION)
	|| (status & CAN_PROTOCOL_ERROR_DATA)) {
		can_clear_interrupt_status(&can_instance, CAN_PROTOCOL_ERROR_ARBITRATION
		| CAN_PROTOCOL_ERROR_DATA);
		port_pin_toggle_output_level(PIN_PA27);
		//printf("Protocol error, please double check the clock in two boards. \r\n\r\n");
	}
}

void buff_init(void)
{
	readOffset = 0;
	writeOffset = 0;
}

void write_byte(uint8_t byte)
{
	callback_buffer[writeOffset++] = byte;
	writeOffset &= XMODEM_BUFLEN - 1;
}

void read_bytes(uint8_t * buffer, uint32_t byteCount)
{
	uint32_t currentBytesRead = 0;
	uint32_t i = 0;

	while(currentBytesRead != byteCount)
	{
		//wdt_reset_count();
		i++;
		if (i == 10000)
		{
			break;
		}
		
		if (readOffset != writeOffset)
		{
			buffer[currentBytesRead++] = callback_buffer[readOffset++];
			readOffset &= XMODEM_BUFLEN - 1;
		}
	}
}

//int read_packet(uint8_t *buffer, uint8_t idx)
//{
	//uint8_t checksum = 0;
	//uint32_t download_address = 0;
	//uint32_t data_length = 0;
	//uint8_t Send_buffer[16] = {0};
	//uint16_t crc16 = 0;
	//uint16_t crc_cal = 0;
//
	//read_bytes(buffer+1,idx);    // 读取除数据长度以为的数据
	////添加校验和算法
	//checksum = check_sum(buffer,idx+1); // idx+1包含数据长度
	//if ( checksum == (buffer[idx]) )
	//{
		//switch(buffer[1])
		//{
			////case 0x01:
			////system_reset();
			////break;
			////case 0x03:
			////download_address = ( buffer[2] << 24 ) | ( buffer[3] << 16 ) | ( buffer[4] << 8 ) | ( buffer[5] );
			////program_memory(download_address+APP_START_ADDRESS, &buffer[6], buffer[0]-4);
			////Send_buffer[0] = 0x55;
			////Send_buffer[1] = 0x00;
			////Send_buffer[2] = 0x00;
			////Send_buffer[3] = 0x04;
			////Send_buffer[4] = 0x04;
			////Send_buffer[5] = buffer[2];
			////Send_buffer[6] = buffer[3];
			////Send_buffer[7] = buffer[4];
			////Send_buffer[8] = buffer[5];
			////Send_buffer[9] = check_sum(Send_buffer+3,7);
			////send_message(Send_buffer,10);
			////break;
			////case 0x05:
			////data_length = ( buffer[2] << 24 ) | ( buffer[3] << 16 ) | ( buffer[4] << 8 ) | ( buffer[5] );
			////crc_cal = Flash_CRC(data_length);
			////crc16 = buffer[6] << 8 | buffer[7] ;
			////Send_buffer[0] = 0x55;
			////Send_buffer[1] = 0x00;
			////Send_buffer[2] = 0x00;
			////Send_buffer[3] = 0x01;
			////Send_buffer[4] = 0x06;
			////
			////if ( crc16 == crc_cal )
			////{
				////Send_buffer[5] = 0;
			////}
			////else
			////{
				////Send_buffer[5] = 1;
			////}
			////Send_buffer[6] = check_sum(Send_buffer+3,4);
			////send_message(Send_buffer,7);
			////Send_buffer[0] = 0x00;
			////Send_buffer[1] = 0x00;
			////nvm_write_buffer(BOOTLOADER_FLAG,Send_buffer,2);
			////system_reset();
			////break;
			////case 0x07:
			////break;
			////default:
			////break;
		//}
	//}
//}

uint8_t check_sum(uint8_t *buffer, uint8_t idx)
{
	uint8_t i = 0;
	uint8_t checksum = 0;
	for (i = 0; i < idx-1;i++)
	{
		checksum += buffer[i];
	}
	return 0x100-checksum;
}

void send_message(uint8_t * buffer, uint8_t idx)
{
	uint8_t i;
	uint8_t length = idx;
	for (i=0;i<((idx-1)>>3)+1;i++)
	{
		if (length > 8)
		{
			can_send_standard_message(CAN_RX_STANDARD_FILTER_ID_1,buffer+(i<<3),8);
			length = length - 8;
		}
		else
		{
			can_send_standard_message(CAN_RX_STANDARD_FILTER_ID_1,buffer+(i<<3),length);
		}
		//delay_us(250);
		delay_us(500);
	}
}


void can_process(void)
{
	uint8_t buffer[XMODEM_BUFLEN];
	uint8_t ch;
	uint8_t checksum = 0;
	uint8_t broadcast = 0;
	if (readOffset != writeOffset)
	{
		broadcast = 0;
		CanTxBuffer[0] = 0x55;
		read_bytes(&ch,1);
		if (ch == 0x55)  //找到第一个字节 0x55
		{
			read_bytes(&ch,1);
			if (ch == 0x00)  //第二个字节 0x00
			{
				read_bytes(&ch,1);
				if (ch == 0x00)  //第三个字节 0x00
				{
					read_bytes(&ch,1);   //第四个字节 数据长
					if(ch == 0x00)
					{
						read_bytes(&ch,1); 
						if(ch == 0x01)
						{
							read_bytes(&ch,1); 
							if(ch == 0xff)
							{
								nvm_erase_row(BOOTLOADER_FLAG);
								system_reset();
							}
						}
					}
				}
			}
			else if(ch == ID_address || ch == 0xff )  //地址
			{
				if (ch == 0xff)
				{
					broadcast = 1;
				}
				read_bytes(&Sequence_ID,1);   //取出定序ID
				read_bytes(buffer,1);   //第四个字节 数据长

				if(buffer[0] <= 9)
				{
					read_bytes(buffer+1,buffer[0]+2);
					checksum = check_sum(buffer,buffer[0]+3);
					if ( checksum == (buffer[buffer[0]+2]) )
					{
						switch(buffer[1])
						{
							case 0xCE:
								if (address_assign_flag == 0)
								{
									if (buffer[2] != 0x00 && buffer[2] != 0xff )
									{
										Latch_id = buffer[2];
										battery_load();
										if ( broadcast == 0)
										{
											latch_answer();
										}
									}
								}
								break;
							case 0xC5:
								if (address_assign_flag == 0 && broadcast == 0)
								{
									if (Latch_id != buffer[2])
									{
										Latch_id = 0xff;
										battery_load();
									}
									battery_answer();
								}
								break;
							case 0xC6:
								if (address_assign_flag == 0 && broadcast == 0)
								{
									profile_answer();
								}
								break;
							case 0x48:
								if (address_assign_flag == 0 && broadcast == 0)
								{
									address_answer();
								}
								break;
							case 0x58:
								if ( buffer[2] == ID_address && address_assign_flag == 1)
								{
									address_conflict = 1;
								}
								break;
#ifdef SIMULATION_AFE 
							case 0x38:
								Can_var(buffer);
								break;
#endif							
							default:
								break;
						}	
					}
				}
			}
		}
	}
}

void address_answer(void)
{
	uint8_t Send_buffer[10];
	Send_buffer[0] = 0x55;
	Send_buffer[1] = ID_address;
	Send_buffer[2] = Sequence_ID;
	Send_buffer[3] = 0x01;
	Send_buffer[4] = 0x58;
	Send_buffer[5] = ID_address;    // 数据开始
	Send_buffer[6] = check_sum(Send_buffer+3,4);
	send_message(Send_buffer,7);
}

void profile_answer(void)
{
	profile_data[0] = 0x55;
	profile_data[1] = ID_address;
	profile_data[2] = Sequence_ID;
	profile_data[3] = 90;
	profile_data[4] = 0xd6;
	profile_data[5] = 13;    //数据开始
	profile_data[6] = 1;
	profile_data[7] = 6300&0Xff;    //额定容量
	profile_data[8] = 6300>>8;
	profile_data[9] = 4800&0xff;    // 公称电压
	profile_data[10] = 4800>>8;
	profile_data[11] = 45;    //充电最高温度
	profile_data[12] = -10;    //充电最低温度
	
	profile_data[13] = 0;    //制造公司名
	profile_data[14] = 0;
	profile_data[15] = 0;
	profile_data[16] = 0;
	profile_data[17] = 0;
	profile_data[18] = 0;
	profile_data[19] = 0;
	profile_data[20] = 0;
	
	profile_data[21] = 0;    //模组名
	profile_data[22] = 0;
	profile_data[23] = 0;
	profile_data[24] = 0;
	profile_data[25] = 0;
	profile_data[26] = 0;
	profile_data[27] = 0;
	profile_data[28] = 0;	
	
	profile_data[29] = 0;    //制造日
	profile_data[30] = 0;
	
	profile_data[31] = 0;    //制造S/N
	profile_data[32] = 0;
	
	profile_data[33] = 0;    //通信协议
	profile_data[34] = 0;    //F/W version
	profile_data[35] = 0;    //数据 version
	profile_data[36] = 0;    //保护板version
	
	profile_data[37] = 0;    //满充电解除SOC
	profile_data[38] = 0;	
	
	profile_data[39] = 0;    //充电完成解除SOC
	profile_data[40] = 0;
		
	profile_data[41] = 0;    //CV控制开始电芯电压1
	profile_data[42] = 0;
		
	profile_data[43] = 0;    //CV控制开始电芯电压2
	profile_data[44] = 0;		

	profile_data[45] = 0;    //CV控制开始电芯电压3
	profile_data[46] = 0;

	profile_data[47] = 0;    //CV控制开始电芯电压4
	profile_data[48] = 0;

	profile_data[49] = 0;    //放电中止电压
	profile_data[50] = 0;
	
	profile_data[51] = 0;    //过充电保护电压1
	profile_data[52] = 0;
	
	profile_data[53] = 0;    //过放电保护电压1
	profile_data[54] = 0;

	profile_data[55] = 0;    //过充电保护电压2
	profile_data[56] = 0;
	
	profile_data[57] = 0;    //过放电保护电压2
	profile_data[58] = 0;	
	
	profile_data[59] = 0;    //最大充电电流限制值
	profile_data[60] = 0;	
	
	profile_data[61] = 0;    //最大放电电流限制值
	profile_data[62] = 0;	
	
	profile_data[63] = 0;    //充电过电流
	profile_data[64] = 0;
	
	profile_data[65] = 0;    //充电放电流
	profile_data[66] = 0;
	
	profile_data[67] = 0;    //过温升保护温度
	profile_data[68] = 0;	 //均衡控制开始电压差
	profile_data[69] = 0;    //均衡控制开始电压差
	profile_data[70] = 0;    //满充电容量测定终止SOC
	
	profile_data[71] = 0;    //1C 充放电电流
	profile_data[72] = 0;
	
	profile_data[73] = 0;    //最大充电倍率设定1
	profile_data[74] = 0;

	profile_data[75] = 0;    //最大充电倍率设定2
	profile_data[76] = 0;
	
	profile_data[77] = 0;    //最大充电倍率设定3
	profile_data[78] = 0;
	
	profile_data[79] = 0;    //最大充电倍率设定4
	profile_data[80] = 0;
	
	profile_data[81] = 0;    //最大放电倍率设定1
	profile_data[82] = 0;

	profile_data[83] = 0;    //最大放电倍率设定2
	profile_data[84] = 0;
	
	profile_data[85] = 0;    //最大放电倍率设定3
	profile_data[86] = 0;
	
	profile_data[87] = 0;    //最大放电倍率设定4
	profile_data[88] = 0;
	
	profile_data[89] = 0;    //DOD设定1
	profile_data[90] = 0;    //DOD设定2
	profile_data[91] = 0;    //DOD设定3
	profile_data[92] = 0;    //DOD设定4
	
	profile_data[93] = 0;    //电芯的lot rank
	profile_data[94] = 0;
	
	profile_data[95] = check_sum(profile_data+3,93);
	send_message(profile_data,96);
}

void battery_answer(void)
{
	send_message(battery_data,55);
}

void latch_answer(void)
{
	uint8_t Send_buffer[10];
	Send_buffer[0] = 0x55;
	Send_buffer[1] = ID_address;
	Send_buffer[2] = Sequence_ID;
	Send_buffer[3] = 0x01;
	Send_buffer[4] = 0x58;
	Send_buffer[5] = Latch_id;    // 数据开始
	Send_buffer[6] = check_sum(Send_buffer+3,4);
	send_message(Send_buffer,7);
}

void profile_load(void)
{
	
}

void battery_load(void)
{
	int16_t current_temp = 0;
	uint16_t V_temp = 0;
	uint32_t DCH_CHG_temp = 0;
	
	battery_data[0] = 0x55;
	battery_data[1] = ID_address;
	battery_data[2] = Sequence_ID;
	battery_data[3] = 49;
	battery_data[4] = 0xD5;
	battery_data[5] = Latch_id; // 数据开始 latch id
	battery_data[6] = g_sys_cap.val.full_cap;//满充电容量
	battery_data[7] = g_sys_cap.val.full_cap >> 8;	
	battery_data[8] = g_sys_cap.val.cap_val;//剩余容量
	battery_data[9] = g_sys_cap.val.cap_val>>8;
	battery_data[10] = g_sys_cap.val.bat_cycle_cnt;//循环次数
	battery_data[11] = g_sys_cap.val.bat_cycle_cnt>>8;
	current_temp = nADC_CURRENT *1800/32768;//电流
	battery_data[12] =  current_temp;
	battery_data[13] =  current_temp>>8;
	battery_data[14] =  ( TEMP_3_BAT + TEMP_4_BAT + TEMP_5_BAT )/3;//平均电池温度
	battery_data[15] = nADC_TMONI_BAT_MIN;//最低电池温度
	battery_data[16] = nADC_TMONI_BAT_MAX;//最高电池温度
	battery_data[17] = 0; //检流电阻温度
	battery_data[18] = 0; //连接部温度
	V_temp = nADC_VPACK *6104 / 10000;
	battery_data[19] = V_temp; // 电池组电压
	battery_data[20] = V_temp >> 8;
	V_temp = Total_VBAT *305 / 10000;
	battery_data[21] = V_temp; // 平均电芯电压
	battery_data[22] = V_temp >> 8;
	V_temp = nADC_CELL_MIN *305 / 10000;
	battery_data[23] = V_temp; // 最小电芯电压
	battery_data[24] = V_temp >> 8;	
	V_temp = nADC_CELL_MAX *305 / 10000;
	battery_data[25] = V_temp; // 最大电芯电压
	battery_data[26] = V_temp >> 8;
	
	DCH_CHG_temp = DCH_Val/1000;
	battery_data[27] = DCH_CHG_temp;    //累积放电量
	battery_data[28] = DCH_CHG_temp>>8;
	battery_data[29] = DCH_CHG_temp>>16;
	battery_data[30] = DCH_CHG_temp>>24;
	
	DCH_CHG_temp = CHG_Val/1000;
	battery_data[31] = DCH_CHG_temp;    //累积充电量
	battery_data[32] = DCH_CHG_temp>>8;
	battery_data[33] = DCH_CHG_temp>>16;
	battery_data[34] = DCH_CHG_temp>>24;	
	
	battery_data[35] = Time_Val;    //累积利用时间
	battery_data[36] = Time_Val>>8;
	battery_data[37] = Time_Val>>16;
	battery_data[38] = Time_Val>>24;	
	
	battery_data[39] = 0;    //电芯充电限制电压
	battery_data[40] = 0;

	battery_data[41] = 0;    //充电限制电流
	battery_data[42] = 0;
	
	battery_data[43] = 0;    //放电限制电流
	battery_data[44] = 0;
	
	battery_data[45] = 0;    //TD Flag
	
	battery_data[46] = (AbnormalState.VAL);    //异常 Flag
	battery_data[47] = (AbnormalState.VAL)>>8;    //异常 Flag
	battery_data[48] = (AbnormalState.VAL)>>16;    //异常 Flag
	
	battery_data[49] = BatteryState.VAL;    //电池状态
	
	battery_data[50] = 0;    //过冲电保护等级3电压
	battery_data[51] = 0;
	
	battery_data[52] = 3445&0xff;    //电芯8%电压
	battery_data[53] = 3445>>8;
	
	battery_data[54] = check_sum(battery_data+3,52);
}

void Address_Init(void)
{
	static uint32_t time_count = 0;
	if (ID_END_Read() == false)
	{
		ID_address = 0x01;
		address_assign_flag = 0;
		ID_OUT_High();
		COM_RES_Low();
	}
	else
	{
		while(ID_IN_Read() == false);//等待ID_IN 为低
		delay_ms(10);
		while(ID_IN_Read() == false);//确认ID_IN 为低
		
		ID_address = 0x01;
		while( address_assign_flag == 1)
		{
			Address_Send();
			time_count = 0;
			address_conflict = 0;
			while(address_conflict == 0)
			{
				can_process();
				time_count++;
				delay_ms(1);
				if (time_count >= 50)
				{
					address_assign_flag = 0;
					ID_OUT_High();
					break;
				}
			}
			if (address_assign_flag == 1)
			{
				ID_address++;
			}
		}
		
	}
}

void Address_Send(void)
{
	uint8_t Send_buffer[10];
	Send_buffer[0] = 0x55;
	Send_buffer[1] = ID_address;
	Send_buffer[2] = 0x00;
	Send_buffer[3] = 0x00;
	Send_buffer[4] = 0x48;
	Send_buffer[5] = check_sum(Send_buffer+3,3);
	send_message(Send_buffer,6);
}

//void profile_init(void)
//{
	//Series_num = 13;
	//Parallel_num = 1;
	//Rated_Capacity = 6300;
	//Nominal_voltage = 4800;
	//CHG_Max_temp = 45;
	//CHG_Min_temp = -10;
	//Manufacturing_Name[0] = 0;
	//Manufacturing_Name[1] = 0;
	//Manufacturing_Name[2] = 0;
	//Manufacturing_Name[3] = 0;
	//Manufacturing_Name[4] = 0;
	//Manufacturing_Name[5] = 0;
	//Manufacturing_Name[6] = 0;
	//Manufacturing_Name[7] = 0;
	//Device_Name[0] = 0;
	//Device_Name[1] = 0;
	//Device_Name[2] = 0;
	//Device_Name[3] = 0;
	//Device_Name[4] = 0;
	//Device_Name[5] = 0;
	//Device_Name[6] = 0;
	//Device_Name[7] = 0;
	//Manufacturing_Data = 0;
	//Manufacturing_SN = 0;
	//Commun_Ver = 0;
	//FW_Ver = 0;
	//Data_Ver = 0;
	//Protect_Rev = 0;
	//Release_SOC = 0;
	//CV_Control_Cell_V1 = 0;
	//CV_Control_Cell_V2 = 0;
	//CV_Control_Cell_V3 = 0;
	//CV_Control_Cell_V4 = 0;
	//DCH_Stop_Vol = 0;
	//OCHG_Protect_Vol = 0;
	//ODCH_Protect_Vol = 0;
	//OCHG_Protect_Vol_MJ = 0;
	//ODCH_Protect_Vol_MJ = 0;
	//MAX_CHG_Current = 0;
	//MAX_DCH_Current = 0;
	//CHG_Overcurrent = 0;
	//DCH_Overcurrent = 0;
	//Temp_Protect = 0;
	//Balance_Start_Vol = 0;
	//Balance_Stop_Vol = 0;
	//Stop_SOC = 0;
	//Current_1C = 0;
	//Max_CHG_Mag1 = 0;
	//Max_CHG_Mag2 = 0;
	//Max_CHG_Mag3 = 0;
	//Max_CHG_Mag4 = 0;
	//Max_DCH_Mag1 = 0;
	//Max_DCH_Mag2 = 0;
	//Max_DCH_Mag3 = 0;
	//Max_DCH_Mag4 = 0;
	//DOD1 = 0;
	//DOD2 = 0;
	//DOD3 = 0;
	//DOD4 = 0;
	//LotRank = 0;	
//}