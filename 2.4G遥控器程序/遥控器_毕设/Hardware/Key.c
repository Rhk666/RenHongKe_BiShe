#include "stm32f10x.h"                  // Device header
#include "Delay.h"
unsigned int val,down,old,up;
void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_14|GPIO_Pin_12|GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_WriteBit(GPIOB,GPIO_Pin_14|GPIO_Pin_5|GPIO_Pin_7|GPIO_Pin_6|GPIO_Pin_12|GPIO_Pin_13,(BitAction)0);
	
}

uint8_t Key_GetNum(void)
{
	uint8_t KeyNum = 0;
	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0)
	{
		Delay_ms(20);
		while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0);
		Delay_ms(20);
		KeyNum = 1;
	}
	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0)
	{
		Delay_ms(20);
		while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0);
		Delay_ms(20);
		KeyNum = 2;
	}
	
	return KeyNum;
}
unsigned char Getanjian(){
	unsigned char anjianzhi=0;
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_5)==1){
	anjianzhi=7;
	}
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_6)==1){
	anjianzhi=2;
	}
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_7)==1){
	anjianzhi=3;
	}
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14)==1){
	anjianzhi=4;
	}
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)==1){
	anjianzhi=5;
	}
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13)==1){
	anjianzhi=6;
	}
	return anjianzhi;
}

void hqanjian(){
	val=Getanjian();
	down=val&(val^old);
	up=~val&(val^old);
	old=val;
}
