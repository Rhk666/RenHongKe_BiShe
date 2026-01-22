/*
Buffer[0] 帧头      0x5a
Buffer[1] 摇杆1(1)  data1
Buffer[2] 摇杆1(2)  data2
Buffer[3] 摇杆2(1)  data1
Buffer[4] 摇杆2(2)  data2
Buffer[5] 摇杆按键1 data1
Buffer[6] 摇杆按键2 data2
Buffer[7] 按键1     data1
Buffer[8] 按键2     data2
Buffer[9] 按键3     data3
Buffer[10]按键4     data4
Buffer[11]帧尾      0xa5
*/
#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "NRF24L01.h"

#include "key.h"
#include "ADC.h"
#include "WS2812B.h"
#include "stdlib.h"
#include "time.h"
#define N 1
#define yg111 SendBuf[1]
#define yg112 SendBuf[2]
#define yg121 SendBuf[3]
#define yg122 SendBuf[4]
#define yg211 SendBuf[5]
#define yg212 SendBuf[6]
#define yg221 SendBuf[7]
#define yg222 SendBuf[8]
#define ygaj1 SendBuf[9]
#define ygaj2 SendBuf[10]
#define aj1  SendBuf[11]
#define aj2  SendBuf[12]
#define aj3  SendBuf[13]
#define aj4  SendBuf[14]
unsigned int suiji=0;
unsigned int ADzhi11=0,runcnt=0,ADzhi12,ADzhi21,ADzhi22;
extern unsigned int val,down,old,up;
unsigned int downflag=0,downcount,countdeng;
unsigned int count=0,i=0;
unsigned int jishu=0;
unsigned int suijishu[4]={0};
unsigned char mysjs=0;
unsigned char anjian_state;

//unsigned int ceshijishu[10];
unsigned int timeshu=0;
unsigned int firstflag=0,firstflagcnt=0;
unsigned char suijishuzucount=0;
unsigned int mycountLED=0;
RGB_Color purple6={108,0,108};
RGB_Color purple5={60,0,60};
RGB_Color purple4={30,0,30};
RGB_Color purple3={15,0,15};
RGB_Color purple2={7,0,7};
RGB_Color purple1={4,0,4};
RGB_Color purple0={1,0,1};
RGB_Color myblack={0,0,0};
RGB_Color ceshikan1={1,5,15};
RGB_Color ceshikan2={10,5,1};
RGB_Color ceshikan3={5,10,1};
RGB_Color ceshikan4={5,5,5};
RGB_Color ceshikan5={10,50,150};

