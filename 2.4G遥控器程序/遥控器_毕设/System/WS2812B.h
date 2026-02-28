#ifndef __WS2812B_H
#define __WS2812B_H

#define LED_NUM 4
typedef struct{
	unsigned char R;
	unsigned char G;
	unsigned char B;
}RGB_Color;


void MySPI_Init(void);
void Set_Color_On_Array(unsigned char LEDID,RGB_Color color);
void Send_ALL_LED_Array(void);
void my_Send(void);




#endif
