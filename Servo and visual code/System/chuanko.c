#include "stm32f10x.h"
#include <string.h>
#include "chuanko.h"

// 定义接收缓冲区
char RxBuffer[RX_BUFFER_SIZE];  // 接收数据的缓冲区
volatile uint8_t RxIndex = 0;   // 接收数据索引
volatile uint8_t data_received = 0; // 接收完成标志

// 偏差值（全局）
int16_t dx = 0, dy = 0;

// 串口接收状态机
typedef enum {
    STATE_WAIT_HEADER,          // 等待帧头
    STATE_RECEIVE_DATA,         // 接收数据部分
    STATE_WAIT_TAIL             // 等待帧尾
} ReceiveState;

ReceiveState state = STATE_WAIT_HEADER; // 当前接收状态

// 初始化串口1
void USART1_Init(void) {
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    // 使能USART1和GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    // 配置USART1 TX (PA9) 和 RX (PA10) 引脚
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;           // TX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;     // 复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;          // RX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置USART1参数
    USART_InitStructure.USART_BaudRate = 115200;        // 波特率115200
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;  // 8位数据
    USART_InitStructure.USART_StopBits = USART_StopBits_1; // 1位停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;  // 无校验
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件流控
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;  // 收发模式
    USART_Init(USART1, &USART_InitStructure);

    // 使能USART1
    USART_Cmd(USART1, ENABLE);

    // 使能USART1接收中断
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    
    // 设置中断优先级
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

// 串口接收中断处理函数
void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        uint8_t received = USART_ReceiveData(USART1);  // 接收数据

        switch (state) {
            case STATE_WAIT_HEADER:      // 等待帧头
                if (received == 0x2C) {  // 检测到帧头
                    state = STATE_RECEIVE_DATA; // 切换到接收数据状态
                    RxIndex = 0;         // 重置索引
                }
                break;

            case STATE_RECEIVE_DATA:     // 接收数据
                if (received == 0x5B) {  // 检测到帧尾
                    state = STATE_WAIT_TAIL; // 切换到等待帧尾状态
                    data_received = 1;   // 设置接收完成标志
                } else {
                    if (RxIndex < RX_BUFFER_SIZE - 1) {
                        RxBuffer[RxIndex++] = received;  // 存储数据
                    }
                }
                break;

            case STATE_WAIT_TAIL:       // 等待帧尾
                state = STATE_WAIT_HEADER; // 重置状态机
                break;
        }
    }
}

// 处理接收到的数据
void Process_Received_Data(void) {
    if (data_received) {
        // 解析偏差数据 (4字节: dx高, dx低, dy高, dy低)
        if (RxIndex >= 4) {
            // 组合dx (16位有符号)
            dx = (int16_t)((RxBuffer[0] << 8) | RxBuffer[1]);
            // 组合dy (16位有符号)
            dy = (int16_t)((RxBuffer[2] << 8) | RxBuffer[3]);
        }
        
        // 重置接收状态
        data_received = 0;
        RxIndex = 0;
        state = STATE_WAIT_HEADER;
    }
}