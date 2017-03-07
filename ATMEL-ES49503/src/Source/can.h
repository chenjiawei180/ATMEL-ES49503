/*
 * can.h
 *
 * Created: 2017/2/27 9:06:00
 *  Author: chenjiawei
 */ 


#ifndef CAN_H_
#define CAN_H_

#include "asf.h"
#include "global.h"

#define CAN_MODULE              CAN0
#define CAN_TX_PIN              PIN_PA24G_CAN0_TX
#define CAN_TX_MUX_SETTING      MUX_PA24G_CAN0_TX
#define CAN_RX_PIN              PIN_PA25G_CAN0_RX
#define CAN_RX_MUX_SETTING      MUX_PA25G_CAN0_RX

#define CAN_RX_STANDARD_FILTER_INDEX_0    0
#define CAN_RX_STANDARD_FILTER_INDEX_1    1
#define CAN_RX_STANDARD_FILTER_ID_0     0x45A
#define CAN_RX_STANDARD_FILTER_ID_0_BUFFER_INDEX     2
#define CAN_RX_STANDARD_FILTER_ID_1     0x001
#define CAN_RX_EXTENDED_FILTER_INDEX_0    0
#define CAN_RX_EXTENDED_FILTER_INDEX_1    1
#define CAN_RX_EXTENDED_FILTER_ID_0     0x100000A5
#define CAN_RX_EXTENDED_FILTER_ID_0_BUFFER_INDEX     1
#define CAN_RX_EXTENDED_FILTER_ID_1     0x10000096

#define XMODEM_BUFLEN	256

extern void configure_can(void);
extern void can_set_standard_filter_1(void);
extern void can_send_standard_message(uint32_t id_value, uint8_t *data,uint32_t data_length);
extern void buff_init(void);
extern void write_byte(uint8_t byte);
extern void read_bytes(uint8_t * buffer, uint32_t byteCount);
//extern int read_packet(uint8_t *buffer, uint8_t idx);
extern uint8_t check_sum(uint8_t *buffer, uint8_t idx);
extern void send_message(uint8_t * buffer, uint8_t idx);
extern void can_process(void);

extern void address_answer(void);
extern void profile_answer(void);
extern void battery_answer(void);
extern void latch_answer(void);
extern void profile_load(void);
extern void battery_load(void);

extern void Address_Init(void);
extern void Address_Send(void);


#endif /* CAN_H_ */