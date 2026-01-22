/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "NRF24L01.h"
#include "MY_IIC.h"
#include "BMP280.h"
#include "ath20.h"
#include "stdio.h"
#include "esp8266.h"
#include "onenet.h"
#include <stdlib.h>
#include "math.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define LK 40
#define RK 70
#define gethui2() Button_Flag[0]
#define gethui3() Button_Flag[1]
#define gethui4() Button_Flag[2]
#define gethui5() Button_Flag[3]
#define gethui6() Button_Flag[4]
#define gethui7() Button_Flag[5]
#define gethui8() Button_Flag[6]
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
typedef struct{
	float kp,ki,kd;
	float error,lastError;
	float integral,maxintegral;
	float output,maxoutput;
}PID;
PID pid_Speed_Z;
PID pid_Speed_Y;
PID pid_Speed_WeiZhi;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void anjianlx();
void MY_SCAN();
int8_t return_to_expectation(void);
void MY_Run(unsigned int mode);
void PID_Yunxing(PID *pid, float mubiao, float fankuizhi);
void PID_Init(PID *pid, float p, float i, float d, float maxI, float maxOut);
void PID_Yunxing_Weizhi(PID *pid, float mubiao, float fankuizhi);
void JsonValue();
void Set_DianJi(unsigned char dianji,unsigned char fangxiang,unsigned int PWM_VAL);


