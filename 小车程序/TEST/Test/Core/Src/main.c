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
#include "OLED.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// 功能宏定义（原程序用到的）
#define LK 40
#define RK 70
#define UPLOAD_INTERVAL_MS  500  // 保留：原程序定义过
// 循迹传感器读取宏
#define gethui2() Button_Flag[0]
#define gethui3() Button_Flag[1]
#define gethui4() Button_Flag[2]
#define gethui5() Button_Flag[3]
#define gethui6() Button_Flag[4]
#define gethui7() Button_Flag[5]
#define gethui8() Button_Flag[6]
// ==================== 【第一步：把这两个变量定义放在 main() 函数外面】 ====================
#define DELTA_TRESHOLD   5000      // 限幅阈值：单次变化超过200认为是脏数据
#define SMOOTH_FACTOR    0.70f    // 平滑系数：越小越丝滑 (0.05~0.3)

// PID控制器结构体（原程序核心用到）
typedef struct{
	float kp,ki,kd;
	float error,lastError;
	float integral,maxintegral;
	float output,maxoutput;
}PID;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// ====================== PID相关（核心用到） ======================
PID pid_Speed_Z;
PID pid_Speed_Y;
PID pid_Speed_WeiZhi;

// ====================== 硬件/串口相关（用到） ======================
uint8_t p3Data = 0;    // USART3接收数据（中断用到）
uint8_t p5Data = 0;    // UART5接收数据（中断用到）
uint8_t p2Data = 0;    // USART2接收数据（中断用到）
uint8_t ret = 0;       // 传感器初始化返回值（用到）

// ====================== 电机/控制相关（用到） ======================
unsigned int PWM_1=0,PWM_2=0;                // 电机PWM值（核心用到）
unsigned char Button_Flag[10] = {0};         // 循迹传感器状态（核心用到）
uint8_t Dian_Ji_KaiQi=0;                     // 电机使能（遥控/强制）（用到）
uint8_t MY_Dianji_Debug=0;                   // PID调试模式（用到）
unsigned int PWM_Debug_Dier_1=0,PWM_Debug_Dier_2=0; // 调试电机方向（用到）
uint8_t Dian_Ji_KaiQi_anjian=0;              // 电机使能（按键）（用到）
int8_t VAL_DATA = 0;                         // 循迹偏差值（核心用到）
unsigned int fangxiang=0;                    // 电机方向（用到）
unsigned int PWM_1_24G=0,PWM_2_24G=0;        // 2.4G遥控PWM值（用到）

// ====================== 2.4G遥控/NRF相关（原程序有赋值，保留） ======================
unsigned int ADzhi11,My_ADzhi11,Last_ADzhi11; // 恢复：原程序NRF接收赋值
unsigned int ADzhi12,My_ADzhi12,Last_ADzhi12; // 油门AD值（滤波，核心用到）
unsigned int ADzhi21,My_ADzhi21,Last_ADzhi21; // 方向AD值（滤波，核心用到）
unsigned int ADzhi22,My_ADzhi22,Last_ADzhi22; // 恢复：原程序NRF接收赋值
unsigned char anjianzhi[6] = {0};            // 遥控按键状态（核心用到）
unsigned int NRF24L01_Send_Error[5] = {0};   // NRF发送错误计数（用到）
static uint16_t last_valid_ad_final[4] = {0}; // 上一次最终输出的AD值
// ====================== 编码器相关（用到） ======================
int Diretion_Y=0,Diretion_Z=0;               // 编码器方向（用到）
int MY_Counter_Y=0,MY_Counter_Z=0;           // 编码器计数（用到）

// ====================== 陀螺仪相关（用到） ======================
extern char Rx_Data[20];                     // 陀螺仪接收数据（用到）
float Roll,Pitch,Yaw,My_Yaw;                 // 姿态角（核心用到）
float Delta_Yaw,Last_Yaw,Yaw_offset;         // 偏航角修正（用到）
float First_Yaw;                             // 初始偏航角（用到）
int First_Yaw_flag=0;                        // 初始偏航角标志（用到）
int Quan_Shu=0;                              // 偏航角圈数（用到）
float MuBiao_JiaoDu=0;                       // 目标角度（用到）
int MUBIAO_Speed_Z=0,MUBIAO_Speed_Y=0;       // PID目标速度（用到）

// ====================== 传感器相关（用到） ======================
float PT, T, ALT;                            // BMP280数据（函数传参用到，保留）
uint32_t CT_data[2];                         // ATH20原始数据（用到）
int c1, t1;                                  // 温湿度处理值（核心用到）
uint8_t status = 0;                          // 恢复：原程序定义过（编译需要）

// ====================== 通信/OneNet相关（用到） ======================
const char devPubTopic[] = "$sys/nE4HArK3N3/Pena/thing/property/post"; // 用到
const char *devSubTopic[] = {"$sys/nE4HArK3N3/Pena/thing/property/set"}; // 用到
unsigned char *dataPtr = NULL;               // ESP8266接收指针（用到）
extern unsigned int WiFi_Success_Flag;       // WiFi连接标志（用到）
uint8_t Error_Count=0;                       // OneNet连接错误计数（用到）
char PUBLIS_BUF[256] = {0};                  // OneNet发布缓存（核心用到）

