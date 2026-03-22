# SE-Protocol-Parser 🚀

![License](https://img.shields.io/badge/License-MIT-blue.svg)
![Platform](https://img.shields.io/badge/Platform-STM32%20%7C%20ARM-orange.svg)
![Language](https://img.shields.io/badge/Language-C-green.svg)

> **一个专为单片机（以 STM32 为例）打造的轻量级、非阻塞式串口数据包解析库。**
> 完美解决嵌入式开发中高频连续串口接收导致的“数据粘包”、“半包”、“CPU 阻塞”以及“大小端对齐”等痛点。

## ✨ 核心特性 (Features)

* ⚡ **零 CPU 占用接收**：深度结合 STM32 的 `USART DMA` + `IDLE 空闲中断`，底层数据搬运完全由硬件代劳。
* 🛡️ **严格的状态机 (FSM) 校验**：采用 `帧头 + 数据 + 帧尾` 的剥洋葱式解析，彻底过滤错位数据和乱码干扰。
* ⏱️ **极致的实时性**：无需传统的长缓冲排队等待，最新数据随用随取，专为电机控制、传感器读取等高频实时场景设计。
* 🧩 **内存对齐与大小端适配**：优雅地使用 `Union` 联合体进行字节拼接，无缝处理上位机到下位机的 Float 类型转换。

## 📦 协议格式 (Protocol Format)

本项目默认解析的数据包格式如下（共 6 字节）：

| 帧头 (Header) | 数据 (Payload - 小端模式) | 帧尾 (Tail) |
| :---: | :---: | :---: |
| `0x45` ('E') | `[Byte 0] [Byte 1] [Byte 2] [Byte 3]` | `0x53` ('S') |

* **数据载荷**：4 字节单精度浮点数 (`float / IEEE 754`)
* **字节序**：低位在前 (Little-Endian)，如发送 `-120.857` (Hex: `C2 F1 B6 C9`)，串口物理发送顺序为 `C9 -> B6 -> F1 -> C2`。

## 🛠️ 如何集成到你的项目 (Quick Start)

### 1. 添加文件
将本仓库中的 `se_protocol.c` 和 `se_protocol.h` 添加到你的工程目录，并在 IDE 中将其包含进编译路径。

### 2. 初始化与配置 (基于 STM32 HAL 库)
在 `main.c` 中包含头文件并初始化：

```c
#include "se_protocol.h"

int main(void) {
    // ... 硬件初始化代码 ...
    
    // 1. 初始化解析器状态机
    SE_Protocol_Init();
    
    
    while (1) {
        // 主循环处理业务
    }

### 3. 在回调中“喂”数据
将收到的 DMA 数据块丢给解析器：

```c
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
    if (huart->Instance == USART1) {
        // 送入状态机解析
        SE_Protocol_ParseBuffer(dma_rx_buffer, Size);
        // 重新开启接收
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, dma_rx_buffer, RX_BUFFER_SIZE);
        __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
    }
}
```

### 4. 随用随取最新数据
在你的控制算法或业务逻辑中直接读取全局看板变量：

```c
if (g_sensor_data_updated) {
    float current_val = g_latest_sensor_val;
    g_sensor_data_updated = 0; // 清除标志位
    
    // Do something with current_val...
}
```

## 🤝 贡献与反馈
如果你在使用过程中发现了 Bug，或者有更好的协议适配建议，欢迎提交 Issue 或 Pull Request！