unsigned int PWM_1=0,PWM_2=0;
uint8_t ret = 0;
const char devPubTopic[] = "$sys/nE4HArK3N3/Pena/thing/property/post";
const char *devSubTopic[] = {"$sys/nE4HArK3N3/Pena/thing/property/set"};
unsigned char *dataPtr = NULL;
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
unsigned int count = 0, jishi_count = 0;
unsigned int bici = 50;
float PT, T, ALT;
unsigned char Send_Flag = 1, Ce_liang_Flag = 0;
uint32_t CT_data[2];
int c1, t1;
uint8_t status = 0;
unsigned int ADzhi11,My_ADzhi11,Last_ADzhi11;
unsigned int ADzhi12,My_ADzhi12,Last_ADzhi12;
unsigned int ADzhi21,My_ADzhi21,Last_ADzhi21;
unsigned int ADzhi22,My_ADzhi22,Last_ADzhi22;
unsigned char anjianzhi[6];
unsigned char p3Data,p5Data,Error_Count=0,p2Data;
char PUBLIS_BUF[256];
unsigned int TimeCount;
float Roll,Pitch,Yaw,My_Yaw,Delta_Yaw,Last_Yaw,Yaw_offset,First_Yaw,MuBiao_JiaoDu=0;
int First_Yaw_flag=0;
unsigned int PWM_1_Dier,PWM_2_Dier;
extern unsigned int WiFi_Success_Flag;
extern char Rx_Data[20];
unsigned int PWM_Debug_1=1000,PWM_Debug_2=1000,Dian_Ji_KaiQi=0,MY_Dianji_Debug=0,PWM_Debug_Dier_1,PWM_Debug_Dier_2,Dian_Ji_KaiQi_anjian=0;
int Diretion_Y=0,Diretion_Z=0,MY_Counter_Y=0,MY_Counter_Z=0;
unsigned int val,down,old;
unsigned char Button_Flag[10]; // 0到6循迹
unsigned int fangxiang,PWM_1_24G,PWM_2_24G;
int VAL_DATA,Quan_Shu=0;
int MUBIAO_Speed_Z,MUBIAO_Speed_Y;
unsigned int Shang_Yun_Flag=1,Shang_Yun_Run_Flag=0;
unsigned int Test_LED_Count = 0;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_TIM6_Init();
  MX_USART3_UART_Init();
  MX_UART5_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM5_Init();
  /* USER CODE BEGIN 2 */
	HAL_UART_Receive_IT(&huart3, &p3Data, 1);
	HAL_UART_Receive_IT(&huart5, &p5Data, 1);
	HAL_UART_Receive_IT(&huart2, &p2Data, 1);
	HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
	HAL_TIM_Encoder_Start(&htim5, TIM_CHANNEL_ALL);
  NRF24L01_Init();

	if(Shang_Yun_Flag==1){
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
	ESP8266_Init();
	if(WiFi_Success_Flag==1){
			while(OneNet_DevLink())//连接Onenet平台,如果失败等待200ms继续尝试。
		{
			HAL_Delay(200);
			Error_Count++;
			if(Error_Count>=5){
				printf("连接失败\r\n");
			return 0;
			}
		}	
	}
	else{
				printf("连接失败\r\n");
	}
			/*订阅主题*/
	OneNet_Subscribe(devSubTopic,1);
	}

  HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); //
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2); //	
	PID_Init(&pid_Speed_Z, 15, 0.1, 0.05, 200, 450);
	PID_Init(&pid_Speed_Y, 15, 0.1, 0.05, 200, 450);
	PID_Init(&pid_Speed_WeiZhi,50, 0, 0.01, 0, 400);
	__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, 1000);
	__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, 1000);
	
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		
		//循迹
		MY_SCAN();
		VAL_DATA = return_to_expectation();
		MY_Run(Dian_Ji_KaiQi|Dian_Ji_KaiQi_anjian);
		
		HAL_UART_Receive_IT(&huart3, &p3Data, 1);
		HAL_UART_Receive_IT(&huart2, &p2Data, 1);
		HAL_UART_Receive_IT(&huart5, &p5Data, 1);
		
		if(RX_Re_Data() == 1){   //陀螺仪处理
		Roll = ( float)((int16_t)Rx_Data[1] << 8 | Rx_Data[0]) / 32768 * 180;
		Pitch = (float)((int16_t)Rx_Data[3] << 8 | Rx_Data[2]) / 32768 * 180;
			if(((float)((int16_t)(Rx_Data[5]) << 8 | Rx_Data[4]) / 32768 * 180)<=360&&((float)((int16_t)(Rx_Data[5]) << 8 | Rx_Data[4]) / 32768 * 180)>=0){
		Yaw = (float)((int16_t)(Rx_Data[5]) << 8 | Rx_Data[4]) / 32768 * 180;			
			}
		if(First_Yaw_flag==0){
		First_Yaw=Yaw;
			First_Yaw_flag=1;
		}	
		Delta_Yaw=Yaw-Last_Yaw;
		if(Delta_Yaw>180.0f){
			Yaw_offset-=360.0f;
			Quan_Shu--;
		}	
		else if(Delta_Yaw<-180.0f){
			Yaw_offset+=360.0f;
			Quan_Shu++;
		}
		My_Yaw=Yaw+Yaw_offset;
		Last_Yaw=Yaw;
//		printf("%f,%f,%f\r\n",My_Yaw,Roll,Pitch);
    Test_LED_Count++;
    if (Test_LED_Count % bici == 0)
    {
      HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_2); //陀螺仪正常执行现象
    }
	}

    if (Send_Flag == 1)
    {
      Send_Flag = 1;
      NRF24L01_TxPacket[0] = t1;
      NRF24L01_TxPacket[1]++;
      NRF24L01_TxPacket[2]++;
      NRF24L01_TxPacket[3]++;
      NRF24L01_Send();
    } 
		//下面注意控制时间
		
		if(Shang_Yun_Flag==1){
				if(++TimeCount >= 400)
				{
					Shang_Yun_Run_Flag=1;
					
					JH_Read_CTdata(CT_data);                        
					c1 = CT_data[0] * 1000 / 1024 / 1024;           
					t1 = CT_data[1] * 200 * 10 / 1024 / 1024 - 500; 
					BMP280GetData(&PT, &T, &ALT);
					
					JsonValue();
					OneNet_Publish(devPubTopic, PUBLIS_BUF);
					ESP8266_Clear();
					TimeCount = 0;
					Shang_Yun_Run_Flag=2;
				}
				dataPtr = ESP8266_GetIPD(1);
				if(dataPtr != NULL)
				OneNet_RevPro(dataPtr);	
				Shang_Yun_Run_Flag=0;		
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
      anjianzhi[4] = (NRF24L01_RxPacket[8] >> 1) & 0x03;
      anjianzhi[5] = (NRF24L01_RxPacket[8]) & 0x01;
			if(anjianzhi[0]==1){
			Dian_Ji_KaiQi_anjian=0;
			}
//			printf("%d,%d,%d,%d,%d,%d,%d,%d\r\n",My_ADzhi12,My_ADzhi21,anjianzhi[0],anjianzhi[1],anjianzhi[2],anjianzhi[3],anjianzhi[4],anjianzhi[5]);
    }
