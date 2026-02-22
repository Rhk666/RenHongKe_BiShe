/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "esp8266.h"
#include "onenet.h"
#include <stdlib.h>
#include "OLED.h"
#include "MY_IIC.h"
#include "BMP280.h"
#include "ath20.h"
#include "math.h"
#include "draw.h"
#include "game1.h"
#include "resources.h"
#include "queue.h"
#include "string.h"   
#include "NRF24L01.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

float PT, T, ALT;
uint32_t CT_data[2];
int c1, t1;
uint8_t status = 0;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
const char devPubTopic[] = "$sys/nE4HArK3N3/Pena/thing/property/post";
const char *devSubTopic[] = {"$sys/nE4HArK3N3/Pena/thing/property/set"};
unsigned char *dataPtr = NULL;
char PUBLIS_BUF[256];
extern unsigned int miao,count;
TaskHandle_t  OLED_Task_Handle;
unsigned int OLED_RUN_Flag=1;

extern unsigned int WiFi_Success_Flag;
float PT, T, ALT;
uint32_t CT_data[2];
int c1, t1;

unsigned int ADzhi11,My_ADzhi11,Last_ADzhi11;
unsigned int ADzhi12,My_ADzhi12,Last_ADzhi12;
unsigned int ADzhi21,My_ADzhi21,Last_ADzhi21;
unsigned int ADzhi22,My_ADzhi22,Last_ADzhi22;
unsigned char anjianzhi[6];
unsigned int NRF24L01_Send_Error[5];

extern uint8_t uptMove;
//extern uint8_t again;

volatile float Roll = 0.0f, Pitch = 0.0f, Yaw = 0.0f;
volatile float First_Yaw = 0.0f, Last_Yaw = 0.0f, Yaw_offset = 0.0f, My_Yaw = 0.0f;
volatile int Quan_Shu = 0;
volatile uint8_t First_Yaw_flag = 0;

QueueHandle_t miao_Duilie;
QueueHandle_t ESP8266_Duilie;
QueueHandle_t xGyroQueue;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void JsonValue()
{
	struct Chuan_Struct Data_Struct;

//	uint8_t Temp = c1;
//	uint8_t Hum_D = t1/10;
//	uint8_t Hum_X = t1%10;
//	float Hum=Hum_D+((float)Hum_X/10);
////	uint8_t Hum = t1/10;
//	float qiya=PT;
//	uint8_t shidu=c1/10;
	if(xQueueReceive(ESP8266_Duilie,&Data_Struct,pdMS_TO_TICKS(1))==pdPASS){
	
	uint8_t Temp = Data_Struct.C1;
	uint8_t Hum_D = Data_Struct.T1/10;
	uint8_t Hum_X = Data_Struct.T1%10;
	float Hum=Hum_D+((float)Hum_X/10);
		if(Hum<=40&&Hum>=0){
//	uint8_t Hum = t1/10;
	float qiya=Data_Struct.PT1;
	uint8_t shidu=Data_Struct.C1/10;	
	memset(PUBLIS_BUF, 0, sizeof(PUBLIS_BUF));
	
	sprintf(PUBLIS_BUF,"{\"id\":\"123\",\"params\":{\"temp_float\":{\"value\":%.1f},\"Qiya\":{\"value\":%.1f},\"ShiDu\":{\"value\":%d}}}",
					Hum,qiya,shidu);	
		}
	}
}
char anjianhq(){
	char anjianzhi=0;
	if(HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_10)==0){
	anjianzhi=1;
	}
	if(HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_14)==0){
	anjianzhi=2;
	}	
	return anjianzhi;
}
void LED_2Task(){
  while(1){
		OLED_ShowNum(3,1,count%10,2);
//		vTaskDelay(pdMS_TO_TICKS(500));	 // �ȴ�100ms
  }
}
void ESP8266_Task(){

	static unsigned int TimeCount,Error_Count;
	 static uint32_t tick_count = 0;
  static uint32_t last_send_tick = 0; // 记录上次发送的时间点
//	ESP8266_Duilie=xQueueCreate(1, sizeof(struct Chuan_Struct)); //创建队列
		ESP8266_Init();
	if(WiFi_Success_Flag==1){
			while(OneNet_DevLink())//连接Onenet平台,如果失败等待200ms继续尝试。
		{
			HAL_Delay(200);
			Error_Count++;
			if(Error_Count>=5){
				printf("连接失败\r\n");
				break;
//			return 0;
			}
		}	
	}
	else{
				printf("连接失败\r\n");
	}
			/*订阅主题*/
	OneNet_Subscribe(devSubTopic,1);
//	OLED_ShowString(3,1,"ESP8266_OK");
  while(1){
		
				HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_11);		
        dataPtr = ESP8266_GetIPD_RTOS(1); 
        if(dataPtr != NULL)
        {
            OneNet_RevPro(dataPtr); // 处理数据
            ESP8266_Clear();        // 处理完立刻清空
        }					
        if((xTaskGetTickCount() - last_send_tick) >= pdMS_TO_TICKS(500))
        {
            last_send_tick = xTaskGetTickCount(); // 更新时间戳4

//						JH_Read_CTdata(CT_data);         //较慢               
//						c1 = CT_data[0] * 1000 / 1024 / 1024;           
//						t1 = CT_data[1] * 200 * 10 / 1024 / 1024 - 500; 
//						BMP280GetData(&PT, &T, &ALT);

            JsonValue();
            OneNet_Publish(devPubTopic, PUBLIS_BUF);
        }		
					vTaskDelay(pdMS_TO_TICKS(5));	 // 
  }
}

