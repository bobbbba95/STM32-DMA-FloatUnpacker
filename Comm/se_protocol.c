#include "se_protocol.h"

/* ================== 全局变量定义 ================== */
volatile float g_latest_sensor_val = 0.0f;
volatile uint8_t g_sensor_data_updated = 0;

/* ================== 内部数据结构 ================== */
// 联合体：巧妙实现 4 字节到 float 的内存共享转换
typedef union {
    uint8_t bytes[4];
    float f_val;
} FloatConverter;

// 状态机枚举 
typedef enum {
    STATE_WAIT_HEADER = 0,   // 等待帧头 0x45
    STATE_RECEIVE_DATA,      // 接收 4 字节有效数据
    STATE_WAIT_TAIL          // 等待帧尾 0x53 校验
} ParserState;

// 静态变量：保留状态机的历史记忆
static ParserState current_state = STATE_WAIT_HEADER;
static FloatConverter converter;
static uint8_t byte_count = 0;

// DMA缓冲区定义
uint8_t dma_rx_buffer[RX_BUFFER_SIZE];

/* ================== 函数实现 ================== */

void SE_Protocol_Init(void) {
    current_state = STATE_WAIT_HEADER;
    byte_count = 0;
    g_latest_sensor_val = 0.0f;
    g_sensor_data_updated = 0;
	  //参数初始化完成以后打开DMA接受，关闭传输过半中断
	  HAL_UARTEx_ReceiveToIdle_DMA(&huart2, dma_rx_buffer, RX_BUFFER_SIZE);
    __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);	
}

void SE_Protocol_ParseByte(uint8_t rx_byte) {
    switch(current_state) {
        
        case STATE_WAIT_HEADER:
            if (rx_byte == 0x45) { // 识别到帧头 0x45
                current_state = STATE_RECEIVE_DATA;
                byte_count = 0; // 清零计数器，准备接收数据
            }
            break;
            
        case STATE_RECEIVE_DATA:
            // 直接按顺序存入联合体 (低位先发，天然匹配小端)
            converter.bytes[byte_count++] = rx_byte;
            
            if (byte_count == 4) {
                // 4字节收齐，进入帧尾校验阶段
                current_state = STATE_WAIT_TAIL;
            }
            break;
            
        case STATE_WAIT_TAIL:
            if (rx_byte == 0x53) { // 识别到帧尾 0x53，校验成功！
                // 只有头尾都匹配，才判定这是一帧有效数据
                g_latest_sensor_val = converter.f_val;
                g_sensor_data_updated = 1; 
            }
            // 无论校验成功还是失败（比如遇到错位杂音），都强制重置状态机
            current_state = STATE_WAIT_HEADER;
            break;
            
        default:
            current_state = STATE_WAIT_HEADER;
            break;
    }
}

// 供外部 DMA 调用的批量处理接口
void SE_Protocol_ParseBuffer(uint8_t *buffer, uint16_t length) {
    for (uint16_t i = 0; i < length; i++) {
        SE_Protocol_ParseByte(buffer[i]);
    }
}