#ifndef __MYSPI_H
#define __MYSPI_H

#include "stm32f10x.h"

// 函数声明
void MySPI_Init(void);
void MySPI_Init_Slave(void);           // 新增：SPI从机初始化
void MySPI_Start(void);
void MySPI_Stop(void);
uint8_t MySPI_SwapByte(uint8_t ByteSend);

// 从机模式专用函数
uint8_t MySPI_ReceiveByte(void);       // 接收一个字节
void MySPI_SendByte(uint8_t ByteSend); // 发送一个字节
uint8_t MySPI_R_SS(void);              // 读取SS引脚状态
uint8_t MySPI_IsSelected(void);        // 检查是否被选中
void MySPI_Process_Slave(void);        // 新增：从机处理函数

// 全局变量声明（用于SPI从机通信）
extern volatile uint8_t spi_rx_buffer[32];
extern volatile uint8_t spi_tx_buffer[32];
extern volatile uint8_t spi_data_ready;
extern volatile uint8_t spi_received_byte;
extern volatile uint32_t spi_rx_count;
extern volatile uint32_t spi_rx_length;

#endif