//			printf("%d,%d\n",My_ADzhi12,My_ADzhi21);
//		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_2);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	static unsigned int anjian_Count=0;
  if (htim == (&htim2))  //1KHZ
  {
		if(anjianzhi[4]==1){ //0正常
		if(anjianzhi[5]==1){
		Dian_Ji_KaiQi=1;
		}		
		else if(anjianzhi[5]==0){
		Dian_Ji_KaiQi=0;
		}
		}
		else if(anjianzhi[4]==2){ //遥控模式
		Dian_Ji_KaiQi=2;
		MY_Dianji_Debug=0;			
		}
		else if(anjianzhi[4]==0){
		Dian_Ji_KaiQi=0;
		}
		if(anjianzhi[1]==1){
		MY_Dianji_Debug=2;
		}
		else if(anjianzhi[1]==0){
		MY_Dianji_Debug=0;
		}
		switch(anjianzhi[4]){  //遥控按键状态 控制小车灯的状态
			case 0:
		if(anjianzhi[2]==1){
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_9,0);
		}
		else{
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_9,1);
		}
		if(anjianzhi[3]==1){
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_11,0);
		}
		else{
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_11,1);
		}					
			break;
			case 1:
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_9,0);
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_11,0);	
			break;
			case 2:
		anjian_Count++;
		if(anjian_Count%500==0){
		HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_9);
		HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_11);	
		}		
			break;
		}
    // while(ATH20_Read_Cal_Enable() == 0)
    // {
    //     ATH20_Init();//
    //     HAL_Delay(30);
    // }
		    jishi_count++;
		if(jishi_count%30==0){  //编码器
			Diretion_Y = __HAL_TIM_IS_TIM_COUNTING_DOWN(&htim4);  //you右轮子
			Diretion_Z = -(__HAL_TIM_IS_TIM_COUNTING_DOWN(&htim5)); //zuo左边轮子
			if(__HAL_TIM_GET_COUNTER(&htim4)>=30000){
			MY_Counter_Y = 65535-(__HAL_TIM_GET_COUNTER(&htim4));
			}
			else{
			MY_Counter_Y = __HAL_TIM_GET_COUNTER(&htim4);
			}	
			MY_Counter_Z = __HAL_TIM_GET_COUNTER(&htim5);
			__HAL_TIM_SET_COUNTER(&htim4, 0);
			__HAL_TIM_SET_COUNTER(&htim5, 0);
		}
		if(jishi_count%10==0){
			anjianlx();
			if(down==1){
//				if(anjianzhi[4]==0){ //运行模式
				MY_Dianji_Debug=0;
				Dian_Ji_KaiQi_anjian=1;
//				}
			}
			if(down==2){
//				if(anjianzhi[4]==0){
				MY_Dianji_Debug=0;
				Dian_Ji_KaiQi_anjian=0;
				
//				}
			}
		}
		if(MY_Dianji_Debug==1){    //速度环测试
		PID_Yunxing(&pid_Speed_Z, MUBIAO_Speed_Z,abs(MY_Counter_Z));
		PID_Yunxing(&pid_Speed_Y, MUBIAO_Speed_Y,abs(MY_Counter_Y));
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,PWM_Debug_Dier_1);		
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,PWM_Debug_Dier_2);				
			__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, 1000-pid_Speed_Y.output);
			__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, 1000-pid_Speed_Z.output);
		}   
		else if(MY_Dianji_Debug==2){  //方向环测试
		PID_Yunxing_Weizhi(&pid_Speed_WeiZhi,MuBiao_JiaoDu,My_Yaw);
			if(pid_Speed_WeiZhi.output>=0){
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,0);
			}
			else{
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,1);
			}
			__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, 1000-fabsf(pid_Speed_WeiZhi.output));
		}
		else if(MY_Dianji_Debug==3){
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,PWM_Debug_Dier_1);		
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,PWM_Debug_Dier_2);				
		__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1,1000-MUBIAO_Speed_Y);
		__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2,1000-MUBIAO_Speed_Z);		
		}
  }
}