RGB_Color *ceshikan[5]={&ceshikan1,&ceshikan2,&ceshikan3,&ceshikan4,&ceshikan5};
uint8_t SendBuf[32]={0x5a,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0xa5},SendFlag;
void myLED_Init(void);
int main(void)
{
	myLED_Init();
	MyADC_Init();
	Key_Init();
	OLED_Init();
	TIM2_Init();
	NRF24L01_Init();
	MySPI_Init();
	suijishu[0]=(AD_GetVal(ADC_Channel_0)%10+AD_GetVal(ADC_Channel_1)%10+AD_GetVal(ADC_Channel_2)%10+AD_GetVal(ADC_Channel_3)%10);
	Set_Color_On_Array(0,*ceshikan[3]);
	Set_Color_On_Array(1,*ceshikan[0]);	
	Set_Color_On_Array(2,*ceshikan[1]);
	Set_Color_On_Array(3,*ceshikan[2]);	
		Send_ALL_LED_Array();
		Delay_us(10);
	OLED_ShowString(1,1,"Init_OK__Send");
	
	while (1)
	{
//	Set_Color_On_Array(0,purple6);
//	Set_Color_On_Array(1,purple4);	
//	Set_Color_On_Array(2,purple2);
//	Set_Color_On_Array(3,purple0);	
//	
//		Send_ALL_LED_Array();
//		Send(SendBuf);
		if(firstflag==0){
		firstflag=1;
		GPIO_WriteBit(GPIOA,GPIO_Pin_8,0);
		}
//		OLED_ShowHexNum(2,1,SendBuf[1],2);
//		OLED_ShowHexNum(2,4,SendBuf[2],2);
//		OLED_ShowHexNum(2,7,SendBuf[3],2);
//		OLED_ShowHexNum(2,10,SendBuf[4],2);
//		OLED_ShowHexNum(2,13,SendBuf[5],2);
//		OLED_ShowHexNum(3,1,SendBuf[6],2);
//		OLED_ShowHexNum(3,4,SendBuf[7],2);
//		OLED_ShowHexNum(3,7,SendBuf[8],2);
//		OLED_ShowHexNum(3,10,SendBuf[9],2);
//		OLED_ShowHexNum(3,13,SendBuf[10],2);
//		OLED_ShowHexNum(4,1,SendBuf[11],2);
//		OLED_ShowHexNum(4,4,SendBuf[12],2);
//		OLED_ShowHexNum(4,7,SendBuf[13],2);
//		OLED_ShowHexNum(4,10,SendBuf[14],2);
		yg111=AD_GetVal(ADC_Channel_0)/256;
		yg112=AD_GetVal(ADC_Channel_0)%256;
		ADzhi11=yg111*256+yg112;
		yg121=AD_GetVal(ADC_Channel_1)/256;
		yg122=AD_GetVal(ADC_Channel_1)%256;
		ADzhi12=yg121*256+yg122;		
		yg211=AD_GetVal(ADC_Channel_2)/256;
		yg212=AD_GetVal(ADC_Channel_2)%256;
		ADzhi21=yg211*256+yg212;
		yg221=AD_GetVal(ADC_Channel_3)/256;
		yg222=AD_GetVal(ADC_Channel_3)%256;
		ADzhi22=yg221*256+yg222;
			if(SendBuf[9]==1){ //|0100 0000 第一个
			anjian_state|=0x40;
			}
			else{              //&1011 1111
			anjian_state&=0xbf;			
			}
			if(SendBuf[10]==1){//|0010 0000 第二个
			anjian_state|=0x20;
			}
			else{              //&1101 1111
			anjian_state&=0xdf;			
			}
			if(SendBuf[12]==1){//|0001 0000 第三个
			anjian_state|=0x10;
			}
			else{              //&1110 1111
			anjian_state&=0xef;			
			}
			if(SendBuf[13]==1){//|0000 1000 第四个
			anjian_state|=0x08;
			}
			else{              //&1111 0111
			anjian_state&=0xf7;			
			}	
			if(SendBuf[11]==1){//&1111 1011  |0000 0010 第五个
			anjian_state&=0xfb;
			anjian_state|=0x02;	
			}
			else if(SendBuf[11]==2){//&1111 1101 |0000 0100 
			anjian_state&=0xfd;	
			anjian_state|=0x04;		
			}		
			else{              //&1111 1001
			anjian_state&=0xf9;
			}
			if(SendBuf[14]==1){//|0000 0001 第六个
			anjian_state|=0x01;
			}
			else{              //&1111 1110
			anjian_state&=0xfe;			
			}	
			NRF24L01_TxPacket[0]=yg111;
			NRF24L01_TxPacket[1]=yg112;
			NRF24L01_TxPacket[2]=yg121;
			NRF24L01_TxPacket[3]=yg122;	
			NRF24L01_TxPacket[4]=yg211;
			NRF24L01_TxPacket[5]=yg212;
			NRF24L01_TxPacket[6]=yg221;
			NRF24L01_TxPacket[7]=yg222;		
			NRF24L01_TxPacket[8]=anjian_state;	
			OLED_ShowNum(2,1,ADzhi11,4);
			OLED_ShowNum(2,6,ADzhi12,4);
			OLED_ShowNum(3,1,ADzhi22,4);
			OLED_ShowNum(3,6,ADzhi21,4);
			OLED_ShowNum(4,1,SendBuf[9],1);
			OLED_ShowNum(4,3,SendBuf[10],1);
			OLED_ShowNum(4,5,SendBuf[12],1);
			OLED_ShowNum(4,7,SendBuf[13],1);
			OLED_ShowNum(4,9,SendBuf[11],1);
			OLED_ShowNum(4,11,SendBuf[14],1);
			
		SendFlag = NRF24L01_Send();
		NRF24L01_Receive();
		OLED_ShowNum(3,13,NRF24L01_RxPacket[0]/10,2);
		OLED_ShowString(3,15,".");
		OLED_ShowNum(3,16,NRF24L01_RxPacket[0]%10,1);
		runcnt++;
//		suijishu[suijishuzucount]=
	}
}
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		mycountLED++;
		if(mycountLED%250==0){
			if(suiji==1){
			suiji=0;
		suijishu[suijishuzucount]=ADzhi21%10;
			suijishuzucount++;
			if(suijishuzucount>=4){
			suijishuzucount=0;
			}
//			ceshijishu[suijishu[suijishuzucount]]++;
		}
		if(ADzhi21!=0){
		suiji=1;
		}		
		mysjs=rand()%2;
		}
		if(mycountLED>=1000){
			mycountLED=0;
			static unsigned char mymyflag=0;
			ceshikan1.B=(suijishu[1]+1)*1;
			ceshikan1.G=(suijishu[0]+2)*2;
			ceshikan1.R=(suijishu[2]+mysjs)*1;
			ceshikan2.B=(suijishu[0]+1)*2;
			ceshikan2.G=(suijishu[1]+mysjs)*1;
			ceshikan2.R=(suijishu[2]+3)*2;
			ceshikan3.B=(suijishu[2]+mysjs)*2;
			ceshikan3.G=(suijishu[1]+1)*3;
			ceshikan3.R=(suijishu[0]+3)*2;			
			switch(mymyflag){
				case 0:
	Set_Color_On_Array(0,*ceshikan[3]);
	Set_Color_On_Array(1,*ceshikan[0]);	
	Set_Color_On_Array(2,*ceshikan[1]);
	Set_Color_On_Array(3,*ceshikan[2]);	
				mymyflag++;				
				break;
				case 1:
	Set_Color_On_Array(0,*ceshikan[3]);
	Set_Color_On_Array(1,*ceshikan[0]);	
	Set_Color_On_Array(2,*ceshikan[1]);
	Set_Color_On_Array(3,*ceshikan[2]);	
				mymyflag=0;	
				break;
			}
		Send_ALL_LED_Array();
		}
		switch(firstflag){
			case 1:
				firstflagcnt++;
					GPIO_WriteBit(GPIOA,GPIO_Pin_8,0);
			if(firstflagcnt>=300){
				firstflagcnt=0;
			firstflag++;
			}
			break;
			case 2:
				firstflagcnt++;
					GPIO_WriteBit(GPIOA,GPIO_Pin_8,1);
			if(firstflagcnt>=300){
				firstflagcnt=0;
			firstflag++;
			}				
			break;
			case 3:
				firstflagcnt++;
					GPIO_WriteBit(GPIOA,GPIO_Pin_8,0);
			if(firstflagcnt>=300){
				firstflagcnt=0;
			firstflag++;
			}				
			break;
			case 4:
				firstflagcnt++;
				GPIO_WriteBit(GPIOA,GPIO_Pin_8,1);
			if(firstflagcnt>=300){
				firstflagcnt=0;
			firstflag++;
			}					
			break;
			case 5:
				firstflagcnt++;
				GPIO_WriteBit(GPIOA,GPIO_Pin_8,0);
			if(firstflagcnt>=300){
				firstflagcnt=0;
			firstflag++;
			}					
			break;			
		}
		count++;
		if(count%10==0){
		hqanjian();
			if(down==5){
			if(ygaj1==0){
			ygaj1=1;
					
			}
			else{
			ygaj1=0;
				
			}
			}
			if(down==6){
			if(ygaj2==0){
			ygaj2=1;
			}
			else{
			ygaj2=0;
			}			
			}
		if(up==4){
		if(aj4==0){
		aj4=1;
		GPIO_WriteBit(GPIOA,GPIO_Pin_12,(BitAction)0);
		}
			else{
			aj4=0;
		GPIO_WriteBit(GPIOA,GPIO_Pin_12,(BitAction)1);	
			}
		}	
//		if(up==3){
//		if(aj3==0){
//		aj3=1;
//		GPIO_WriteBit(GPIOA,GPIO_Pin_10,(BitAction)0);
//		}
//			else{
//			aj3=0;
//		GPIO_WriteBit(GPIOA,GPIO_Pin_10,(BitAction)1);
//			}
//		}
//		if(up==2){
//		if(aj2==0){
//		aj2=1;
//		GPIO_WriteBit(GPIOA,GPIO_Pin_9,(BitAction)0);
//		}
//			else{
//			aj2=0;
//		GPIO_WriteBit(GPIOA,GPIO_Pin_9,(BitAction)1);
//			}
//		}
		if(down==3){  //中间左右1
			aj3=1;
		GPIO_WriteBit(GPIOA,GPIO_Pin_10,(BitAction)0);
		}
		if(up==3){
		aj3=0;
		GPIO_WriteBit(GPIOA,GPIO_Pin_10,(BitAction)1);	
		}
		if(down==2){   //中间左右2
			aj2=1;
		GPIO_WriteBit(GPIOA,GPIO_Pin_9,(BitAction)0);
		}
		if(up==2){
		aj2=0;
		GPIO_WriteBit(GPIOA,GPIO_Pin_9,(BitAction)1);	
		}
		if(down==7){     //左下角按键
			downflag=1;
			countdeng=0;
		}
		if(downcount<2000&&up==7){  
		downflag=2;
		downcount=0;
		}
		if(up==7){
		downcount=0;
		}
		}	
			if(downflag==1){
			downcount++;
			if(downcount>=2000){
			downflag=3;
			}
		}
		if(downflag==2){
			downflag=0;
		if(aj1==0){
		aj1=1;
		GPIO_WriteBit(GPIOA,GPIO_Pin_11,(BitAction)0);
		}
			else{
			aj1=0;
		GPIO_WriteBit(GPIOA,GPIO_Pin_11,(BitAction)1);
			}		
		}
		if(downflag==3){
		GPIO_WriteBit(GPIOA,GPIO_Pin_8,1);	
		if(countdeng>=500){
		GPIO_WriteBit(GPIOA,GPIO_Pin_8,0);
		}
		aj1=2;
		countdeng++;
		if(countdeng%500==0){
		static unsigned int jiciflag=0;
		jiciflag++;
			if(jiciflag%2==0){
			GPIO_WriteBit(GPIOA,GPIO_Pin_11,(BitAction)1);
			}
			else{
			GPIO_WriteBit(GPIOA,GPIO_Pin_11,(BitAction)0);
			}
		}
		}
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}

void myLED_Init(){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef GPIOSTRUCE;
	GPIOSTRUCE.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIOSTRUCE.GPIO_Pin=GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12;
	GPIOSTRUCE.GPIO_Speed=GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOA,&GPIOSTRUCE);
	
	GPIO_WriteBit(GPIOA,GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12,(BitAction)1);
	GPIO_WriteBit(GPIOA,GPIO_Pin_8,(BitAction)0);
}
