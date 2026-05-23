#ifndef __MYCAN_H
#define __MYCAN_H

void MyCAN_Init(void);
uint8_t can_send_data(CAN_TypeDef *hcan, uint32_t id, uint8_t *data, uint32_t len);
void MyCAN_Transmit(uint32_t ID, uint8_t Length, uint8_t *Data);
uint8_t MyCAN_ReceiveFlag(void);
void MyCAN_Receive(uint32_t *ID, uint8_t *Length, uint8_t *Data);

#endif