void Set_DianJi(unsigned char dianji,unsigned char fangxiang,unsigned int PWM_VAL){  
	//1,2 1,2 0-1000
	switch(dianji){
	case 1:
		if(fangxiang==0){
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,0);		
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,0);	
		}
		else if(fangxiang==1){
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,1);		
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,1);		
		}
		__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, PWM_VAL);
	break;
	case 2:
		if(fangxiang==0){
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,0);		
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,0);	
		}
		else if(fangxiang==1){
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,1);		
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,1);	
		}
		__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, PWM_VAL);
	break;
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
void anjianlx(){
	val=anjianhq();
	down=val&(val^old);
	old=val;
}
void MY_Run(unsigned int mode){  //主程序
	switch(mode){
		case 0: //stop
		PWM_1=1000;
		PWM_2=1000;
		Set_DianJi(1,0,1000);
		Set_DianJi(2,0,1000);
		break;
		case 1: //循迹run
		PWM_1 = 850 - LK * VAL_DATA;
		PWM_2 = 820 + RK * VAL_DATA;
		if (PWM_1 >= 980)
		{
			PWM_1 = 980;
		}
		if (PWM_2 >= 980)
		{
			PWM_2 = 980;
		}
		if (PWM_1 <= 500)
		{
			PWM_1 = 500;
		}
		if (PWM_2 <= 500)
		{
			PWM_2 = 500;
		}
		Set_DianJi(1,0,PWM_1);
		Set_DianJi(2,0,PWM_2);		
		break;
		case 2: //2.4G遥控
			if(My_ADzhi12<=2500&&My_ADzhi12>=1500){ //停止
			fangxiang=0;	
			PWM_1_24G=0;
			PWM_2_24G=0;	
			}
			else if(My_ADzhi12>2500){  //加速
				fangxiang=0;
			PWM_1_24G=(My_ADzhi12-2500)*0.3125;
			PWM_2_24G=(My_ADzhi12-2500)*0.3125;			
			}
			else if(My_ADzhi12<1500){
				fangxiang=1;
			PWM_1_24G=(1500-My_ADzhi12)*0.3125;
			PWM_2_24G=(1500-My_ADzhi12)*0.3125;				
			}
			if(My_ADzhi21>=3500){ //左转
				PWM_2_24G=PWM_1_24G-200;
			}
			else if(My_ADzhi21<=500){ //右转
				PWM_2_24G=PWM_1_24G+200;
			}
			Set_DianJi(1,fangxiang,1000-PWM_1_24G);
			Set_DianJi(2,fangxiang,1000-PWM_2_24G);	
		break;
		case 3:  //Debug控制模式
			Set_DianJi(1,PWM_Debug_Dier_1,1000-MUBIAO_Speed_Z);
			Set_DianJi(2,PWM_Debug_Dier_2,1000-MUBIAO_Speed_Y);		
		break;
	}
}
void MY_SCAN()
{
	if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_15) == 0)
	{
		Button_Flag[0] = 0;
	}
	else
	{
		Button_Flag[0] = 1;
	}
	if (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_0) == 0)
	{
		Button_Flag[1] = 0;
	}
	else
	{
		Button_Flag[1] = 1;
	}
	if (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_1) == 0)
	{
		Button_Flag[2] = 0;
	}
	else
	{
		Button_Flag[2] = 1;
	}
	if (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_4) == 0)
	{
		Button_Flag[3] = 0;
	}
	else
	{
		Button_Flag[3] = 1;
	}
	if (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_5) == 0)
	{
		Button_Flag[4] = 0;
	}
	else
	{
		Button_Flag[4] = 1;
	}
	if (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_6) == 0)
	{
		Button_Flag[5] = 0;
	}
	else
	{
		Button_Flag[5] = 1;
	}
	if (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_7) == 0)
	{
		Button_Flag[6] = 0;
	}
	else
	{
		Button_Flag[6] = 1;
	}
}
int8_t return_to_expectation(void)
{
	int8_t temp_expectation = 0;
	if ((!gethui2()) && (!gethui3()) && (!gethui4()) && (gethui5()) && (!gethui6()) && (!gethui7()) && (!gethui8())) // 居中
	{
		temp_expectation = 0;
	}
	else if ((!gethui2()) && (!gethui3()) && (gethui4()) && (gethui5()) && (!gethui6()) && (!gethui7()) && (!gethui8())) // 正期望
	{
		temp_expectation = 1;
	}
	else if ((!gethui2()) && (!gethui3()) && (gethui4()) && (!gethui5()) && (!gethui6()) && (!gethui7()) && (!gethui8())) // 正期望
	{
		temp_expectation = 2;
	}
	else if ((!gethui2()) && (gethui3()) && (gethui4()) && (!gethui5()) && (!gethui6()) && (!gethui7()) && (!gethui8())) // 正期望
	{
		temp_expectation = 3;
	}
	else if ((!gethui2()) && (gethui3()) && (!gethui4()) && (!gethui5()) && (!gethui6()) && (!gethui7()) && (!gethui8())) // 正期望
	{
		temp_expectation = 4;
	}
	else if ((gethui2()) && (gethui3()) && (!gethui4()) && (!gethui5()) && (!gethui6()) && (!gethui7()) && (!gethui8())) // 正期望
	{
		temp_expectation = 5;
	}
	else if ((gethui2()) && (!gethui3()) && (!gethui4()) && (!gethui5()) && (!gethui6()) && (!gethui7()) && (!gethui8())) // 正期望
	{
		temp_expectation = 6;
	}
	else if ((!gethui2()) && (!gethui3()) && (!gethui4()) && (gethui5()) && (gethui6()) && (!gethui7()) && (!gethui8())) // 负期望
	{
		temp_expectation = -1;
	}
	else if ((!gethui2()) && (!gethui3()) && (!gethui4()) && (!gethui5()) && (gethui6()) && (!gethui7()) && (!gethui8())) // 负期望
	{
		temp_expectation = -2;
	}
	else if ((!gethui2()) && (!gethui3()) && (!gethui4()) && (!gethui5()) && (gethui6()) && (gethui7()) && (!gethui8())) // 负期望
	{
		temp_expectation = -3;
	}
	else if ((!gethui2()) && (!gethui3()) && (!gethui4()) && (!gethui5()) && (!gethui6()) && (gethui7()) && (!gethui8())) // 负期望
	{
		temp_expectation = -4;
	}
	else if ((!gethui2()) && (!gethui3()) && (!gethui4()) && (!gethui5()) && (!gethui6()) && (gethui7()) && (gethui8())) // 负期望
	{
		temp_expectation = -5;
	}
	else if ((!gethui2()) && (!gethui3()) && (!gethui4()) && (!gethui5()) && (!gethui6()) && (!gethui7()) && (gethui8())) // 负期望
	{
		temp_expectation = -6;
	} // 以上为直线循迹

	else if ((!gethui2()) && (!gethui3()) && (!gethui4()) && (!gethui5()) && (gethui6()) && (gethui7()) && (gethui8())) // 负期望
	{
		temp_expectation = -9;
	}
	else if ((!gethui2()) && (!gethui3()) && (!gethui4()) && (gethui5()) && (gethui6()) && (gethui7()) && (gethui8())) // 负期望
	{
		temp_expectation = -9;
	}
	else if ((!gethui2()) && (!gethui3()) && (gethui4()) && (gethui5()) && (gethui6()) && (gethui7()) && (gethui8())) // 负期望
	{
		temp_expectation = -9;
	}
	else if ((!gethui2()) && (gethui3()) && (gethui4()) && (gethui5()) && (gethui6()) && (gethui7()) && (gethui8())) // 负期望
	{
		temp_expectation = -9;
	}
	else if ((gethui2()) && (gethui3()) && (gethui4()) && (gethui5()) && (gethui6()) && (gethui7()) && (gethui8())) // 负期望
	{
		temp_expectation = 9;
	}
	else if ((gethui2()) && (gethui3()) && (gethui4()) && (gethui5()) && (gethui6()) && (gethui7()) && (!gethui8())) // 负期望
	{
		temp_expectation = 9;
	}
	else if ((gethui2()) && (gethui3()) && (gethui4()) && (gethui5()) && (gethui6()) && (!gethui7()) && (!gethui8())) // 负期望
	{
		temp_expectation = 9;
	}
	else if ((gethui2()) && (gethui3()) && (gethui4()) && (gethui5()) && (!gethui6()) && (!gethui7()) && (!gethui8())) // 负期望
	{
		temp_expectation = 9;
	}
		else if ((gethui2()) && (gethui3()) && (gethui4()) && (!gethui5()) && (!gethui6()) && (!gethui7()) && (!gethui8())) // 负期望
	{
		temp_expectation = 9;
	}
	else if ((!gethui2()) && (!gethui3()) && (!gethui4()) && (!gethui5()) && (!gethui6()) && (!gethui7()) && (!gethui8()))
	{
		temp_expectation = 10;
	}
	// 最后一种没线  全白 //下面拿10作为直角判据（100ms内一直是状态10）
	//	else{
	//		temp_expectation=10;
	//	}
	return temp_expectation;
}
void PID_Init(PID *pid, float p, float i, float d, float maxI, float maxOut)
{
	pid->kp = p;
	pid->ki = i;
	pid->kd = d;
	pid->maxintegral = maxI;
	pid->maxoutput = maxOut;
}

