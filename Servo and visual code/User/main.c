#include "stm32f10x.h"                  // Device header
#include "Servo.h"
#include "OLED.h"
#include "KEY.h"
#include "Delay.h"
#include "chuanko.h"
#include "PID.h"
#include "PWM.h"
#include "Laser.h"

// 接收数据长度
extern char RxBuffer[RX_BUFFER_SIZE];  // 接收数据的缓冲区
extern volatile uint8_t RxIndex;   // 接收数据的索引
extern uint8_t data_received;
extern int16_t dx,dy;

int x = 0,y = 0,x_err = 0,y_err = 0;

// 主函数
int main(void) 
{
	Servo_Init();
	USART1_Init();
	OLED_Init();

	
  OLED_ShowString(1, 1, "dx:");
  OLED_ShowString(2, 1, "dy:");
  OLED_ShowString(3, 1, "Y_PWM:");
  OLED_ShowString(4, 1, "X_PWM:");
	
  
  NVIC_EnableIRQ(USART1_IRQn);
	
	Servo_SetAngle1(90);
	Servo_SetAngle2(135);
	Delay_ms(500);	
	
	
    while (1) 
	{
		Process_Received_Data();
		// 显示偏差值
		OLED_ShowSignedNum(1, 4, dx, 5);  // X偏差
		OLED_ShowSignedNum(2, 4, dy, 5);  // Y偏差
		if(dx != 0)
		{
			pid_S_X(dx,0);
		}
		if(dy != 0)
		{
			pid_S_Y(dy,0);
		}
		
	}
}	

