#ifndef __SE_PROTOCOL_H
#define __SE_PROTOCOL_H

#include <stdint.h>
#include "main.h"
#include "dma.h"
#include "usart.h"
/* ================== 注意事项 ================== */
//1.此通讯函数依赖STM32HAL库的HAL_UARTEx_ReceiveToIdle_DMA函数，其回调函数HAL_UARTEx_RxEventCallback
//写在了main.c中，移植时复制其中逻辑进入自己的回调函数中，不建议在库中写回调函数以防止重定义
//2.此通讯函数使用DMA模式，使用时务必打开串口DMA并且打开串口全局中断
//3.简单使用流程：
//a.调用void SE_Protocol_Init进行初始化（其中有串口DMA接受函数，移植时使用UART2以外的串口需要进行修改）
//b.在HAL_UARTEx_RxEventCallback中调用SE_Protocol_ParseBuffer进行数据包解析
//c.解析后的最新数据自动存入g_latest_sensor_val，需要时直接使用即可
//d.通讯逻辑0x45包头，0x53包尾，拼接中间4位Hex转化为float类型

/* ================== 宏定义 ================== */
#define RX_BUFFER_SIZE 128  //接受数据包缓冲区
/* ================== 全局变量声明 ================== */
// 存放最新解析出的浮点数 (volatile 防止编译器过度优化，保证每次都从内存读取最新值)
extern volatile float g_latest_sensor_val;
// 新数据标志位：当有新数据成功解析时置 1，用户使用后可手动清 0
extern volatile uint8_t g_sensor_data_updated;

extern uint8_t dma_rx_buffer[RX_BUFFER_SIZE];

/* ================== 函数接口声明 ================== */
// 初始化解析器状态
void SE_Protocol_Init(void);
// 解析单个字节（状态机核心）
void SE_Protocol_ParseByte(uint8_t rx_byte);
// 解析数据块（供 DMA 批量回调使用）
void SE_Protocol_ParseBuffer(uint8_t *buffer, uint16_t length);

#endif /* __SE_PROTOCOL_H */