void PID_Yunxing(PID *pid, float mubiao, float fankuizhi)
{
	pid->lastError = pid->error;
	pid->error = mubiao - fankuizhi;
	// 计算微分D
	float Dout = (pid->error - pid->lastError) * pid->kd;
	// 计算比例P
	float Pout = (pid->error) * pid->kp;
	// 计算积分D
	pid->integral += pid->error * pid->ki;
	// 积分限幅
	if (pid->integral > pid->maxintegral)
	{
		pid->integral = pid->maxintegral;
	}
	else if (pid->integral < -pid->maxintegral)
	{
		pid->integral = -pid->maxintegral;
	}
	// out计算
	pid->output = Dout + Pout + pid->integral;
	// 输出限幅
	if (pid->output > pid->maxoutput)
	{
		pid->output = pid->maxoutput;
	}
	else if (pid->output < -pid->maxoutput)
	{
		pid->output = 0;
	}
}
void PID_Yunxing_Weizhi(PID *pid, float mubiao, float fankuizhi)
{
	pid->lastError = pid->error;
	pid->error = mubiao - fankuizhi;
	// 计算微分D
	float Dout = (pid->error - pid->lastError) * pid->kd;
	// 计算比例P
	float Pout = (pid->error) * pid->kp;
	// 计算积分D
	pid->integral += pid->error * pid->ki;
	// 积分限幅
	if (pid->integral > pid->maxintegral)
	{
		pid->integral = pid->maxintegral;
	}
	else if (pid->integral < -pid->maxintegral)
	{
		pid->integral = -pid->maxintegral;
	}
	// out计算
	pid->output = Dout + Pout + pid->integral;
	// 输出限幅
	if (pid->output > pid->maxoutput)
	{
		pid->output = pid->maxoutput;
	}
	else if (pid->output < -pid->maxoutput)
	{
		pid->output = -pid->maxoutput;
	}
}
void JsonValue()
{
//	uint8_t Temp = c1;
	uint8_t Hum = t1/10;
	float qiya=PT;
	uint8_t shidu=c1/10;
	
	memset(PUBLIS_BUF, 0, sizeof(PUBLIS_BUF));
	
	sprintf(PUBLIS_BUF,"{\"id\":\"123\",\"params\":{\"Temp\":{\"value\":%d},\"Qiya\":{\"value\":%.1f},\"ShiDu\":{\"value\":%d}}}",
					Hum,qiya,shidu);	
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
