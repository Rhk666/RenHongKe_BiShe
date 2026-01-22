
//单片机头文件

//网络设备驱动
#include "esp8266.h"

//硬件驱动
#include "usart.h"
//C库
#include <string.h>
#include <stdio.h>

#define ONENET_RETRY_MAX 8  // OneNet连接最大重试次数
#define WIFI_RETRY_MAX 8  // 最大重试次数，方便修改

#define ESP8266_WIFI_INFO		"AT+CWJAP=\"stm32f407\",\"rgh2742714184\"\r\n"//手机热点wifi的账号和密码
//#define ESP8266_WIFI_INFO		"AT+CWJAP=\"NEW_5501-B\",\"elec5501\"\r\n"//实验室wifi的账号和密码
#define ESP8266_ONENET_INFO		"AT+CIPSTART=\"TCP\",\"mqtts.heclouds.com\",1883\r\n"
extern unsigned char p3Data,p5Data,p2Data;
unsigned char esp8266_buf[512];
unsigned short esp8266_cnt = 0, esp8266_cntPre = 0;
unsigned int WiFi_Success_Flag=0;
unsigned char Rx_Data[20],Rx_Flag=0;
extern unsigned char anjianzhi[6];
extern float Roll,Pitch,Yaw,My_Yaw,Delta_Yaw,Last_Yaw,Yaw_offset,First_Yaw;
extern int First_Yaw_flag,Quan_Shu;
//==========================================================
//	函数名称：	ESP8266_Clear
//
//	函数功能：	清空缓存
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void ESP8266_Clear(void)
{

	memset(esp8266_buf, 0, sizeof(esp8266_buf));
	esp8266_cnt = 0;

}

//==========================================================
//	函数名称：	ESP8266_WaitRecive
//
//	函数功能：	等待接收完成
//
//	入口参数：	无
//
//	返回参数：	REV_OK-接收完成		REV_WAIT-接收超时未完成
//
//	说明：		循环调用检测是否接收完成
//==========================================================
_Bool ESP8266_WaitRecive(void)
{

	if(esp8266_cnt == 0) 							//如果接收计数为0 则说明没有处于接收数据中，所以直接跳出，结束函数
		return REV_WAIT;
		
	if(esp8266_cnt == esp8266_cntPre)				//如果上一次的值和这次相同，则说明接收完毕
	{
		esp8266_cnt = 0;							//清0接收计数
			
		return REV_OK;								//返回接收完成标志
	}
		
	esp8266_cntPre = esp8266_cnt;					//置为相同
	
	return REV_WAIT;								//返回接收未完成标志

}

//==========================================================
//	函数名称：	ESP8266_SendCmd
//
//	函数功能：	发送命令
//
//	入口参数：	cmd：命令
//				res：需要检查的返回指令
//
//	返回参数：	0-成功	1-失败
//
//	说明：		
//==========================================================
_Bool ESP8266_SendCmd(char *cmd, char *res)
{
	
	unsigned char timeOut = 180;
	HAL_UART_Transmit(&huart3,(unsigned char *)cmd,strlen((const char *)cmd),10);
//	return 1;
//	Usart_SendString(USART2, (unsigned char *)cmd, strlen((const char *)cmd));
	
	while(timeOut--)
	{
		if(ESP8266_WaitRecive() == REV_OK)							//如果收到数据
		{
			if(strstr((const char *)esp8266_buf, res) != NULL)		//如果检索到关键词
			{
				ESP8266_Clear();									//清空缓存
				
				return 0;
			}
		}
		
		HAL_Delay(10);
	}
	
	return 1;

}

//==========================================================
//	函数名称：	ESP8266_SendData
//
//	函数功能：	发送数据
//
//	入口参数：	data：数据
//				len：长度
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void ESP8266_SendData(unsigned char *data, unsigned short len)
{

	char cmdBuf[32];
	
	ESP8266_Clear();								//清空接收缓存
	sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len);		//发送命令
	if(!ESP8266_SendCmd(cmdBuf, ">"))				//收到‘>’时可以发送数据
	{
		HAL_UART_Transmit(&huart3,(unsigned char *)data,len,100);
//		Usart_SendString(USART2, data, len);		//发送设备连接请求数据
	}

}

