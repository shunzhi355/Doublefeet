#include "include.h"





static bool fUartRxComplete = FALSE;
static uint8 UartRxBuffer[260];



void InitUart3(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);
	//USART3_TX   PB.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//USART3_RX   PB.11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//USART 初始化设置

	USART_InitStructure.USART_BaudRate = 9600;//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART3, &USART_InitStructure);

	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启中断

	USART_Cmd(USART3, ENABLE);					  //使能串口
	
	
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);
}


void USART3_IRQHandler(void)                	//串口3中断服务程序
{
	u8 rxBuf;
	static uint8 startCodeSum = 0;
	static bool fFrameStart = FALSE;
	static uint8 messageLength = 0;
	static uint8 messageLengthSum = 0;
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		rxBuf =USART_ReceiveData(USART3);//(USART3->DR);	//读取接收到的数据

		if(!fFrameStart)
		{
			if(rxBuf == 0x55)
			{

				startCodeSum++;
				if(startCodeSum == 2)
				{
					startCodeSum = 0;
					fFrameStart = TRUE;
					messageLength = 0;
				}
			}
			else
			{

				fFrameStart = FALSE;
				messageLength = 0;
				startCodeSum = 0;
			}
			
		}
		// if(fFrameStart)
		// {
		// 	UartRxBuffer[messageLength] = rxBuf;
		// 	if(messageLength == 0)
		// 	{
		// 		messageLengthSum = UartRxBuffer[messageLength];
		// 		if(messageLengthSum < 1)// || messageLengthSum > 30
		// 		{
		// 			messageLengthSum = 2;
		// 			fFrameStart = FALSE;
				
		// 		}
					
		// 	}
		// 	messageLength++;
		else  // 已检测到起始码，接收有效数据
        {
            UartRxBuffer[messageLength] = rxBuf;
            
            // 第1个有效字节是长度字段（N）
            if(messageLength == 0)
            {
                messageLengthSum = UartRxBuffer[0];
                // 过滤无效长度（根据实际协议调整范围）
                if(messageLengthSum < 1 || messageLengthSum > 255)
                {
                    fFrameStart = FALSE;
                    messageLength = 0;
                    return;
                }
            }
            
            messageLength++;
	
			if(messageLength == messageLengthSum +2 ) 
			{

				fUartRxComplete = TRUE;

				fFrameStart = FALSE;
				messageLength = 0;  // 重置，准备下一次接收
			}
		}

	}

}

void USART3SendDataPacket(uint8 tx[],uint32 count)
{
	uint32 i;
	for(i = 0; i < count; i++)
	{
		while((USART3->SR&0X40)==0);//循环发送,直到发送完毕
		USART3->DR = tx[i];
		//while((USART3->SR&0X40)==0);//循环发送,直到发送完毕
	}
}

static bool UartRxOK(void)
{
	if(fUartRxComplete)
	{
		fUartRxComplete = FALSE;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void McuToPCSendDataByBLE(uint8 cmd,uint8 prm1,uint8 prm2)
{
	uint8 dat[8];
	uint8 datlLen = 2;
// 	switch(cmd)
// 	{

// //		case CMD_ACTION_DOWNLOAD:
// //			datlLen = 2;
// //			break;

// 		default:
// 			datlLen = 2;
// 			break;
// 	}

	dat[0] = 0x55;
	dat[1] = 0x55;
	dat[2] = datlLen;
	dat[3] = cmd;
	dat[4] = prm1;
	dat[5] = prm2;
	USART3SendDataPacket(dat,8);
}

void TaskBLEMsgHandle(void)
{

	uint16 i;
	uint8 cmd;
	uint8 id;
	uint8 servoCount;
	uint16 time;
	uint16 pos;
	uint16 times;
	uint8 fullActNum;
	if(UartRxOK())
	{
		LED = !LED;
		cmd = UartRxBuffer[1];
 		switch(cmd)
 		{
 			case CMD_MULT_SERVO_MOVE:
				servoCount = UartRxBuffer[2];
				time = UartRxBuffer[3] + (UartRxBuffer[4]<<8);
				for(i = 0; i < servoCount; i++)
				{
					id =  UartRxBuffer[5 + i * 3];
					pos = UartRxBuffer[6 + i * 3] + (UartRxBuffer[7 + i * 3]<<8);

					
					BusServoCtrl(id,SERVO_MOVE_TIME_WRITE,pos,time);
				}
				// 添加回传逻辑，通知上位机命令已执行
    			McuToPCSendDataByBLE(CMD_MULT_SERVO_MOVE, servoCount, 0); // 回传舵机数量作为参数
    
 				break;
			
			case CMD_FULL_ACTION_RUN:
				fullActNum = UartRxBuffer[2];//动作组编号
				if (UartRxBuffer[0] >= 3) { // 长度字段至少为3（命令+2字节参数）
        		times = UartRxBuffer[4] + (UartRxBuffer[5] << 8);
    			}		 
				else 
				{
        		times = 0; // 长度错误时强制为0（避免异常）
    			}
    			// 使用串口3的回传函数，正确填充长度和参数
    			McuToPCSendDataByBLE(CMD_FULL_ACTION_RUN, fullActNum, times& 0xFF);
				FullActRun(fullActNum,times);
				break;
				
			case CMD_FULL_ACTION_STOP:
				FullActStop();
				break;				
				
			case CMD_FULL_ACTION_ERASE:
				FlashEraseAll();
				McuToPCSendDataByBLE(CMD_FULL_ACTION_ERASE,0,0);
				break;

			case CMD_ACTION_DOWNLOAD:
				SaveAct(UartRxBuffer[4],UartRxBuffer[5],UartRxBuffer[6],UartRxBuffer + 7);
				McuToPCSendDataByBLE(CMD_ACTION_DOWNLOAD,0,0);
				break;

 		}
	}
}



