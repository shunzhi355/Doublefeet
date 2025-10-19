
#include "include.h"
uint8 testData = 0xAA;

int main(void)
{
	SystemInit(); 			 //系统时钟初始化为72M	  SYSCLK_FREQ_72MHz
	InitDelay(72);	     //延时初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	InitTimer2();//用于产生100us的定时中断
	InitUart1();//用于与PC端进行通信
	InitUart3();//外接模块的串口
	InitADC();
	InitLED();
	InitKey();
	InitBuzzer();
	InitFlash();
	InitMemory();
	InitBusServoCtrl();
	LED = LED_ON;
	BusServoCtrl(1,SERVO_MOVE_TIME_WRITE,500,1000);
	BusServoCtrl(2,SERVO_MOVE_TIME_WRITE,500,1000);
	BusServoCtrl(3,SERVO_MOVE_TIME_WRITE,500,1000);
	BusServoCtrl(4,SERVO_MOVE_TIME_WRITE,500,1000);
	BusServoCtrl(5,SERVO_MOVE_TIME_WRITE,500,1000);
	BusServoCtrl(6,SERVO_MOVE_TIME_WRITE,500,1000);
	BusServoCtrl(7,SERVO_MOVE_TIME_WRITE,500,1000);
	BusServoCtrl(8,SERVO_MOVE_TIME_WRITE,500,1000);
	DelayMs(1000);
	
	
	while(1)
	{
		TaskRun();			//主控制程序
		// 初始化后立即发送测试数据
	
	
    // 每隔1秒发送一次0xAA
//     while((USART3->SR & 0X80) == 0);  // 等待发送缓冲区空
//     USART3->DR = testData;
//     DelayMs(1000);
	}
		//以下是LED闪烁测试代码，默认情况下请将其注释掉不使用，当需要测试主板LED功能时再取消注释进行使用，使用完恢复注释
		/*
		LED = 0 ;				//LED开
		DelayMs(500);   //延时500ms
		LED = 1 ;				//LED关
		DelayMs(500);   //延时500ms
		*/
}


