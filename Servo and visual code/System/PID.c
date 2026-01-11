#include "stm32f10x.h"                  // Device header
#include "PWM.h"
#include "OLED.h"

// PID控制变量
int16_t Err_X = 0, Err_Y = 0;

int16_t x_pwm = 0,
        now_x;

int16_t y_pwm = 0,
        now_y,
        ks1 = 1500,  // Y轴中位值
        ks2 = 1500;   // X轴起始值
				
float Err_S_Y = 0,
      last_Err_S_Y = 0,
      integral = 0,
      p_S = -0.4,      // Y轴比例系数
      i_S = -0.09,      // Y轴积分系数
      d_S = -0.06;      // Y轴微分系数


float Err_S_X = 0,
      last_Err_S_X = 0,
      integral_X = 0,
      p_S_X = -0.34,    // X轴比例系数
      i_S_X = -0.09,    // X轴积分系数
      d_S_X = -0.06;    // X轴微分系数



// Y轴PID控制
void pid_S_Y(float true_S, float tar_S)
{
    // 计算误差 (目标0 - 当前偏差)
    Err_S_Y = tar_S - true_S;
    
    // 积分项累加
    integral += Err_S_Y;
    
    // 积分限幅防止饱和
    if(integral > 1000) integral = 1000;
    else if(integral < -1000) integral = -1000;
    
    // PID计算
    y_pwm = p_S * Err_S_Y + d_S*(Err_S_Y - last_Err_S_Y) + i_S * integral;
    last_Err_S_Y = Err_S_Y;
    
    // 计算PWM值 (中位1500)
    now_y = ks1 + y_pwm;
    
    // PWM限幅保护 (500-2500μs)
    if(now_y > 2500) now_y = 2500;
    else if(now_y < 500) now_y = 500;
    
    // 设置PWM输出
    PWM_SetCompare2(now_y);
    // OLED显示Y轴PWM值
    OLED_ShowSignedNum(3, 8, now_y, 5);
}

// X轴PID控制
void pid_S_X(float true_S, float tar_S)
{
    // 计算误差 (目标0 - 当前偏差)
    Err_S_X = tar_S - true_S;
    
    // 积分项累加
    integral_X += Err_S_X;
    
    // 积分限幅防止饱和
    if(integral_X > 1000) integral_X = 1000;
    else if(integral_X < -1000) integral_X = -1000;
    
    // PID计算
    x_pwm = p_S_X * Err_S_X + d_S_X*(Err_S_X - last_Err_S_X) + i_S_X * integral_X;
    last_Err_S_X = Err_S_X;
    
    // 计算PWM值 (起始500)
    now_x = ks2 + x_pwm;
    
    // PWM限幅保护 (500-2500μs)
    if(now_x > 2500) now_x = 2000;
    else if(now_x < 500) now_x = 500;
    
    // 设置PWM输出
    PWM_SetCompare3(now_x);
    // OLED显示X轴PWM值
    OLED_ShowSignedNum(4, 8, now_x, 5);
}