//==========================================================
//	函数名称：	ESP8266_GetIPD
//
//	函数功能：	获取平台返回的数据
//
//	入口参数：	等待的时间(乘以10ms)
//
//	返回参数：	平台返回的原始数据
//
//	说明：		不同网络设备返回的格式不同，需要去调试
//				如ESP8266的返回格式为	"+IPD,x:yyy"	x代表数据长度，yyy是数据内容
//==========================================================
unsigned char *ESP8266_GetIPD(unsigned short timeOut)
{

	char *ptrIPD = NULL;
	
	do
	{
		if(ESP8266_WaitRecive() == REV_OK)								//如果接收完成
		{	
			ptrIPD = strstr((char *)esp8266_buf, "IPD,");				//搜索“IPD”头
			if(ptrIPD == NULL)											//如果没找到，可能是IPD头的延迟，还是需要等待一会，但不会超过设定的时间
			{
//				printf("\"IPD\" not found\r\n");
			}
			else
			{
				ptrIPD = strchr(ptrIPD, ':');							//找到':'
				if(ptrIPD != NULL)
				{
					ptrIPD++;
					return (unsigned char *)(ptrIPD);
				}
				else
					return NULL;
				
			}
		}
		
		HAL_Delay(5);
        timeOut--;		//延时等待
	} while(timeOut>0);
	
	return NULL;														//超时还未找到，返回空指针

}

