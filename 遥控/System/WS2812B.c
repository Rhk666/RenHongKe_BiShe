#include "stm32f10x.h"                  // Device header
#include "Ws2812b.h"

#include "Delay.h"


#define CODE0 0xc0
#define CODE1 0xfc

//typedef struct{
//	unsigned char R;
//	unsigned char G;
//	unsigned char B;
//}RGB_Color;

const RGB_Color red={10,0,0};
const RGB_Color green={0,10,0};
const RGB_Color blue={0,0,10};
const RGB_Color yellow={10,10,0};
const RGB_Color black={0,0,0};
const RGB_Color white={10,10,10};
const RGB_Color purple={10,0,10};
unsigned int ceshisz[15]={
0,0,0,
2,2,2,
4,4,4,
6,6,6,
10,10,10
};
unsigned int All_LED[LED_NUM*3];


void MySPI_Init(){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef GPIOSTRUCE;
	GPIOSTRUCE.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIOSTRUCE.GPIO_Pin=GPIO_Pin_15;
	GPIOSTRUCE.GPIO_Speed=GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOB,&GPIOSTRUCE);
	
	SPI_InitTypeDef SPIOSTRUCE;
	SPIOSTRUCE.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_8;
	SPIOSTRUCE.SPI_CPHA=SPI_CPHA_1Edge;
	SPIOSTRUCE.SPI_CPOL=SPI_CPOL_High;
	SPIOSTRUCE.SPI_CRCPolynomial=7;
	SPIOSTRUCE.SPI_DataSize=SPI_DataSize_8b;
	SPIOSTRUCE.SPI_Direction=SPI_Direction_1Line_Tx;
	SPIOSTRUCE.SPI_FirstBit=SPI_FirstBit_MSB;
	SPIOSTRUCE.SPI_Mode=SPI_Mode_Master;
	SPIOSTRUCE.SPI_NSS=SPI_NSS_Soft;
	
	SPI_Init(SPI2,&SPIOSTRUCE);
	
	SPI_CalculateCRC(SPI2,DISABLE);
	
	SPI_Cmd(SPI2,ENABLE);

}

void SPI_SendOneBit(volatile unsigned char Data){
	while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_TXE)==RESET);
	switch(Data){
		case 0: SPI_I2S_SendData(SPI2,CODE0);  break;
		default :  SPI_I2S_SendData(SPI2,CODE1);  break;
	}

}
void Set_Color_On_Array(unsigned char LEDID,RGB_Color color){ //ID´Ó0¿ªÊ¼
	All_LED[LEDID*3]=color.G;
	All_LED[LEDID*3+1]=color.R;
	All_LED[LEDID*3+2]=color.B;
}

void Send_ALL_LED_Array(){
	for(unsigned int i=0;i<LED_NUM;i++){
		
	for(int j=7;j>=0;j--){
	SPI_SendOneBit(All_LED[i*3]&(1<<j));
	}
	for(int j=7;j>=0;j--){
	SPI_SendOneBit(All_LED[i*3+1]&(1<<j));
	}
	for(int j=7;j>=0;j--){
	SPI_SendOneBit(All_LED[i*3+2]&(1<<j));
	}
	}
}
void my_Send(){
	for(unsigned int i=0;i<LED_NUM;i++){
		
	for(int j=7;j>=0;j--){
	SPI_SendOneBit(ceshisz[(i)*3]&(1<<j));
	}
	for(int j=7;j>=0;j--){
	SPI_SendOneBit(ceshisz[(i)*3+1]&(1<<j));
	}
	for(int j=7;j>=0;j--){
	SPI_SendOneBit(ceshisz[(i)*3+2]&(1<<j));
	}
	}
}
