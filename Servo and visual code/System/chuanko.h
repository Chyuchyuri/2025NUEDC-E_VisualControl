#ifndef _CHUANKO_H
#define _CHUANKO_H

#define RX_BUFFER_SIZE 64

extern int16_t dx, dy;  // Æ«²îÖµ

void USART1_Init(void);
void USART1_IRQHandler(void);
void Process_Received_Data(void);

#endif