// ====================== 计时/标志相关（原程序有赋值/用到，保留） ======================
unsigned int count = 0;                      // 恢复：原程序定义（编译需要）
unsigned int jishi_count = 0;                // 1KHz定时器计数（核心用到）
unsigned int TimeCount=0,TimeCount2=0,TimeCount3=0;       // 上传/发送计数（核心用到）
uint8_t Send_Flag = 1;                       // NRF发送标志（用到）
uint8_t Ce_liang_Flag = 0;                   // 恢复：原程序定义（编译需要）
unsigned int Test_LED_Count = 0;             // LED测试计数（用到）
uint8_t WhilE_Flag=0;                        // 主循环分段标记（用到）
unsigned int bici = 100;                     // LED闪烁周期（用到）
uint8_t Shang_Yun_Flag=1;                    // 上传使能标志（核心用到）
unsigned int Shang_Yun_Run_Flag=0;           // 恢复：原程序赋值（编译需要）
static uint32_t tick_count = 0;              // 恢复：原程序定义（编译需要）
static uint32_t last_send_tick = 0;          // 恢复：原程序定义（编译需要）
uint32_t last_upload_tick=0;                 // 恢复：原程序赋值（编译需要）
// 按键检测变量（用到）
unsigned int val=0,down=0,old=0;
static unsigned int anjian_Count=0;          // 灯光闪烁计数（用到）

// ====================== 调试相关（原程序定义，保留） ======================
unsigned int PWM_1_Dier=0,PWM_2_Dier=0;       // 恢复：原程序定义（编译需要）
unsigned int PWM_Debug_1=1000,PWM_Debug_2=1000; // 恢复：原程序定义（编译需要）
unsigned int *p;                             // 恢复：原程序定义（编译需要）
//extern unsigned char uptMove;
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
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
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
	OLED_Init();