struct OLED_Show_Param OLED_Test1={1,1,0};
struct OLED_Show_Param OLED_Test2={2,1,0};
struct OLED_Show_Param OLED_Test3={3,1,0};
unsigned int Bao_Hu=1;
void OLED_Task(void *param){
	struct OLED_Show_Param *info=param;
	struct miao_Duilie_Struct miao_Struct; //存数据的结构体
	
	BaseType_t preTime;
	preTime=xTaskGetTickCount();
	
//  miao_Duilie=xQueueCreate(1, sizeof(struct miao_Duilie_Struct)); //创建队列
	
	while(1){
		if(Bao_Hu==1){
		Bao_Hu=0;
		OLED_ShowNum(info->hang,info->lie,info->num,5);
		(info->num)++;
		Bao_Hu=1;
		}
		if(xQueueReceive(miao_Duilie,&miao_Struct,portMAX_DELAY)==pdPASS){ //判断是否接收到数据
		OLED_ShowNum(4,1,miao_Struct.num,4); //引用队列，用miao_Struct接收 miao_Duilie属于标识符
		}
		vTaskDelayUntil(&preTime,pdMS_TO_TICKS(1000));
//		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}
unsigned int A=0;
static unsigned char val,down,old;
void anjian_Task(){
	
	while(1){
	val=anjianhq();
	down=val&(val^old);
	old=val;
	if(down==1){
	A++;
		uptMove=2;
	}
	if(down==2){
		uptMove=1;
	A--;
	}
	vTaskDelay(pdMS_TO_TICKS(10));
	}
}

void IIC_Task(){
	static uint8_t ret;
	struct Chuan_Struct idata;
	
	  I2C_Bus_Init();
  ret = BMP280_Init();
  if (ret != 0x58)
  {
    while (1)
      ;
  }
  ret = JH_Init(); //
  if (ret == 0)
  {
    while (1)
      ;
  }
	while(1){
		JH_Read_CTdata(CT_data);         //较慢               
		c1 = CT_data[0] * 1000 / 1024 / 1024;           
		t1 = CT_data[1] * 200 * 10 / 1024 / 1024 - 500; 
		BMP280GetData(&PT, &T, &ALT);
		
		idata.C1=c1;
		idata.PT1=PT;
		idata.T1=t1;
		xQueueOverwrite(ESP8266_Duilie,&idata); // 无需等待，直接覆盖旧数据
	//	xQueueSend(ESP8266_Duilie,&idata,20); //有延迟 等待读取后再写
		vTaskDelay(pdMS_TO_TICKS(5));
	}
}
void NRF_Task(){
	 NRF24L01_Init();
	static unsigned int count=0;
	while(1){
		if(++count==50){
			count=0;	
      NRF24L01_TxPacket[0] = t1;
      NRF24L01_TxPacket[1]++;
      NRF24L01_TxPacket[2]++;
      NRF24L01_TxPacket[3]++;
//			NRF24L01_Send();
			switch(NRF24L01_Send()){
				case 0:
					NRF24L01_Send_Error[0]++;
				break;
				case 1:
					NRF24L01_Send_Error[1]++;
				break;
				case 2:
					NRF24L01_Send_Error[2]++;
				break;
				case 3:
					NRF24L01_Send_Error[3]++;
				break;
				case 4:
					NRF24L01_Send_Error[4]++;
				break;
			}
		}
		    if (NRF24L01_Receive() == 1)
    { 
			if((NRF24L01_RxPacket[0] * 256 + NRF24L01_RxPacket[1])>0&&(NRF24L01_RxPacket[0] * 256 + NRF24L01_RxPacket[1])<=5000&&(NRF24L01_RxPacket[0] * 256 + NRF24L01_RxPacket[1])!=2056){
				
			ADzhi11 = NRF24L01_RxPacket[0] * 256 + NRF24L01_RxPacket[1];
			}
			if((NRF24L01_RxPacket[2] * 256 + NRF24L01_RxPacket[3])>0&&(NRF24L01_RxPacket[2] * 256 + NRF24L01_RxPacket[3])<=5000&&(NRF24L01_RxPacket[2] * 256 + NRF24L01_RxPacket[3])!=2056){
			ADzhi12 = NRF24L01_RxPacket[2] * 256 + NRF24L01_RxPacket[3];
			}
			if((NRF24L01_RxPacket[4] * 256 + NRF24L01_RxPacket[5])>0&&(NRF24L01_RxPacket[4] * 256 + NRF24L01_RxPacket[5])<=5000&&(NRF24L01_RxPacket[4] * 256 + NRF24L01_RxPacket[5])!=2056){
			ADzhi22 = NRF24L01_RxPacket[4] * 256 + NRF24L01_RxPacket[5];
			}
			if((NRF24L01_RxPacket[6] * 256 + NRF24L01_RxPacket[7])>0&&(NRF24L01_RxPacket[6] * 256 + NRF24L01_RxPacket[7])<=5000&&(NRF24L01_RxPacket[6] * 256 + NRF24L01_RxPacket[7])!=2056){
			ADzhi21 = NRF24L01_RxPacket[6] * 256 + NRF24L01_RxPacket[7];
			}
			My_ADzhi12=0.4*ADzhi12+0.6*Last_ADzhi12;
			Last_ADzhi12=My_ADzhi12;
			
			My_ADzhi21=0.4*ADzhi21+0.6*Last_ADzhi21;
			Last_ADzhi21=My_ADzhi21;	
			
      anjianzhi[0] = (NRF24L01_RxPacket[8] >> 6) & 0x01;
      anjianzhi[1] = (NRF24L01_RxPacket[8] >> 5) & 0x01;
      anjianzhi[2] = (NRF24L01_RxPacket[8] >> 4) & 0x01;
      anjianzhi[3] = (NRF24L01_RxPacket[8] >> 3) & 0x01;
			if(anjianzhi[0]==1){
//			again=1;
			}
			if(anjianzhi[2]==1){
			uptMove=2;
			}
			else if(anjianzhi[3]==1){
			uptMove=1;
			}			
      anjianzhi[4] = (NRF24L01_RxPacket[8] >> 1) & 0x03;
      anjianzhi[5] = (NRF24L01_RxPacket[8]) & 0x01;
    }
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}
void TuoLuoYi_Task(){
	GyroRawData_t recv_data;  // 任务内缓存，接收队列数据
//	xGyroQueue = xQueueCreate(2, sizeof(GyroRawData_t));
    for (;;) {
        // -------------------------- 核心：任务中接收队列数据 --------------------------
        // xQueueReceive：阻塞式接收，队列无数据时任务休眠，不占用CPU
        // 第三个参数：阻塞超时时间（pdMS_TO_TICKS(100) = 100ms）
        if (xQueueReceive(xGyroQueue, &recv_data, pdMS_TO_TICKS(100)) == pdPASS) {
            
            // 你的原有姿态角计算逻辑（改用任务内的recv_data，无并发风险）
            Roll = (float)((int16_t)recv_data.data[1] << 8 | recv_data.data[0]) / 32768 * 180;
            Pitch = (float)((int16_t)recv_data.data[3] << 8 | recv_data.data[2]) / 32768 * 180;
            
            // Yaw范围校验
            float temp_yaw = (float)((int16_t)recv_data.data[5] << 8 | recv_data.data[4]) / 32768 * 180;
            if (temp_yaw >= 0 && temp_yaw <= 360) {
                Yaw = temp_yaw;
            }
            
            // 累计Yaw计算
            if (First_Yaw_flag == 0) {
                First_Yaw = Yaw;
                First_Yaw_flag = 1;
            }
            float Delta_Yaw = Yaw - Last_Yaw;
            if (Delta_Yaw > 180.0f) {
                Yaw_offset -= 360.0f;
                Quan_Shu--;
            } else if (Delta_Yaw < -180.0f) {
                Yaw_offset += 360.0f;
                Quan_Shu++;
            }
            My_Yaw = Yaw + Yaw_offset;
            Last_Yaw = Yaw;
            
            // 调试输出（按需保留）
            // printf("%f,%f,%f\r\n", My_Yaw, Roll, Pitch);
        }
        // 任务让出CPU（短延时，避免任务空转）
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) //狗子函数
{
//    OLED_Clear();
    // 判断任务名，显示不同数字
    if(strcmp(pcTaskName, "ESP8266_Task") == 0){
				OLED_ShowString(2,5,"ESP8266_Error!!!");
//        OLED_ShowNum(2,1,55,2); // ESP8266_Task溢出显示55
    }else if(strcmp(pcTaskName, "IIC_Task") == 0){
			OLED_ShowString(2,5,"IIC_Error!!!");
//        OLED_ShowNum(2,1,66,2); // IIC_Task溢出显示66
    }else if(strcmp(pcTaskName, "LED_1Task") == 0){
			OLED_ShowString(2,5,"OLED_Error!!!");
//				OLED_ShowNum(2,1,77,2); // OLED_Task溢出显示77
		}else if(strcmp(pcTaskName, "anjian_Task") == 0){
			OLED_ShowString(2,5,"Button_Error!!!");
//				OLED_ShowNum(2,1,88,2); // anjian_Task溢出显示
		}else{
			OLED_ShowString(2,5,"_Error!!!");
//        OLED_ShowNum(2,1,99,2); // 其他任务溢出显示99
    }
//    printf("【栈溢出】任务：%s\r\n", pcTaskName);
    for(;;); // 卡死系统，方便观察OLED提示
}
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
//  xTaskCreate(OLED_Task,"LED_1Task",256,&OLED_Test1,osPriorityNormal,&OLED_Task_Handle);
//	xTaskCreate(OLED_Task,"LED_2Task",128,&OLED_Test2,osPriorityNormal,NULL);
	xTaskCreate(ESP8266_Task,"ESP8266_Task",1024,NULL,osPriorityNormal,NULL);
	xTaskCreate(anjian_Task,"anjian_Task",128,NULL,osPriorityNormal,NULL);
	xTaskCreate(IIC_Task,"IIC_Task",512,NULL,osPriorityNormal,NULL);
	xTaskCreate(game1_task, "GameTask", 128, NULL, osPriorityNormal, NULL);
	xTaskCreate(NRF_Task,"NRF_Task",512,NULL,osPriorityNormal,NULL);
	xTaskCreate(TuoLuoYi_Task,"TuoLuoYi_Task",512,NULL,osPriorityNormal,NULL);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
		HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_9);	
		
		vTaskDelay(pdMS_TO_TICKS(500));	 // �ȴ�100ms
		
//    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