//==========================================================
//	函数名称：	ESP8266_Init
//
//	函数功能：	初始化ESP8266
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
// 原循环的修改版：最多重试5次WiFi连接，失败则跳出并打印提示
void ESP8266_ConnectWiFi(void)
{
    int retry_count = 0;  // 重试计数器，初始化为0
    _Bool wifi_connect_status = 0;  // WiFi连接状态：0=失败，1=成功

    printf("开始连接WiFi，最大重试%d次...\r\n", WIFI_RETRY_MAX);

    // 循环重试WiFi连接，直到成功 或 达到最大重试次数
    while(retry_count < WIFI_RETRY_MAX)
    {
        // 执行WiFi连接指令，判断是否收到"GOT IP"（成功标识）
        // ESP8266_SendCmd返回值：1=成功（收到GOT IP），0=失败
        if(ESP8266_SendCmd(ESP8266_WIFI_INFO, "GOT IP")==0)
        {
            // 连接成功，标记状态并跳出循环
            wifi_connect_status = 1;
            printf("WiFi连接成功！（重试次数：%d）\r\n", retry_count + 1);
					WiFi_Success_Flag=1;
            break;
        }
        else
        {
            // 连接失败，计数器+1
            retry_count++;
            printf("WiFi连接失败，剩余重试次数：%d\r\n", WIFI_RETRY_MAX - retry_count);
            
            // 失败后延时500ms再重试（避免频繁发送指令）
            HAL_Delay(100);  // 替换为你平台的延时函数，如HAL_Delay(500)
        }
    }

    // 循环结束后判断最终状态
    if(!wifi_connect_status)
    {
        printf("错误：WiFi连接重试%d次均失败，已退出重试！\r\n", WIFI_RETRY_MAX);
    }
}
void ESP8266_ConnectOneNet(void)
{
    int retry_count = 0;          // 重试计数器，初始为0
    _Bool onenet_connect_status = 0;  // OneNet连接状态：0=失败，1=成功

    printf("开始连接OneNet服务器，最大重试%d次...\r\n", ONENET_RETRY_MAX);

    // 循环重试OneNet连接，直到成功 或 达到最大重试次数
    while(retry_count < ONENET_RETRY_MAX)
    {
        // 执行OneNet连接指令，判断是否收到"CONNECT"成功标识
        // ESP8266_SendCmd返回值：1=成功（收到CONNECT），0=失败
        if(ESP8266_SendCmd(ESP8266_ONENET_INFO, "CONNECT")==0)
        {
            // 连接成功，标记状态并跳出循环
            onenet_connect_status = 1;
            printf("OneNet服务器连接成功！（重试次数：%d）\r\n", retry_count + 1);
            break;
        }
        else
        {
            // 连接失败，计数器+1
            retry_count++;
            printf("OneNet服务器连接失败，剩余重试次数：%d\r\n", ONENET_RETRY_MAX - retry_count);
            
            // 失败后延时1000ms再重试（OneNet连接建议延时稍长，避免服务器限流）
            HAL_Delay(100);  // 替换为你平台的延时函数，如HAL_Delay(1000)
        }
    }

    // 循环结束后判断最终状态
    if(!onenet_connect_status)
    {
        printf("错误：OneNet服务器连接重试%d次均失败，已退出重试！\r\n", ONENET_RETRY_MAX);
    }
}
void ESP8266_Init(void)
{
		
		ESP8266_Clear();
		printf("1. AT\r\n");
//		printf("1. AT\r\n");
		while(ESP8266_SendCmd("AT\r\n", "OK"))
		HAL_Delay(100);
	
		printf("2. CWMODE\r\n");
		while(ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK"))
		HAL_Delay(100);
	
		printf( "3. AT+CWDHCP\r\n");
		while(ESP8266_SendCmd("AT+CWDHCP=1,1\r\n", "OK"))
		HAL_Delay(100);
	
		printf("4. CWJAP\r\n");
		
		ESP8266_ConnectWiFi();
//		while(ESP8266_SendCmd(ESP8266_WIFI_INFO, "GOT IP"))
//		HAL_Delay(500);
	
		if(WiFi_Success_Flag==1){  //wifi连接成功
		printf("5. CIPSTART\r\n");
		ESP8266_ConnectOneNet();
//		while(ESP8266_SendCmd(ESP8266_ONENET_INFO, "CONNECT"))
//		HAL_Delay(500);
	
		printf("6. ESP8266 Init OK\r\n");		
		}
}

//==========================================================
//	函数名称：	USART2_IRQHandler
//
//	函数功能：	串口2收发中断
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
//void USART2_IRQHandler(void)
//{

//	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) //接收中断
//	{
//		if(esp8266_cnt >= sizeof(esp8266_buf))	esp8266_cnt = 0; //防止串口被刷爆
//		esp8266_buf[esp8266_cnt++] = USART2->DR;
//				
//		USART_ClearFlag(USART2, USART_FLAG_RXNE);
//	}

//}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	static unsigned char Temp_Data,Rxstate,first_Flag,RX_ZHEN;
	if (huart == &huart3) //esp8266
	{
		HAL_UART_Receive_IT(&huart3, &p3Data, 1);
	if(esp8266_cnt >= sizeof(esp8266_buf))	esp8266_cnt = 0; //防止串口被刷爆
		esp8266_buf[esp8266_cnt++] = p3Data;
//		printf("%c", p3Data);
	}
	if (huart == &huart5) //debug(蓝牙)
	{
		HAL_UART_Receive_IT(&huart5, &p5Data, 1);
	}
	if (huart == &huart2) //陀螺仪
	{
		HAL_UART_Receive_IT(&huart2, &p2Data, 1);
//		if(anjianzhi[0]==1){
//		printf("%d\r\n",p2Data);
//		}
		
		Temp_Data = p2Data;
		if (Rxstate == 0)
		{
			if (Temp_Data == 0x01 && first_Flag == 0)
			{
				first_Flag = 1;
			}
			else if (first_Flag == 1 && Temp_Data == 0x06)
			{
				first_Flag = 0;
				Rxstate = 1;
				RX_ZHEN = 0;
			}
		}
		else if (Rxstate == 1)
		{
			Rx_Data[RX_ZHEN] = Temp_Data;
			RX_ZHEN++;
			if (RX_ZHEN >= 8)
			{
				Rxstate = 2;
			}
		}
		else if (Rxstate == 2)
		{
			if (Temp_Data == 0x55)
			{

				Rxstate = 0;
				Rx_Flag = 1;				
			}
		}
//		printf("%c", p5Data);
	}	
	
}
unsigned int Usart_Error_count=0;
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
        // 清除错误标志，重新开启接收中断
			Usart_Error_count++;
				HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_2);
        __HAL_UART_CLEAR_FLAG(&huart2, UART_FLAG_ORE | UART_FLAG_FE | UART_FLAG_NE);
        HAL_UART_Receive_IT(&huart2, &p2Data, 1);
    }
}





char RX_Re_Data()
{
	if (Rx_Flag == 1)
	{
		Rx_Flag = 0;
		return 1;
	}
	return 0;
}
int fputc(int ch, FILE *f)
{
	// 采用轮询方式发送1字节数据，超时时间设置为无限等待
	HAL_UART_Transmit(&huart5, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
	return ch;
}
int fgetc(FILE *f)
{
	uint8_t ch;
	// 采用轮询方式接收 1字节数据，超时时间设置为无限等待
	HAL_UART_Receive(&huart5, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
	return ch;
}