//	OLED_ShowString(2,1,"WiFi Initing ...");
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
	last_upload_tick = HAL_GetTick(); // 记录上次上传的时间点
	}

  HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); //
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2); //	
	PID_Init(&pid_Speed_Z, 15, 0.1, 0.05, 200, 450);
	PID_Init(&pid_Speed_Y, 15, 0.1, 0.05, 200, 450);
	PID_Init(&pid_Speed_WeiZhi,50, 0, 0.01, 0, 400);
	__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, 1000);
	__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, 1000);
	
	Game_Vri_Init();
	OLED_Clear();
	OLED_ShowString(2,5,"Init_OK");
	HAL_Delay(800);
	OLED_Clear();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		
		//循迹
		WhilE_Flag=1;
		MY_SCAN();
		VAL_DATA = return_to_expectation();
		MY_Run(Dian_Ji_KaiQi|Dian_Ji_KaiQi_anjian);
		WhilE_Flag=2;
		HAL_UART_Receive_IT(&huart3, &p3Data, 1);
		HAL_UART_Receive_IT(&huart2, &p2Data, 1);
		HAL_UART_Receive_IT(&huart5, &p5Data, 1);
		WhilE_Flag=3;
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
	}
		    Test_LED_Count++;
    if (Test_LED_Count % bici == 0)
    {
      HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_2); //陀螺仪正常执行现象
    }
	WhilE_Flag=4;
    if (Send_Flag == 1)
    {
			if(++TimeCount2>=50){
			
			TimeCount2=0;
      Send_Flag = 1;
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
		}

		WhilE_Flag=5;
		//下面注意控制时间
		
		if(Shang_Yun_Flag==1){
			
    dataPtr = ESP8266_GetIPD(1); 
    if(dataPtr != NULL)
    {
        OneNet_RevPro(dataPtr);
        ESP8266_Clear(); // 【必须加】只有处理完接收才清空
    }
		
				if(++TimeCount >= 30)
				{
					Shang_Yun_Run_Flag=1;
					JH_Read_CTdata(CT_data);         //较慢               
					c1 = CT_data[0] * 1000 / 1024 / 1024;           
					t1 = CT_data[1] * 200 * 10 / 1024 / 1024 - 500; 
					BMP280GetData(&PT, &T, &ALT);
					
					JsonValue();
					OneNet_Publish(devPubTopic, PUBLIS_BUF); //非常慢
//					ESP8266_Clear();
					TimeCount = 0;
				}
		}
		WhilE_Flag=6;


// ==================== 【第二步：在 main() 的 while(1) 里用这段替换你原来的接收代码】 ====================
if (NRF24L01_Receive() == 1)
{ 
    // 只有帧头对了，才进里面处理数据
    if (NRF24L01_RxPacket[0] == 0x11)
    {
        // 1. 先提取所有原始数据
        uint16_t ad1 = (uint16_t)NRF24L01_RxPacket[0+1] * 256 + NRF24L01_RxPacket[1+1];
        uint16_t ad2 = (uint16_t)NRF24L01_RxPacket[2+1] * 256 + NRF24L01_RxPacket[3+1];
        uint16_t ad3 = (uint16_t)NRF24L01_RxPacket[4+1] * 256 + NRF24L01_RxPacket[5+1];
        uint16_t ad4 = (uint16_t)NRF24L01_RxPacket[6+1] * 256 + NRF24L01_RxPacket[7+1];

        // 2. ADzhi11 (丝滑滤波)
        if (ad1 > 0 && ad1 <= 5000 && ad1 != 2056)
        {
            int32_t delta = ad1 - last_valid_ad_final[0];
            if (delta >= -DELTA_TRESHOLD && delta <= DELTA_TRESHOLD)
            {
                ADzhi11 = (uint16_t)(SMOOTH_FACTOR * ad1 + (1.0f - SMOOTH_FACTOR) * last_valid_ad_final[0]);
                last_valid_ad_final[0] = ADzhi11;
            }
        }

        // 3. ADzhi12 (丝滑滤波 + 你的原有逻辑)
        if (ad2 > 0 && ad2 <= 5000 && ad2 != 2056)
        {
            int32_t delta = ad2 - last_valid_ad_final[1];
            if (delta >= -DELTA_TRESHOLD && delta <= DELTA_TRESHOLD)
            {
                ADzhi12 = (uint16_t)(SMOOTH_FACTOR * ad2 + (1.0f - SMOOTH_FACTOR) * last_valid_ad_final[1]);
                My_ADzhi12 = 0.4f * ADzhi12 + 0.6f * Last_ADzhi12;
                Last_ADzhi12 = My_ADzhi12;
                last_valid_ad_final[1] = ADzhi12;
            }
        }

        // 4. ADzhi22 (丝滑滤波)
        if (ad3 > 0 && ad3 <= 5000 && ad3 != 2056)
        {
            int32_t delta = ad3 - last_valid_ad_final[2];
            if (delta >= -DELTA_TRESHOLD && delta <= DELTA_TRESHOLD)
            {
                ADzhi22 = (uint16_t)(SMOOTH_FACTOR * ad3 + (1.0f - SMOOTH_FACTOR) * last_valid_ad_final[2]);
                last_valid_ad_final[2] = ADzhi22;
            }
        }

        // 5. ADzhi21 (丝滑滤波 + 你的原有逻辑)
        if (ad4 > 0 && ad4 <= 5000 && ad4 != 2056)
        {
            int32_t delta = ad4 - last_valid_ad_final[3];
            if (delta >= -DELTA_TRESHOLD && delta <= DELTA_TRESHOLD)
            {
                ADzhi21 = (uint16_t)(SMOOTH_FACTOR * ad4 + (1.0f - SMOOTH_FACTOR) * last_valid_ad_final[3]);
                My_ADzhi21 = 0.4f * ADzhi21 + 0.6f * Last_ADzhi21;
                Last_ADzhi21 = My_ADzhi21;
                last_valid_ad_final[3] = ADzhi21;
            }
        }

        // 6. 解析按键 (保持原样)
        anjianzhi[0] = (NRF24L01_RxPacket[9] >> 6) & 0x01;
        anjianzhi[1] = (NRF24L01_RxPacket[9] >> 5) & 0x01;
        anjianzhi[2] = (NRF24L01_RxPacket[9] >> 4) & 0x01;
        anjianzhi[3] = (NRF24L01_RxPacket[9] >> 3) & 0x01;
        anjianzhi[4] = (NRF24L01_RxPacket[9] >> 1) & 0x03;
        anjianzhi[5] = NRF24L01_RxPacket[9] & 0x01;
        
        bici = 30;
    }
    
    // 无论数据好坏，最后都清空一下FIFO，防止残留污染下一包
    NRF24L01_FlushRx(); 
}

		WhilE_Flag=7;
		if(++TimeCount3>=10){
		TimeCount3=0;
		game1_draw();
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
			if(anjianzhi[0]==1){
			Shang_Yun_Flag=0;
			}
			if(anjianzhi[0]==0){
			Shang_Yun_Flag=1;
			}
		if(Dian_Ji_KaiQi_anjian==1){
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_8,0);
		}
		else if(Dian_Ji_KaiQi_anjian==0){
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_8,1);
		}
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
//				uptMove=1;
				Dian_Ji_KaiQi_anjian=1;
//				}
			}
			if(down==2){
//				if(anjianzhi[4]==0){
				MY_Dianji_Debug=0;
//				uptMove=2;
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
	uint8_t Temp = c1;
	uint8_t Hum_D = t1/10;
	uint8_t Hum_X = t1%10;
	float Hum=Hum_D+((float)Hum_X/10);
//	uint8_t Hum = t1/10;
	float qiya=PT;
	uint8_t shidu=c1/10;
	if(Hum<=37&&Hum>=0){
	memset(PUBLIS_BUF, 0, sizeof(PUBLIS_BUF));
	
	sprintf(PUBLIS_BUF,"{\"id\":\"123\",\"params\":{\"temp_float\":{\"value\":%.1f},\"Qiya\":{\"value\":%.1f},\"ShiDu\":{\"value\":%d}}}",
					Hum,qiya,shidu);	
	}	
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
