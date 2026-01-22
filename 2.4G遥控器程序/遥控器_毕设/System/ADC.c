#include "stm32f10x.h"                  // Device header


void MyADC_Init(){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitTypeDef GPISTRUCE;
	GPISTRUCE.GPIO_Mode=GPIO_Mode_AIN;
	GPISTRUCE.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
	GPISTRUCE.GPIO_Speed=GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOA,&GPISTRUCE);

	ADC_InitTypeDef ADCSTRUCE;
	
	ADCSTRUCE.ADC_ContinuousConvMode=DISABLE;
	ADCSTRUCE.ADC_DataAlign=ADC_DataAlign_Right;
	ADCSTRUCE.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None;
	ADCSTRUCE.ADC_Mode=ADC_Mode_Independent;
	ADCSTRUCE.ADC_NbrOfChannel=1;
	ADCSTRUCE.ADC_ScanConvMode=DISABLE;
	
	ADC_Init(ADC1,&ADCSTRUCE);
	
	ADC_Cmd(ADC1,ENABLE);
	
	ADC_ResetCalibration(ADC1);								//固定流程，内部有电路会自动执行校准
	while (ADC_GetResetCalibrationStatus(ADC1) == SET);
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1) == SET);
	
}

uint16_t AD_GetVal(unsigned char ADC_Channel){

	ADC_RegularChannelConfig(ADC1, ADC_Channel, 1, ADC_SampleTime_55Cycles5);	//在每次转换前，根据函数形参灵活更改规则组的通道1
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);					//软件触发AD转换一次
	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);	//等待EOC标志位，即等待AD转换结束
	return ADC_GetConversionValue(ADC1);			
	
	
}
