#ifndef __MY_IIC_H
#define __MY_IIC_H
#include "main.h"
#include "tim.h"

/*等待超时时间*/
//#define I2CT_FLAG_TIMEOUT         ((uint32_t)0x1000)
//#define I2CT_LONG_TIMEOUT         ((uint32_t)(10 * I2CT_FLAG_TIMEOUT))



#define Soft_I2C_SDA 		GPIO_PIN_9
#define Soft_I2C_SCL 		GPIO_PIN_8
#define Soft_I2C_PORT       GPIOB
//
#define Soft_I2C_SCL_0 		HAL_GPIO_WritePin(Soft_I2C_PORT, Soft_I2C_SCL,0)
#define Soft_I2C_SCL_1 		HAL_GPIO_WritePin(Soft_I2C_PORT, Soft_I2C_SCL,1)
#define Soft_I2C_SDA_0 		HAL_GPIO_WritePin(Soft_I2C_PORT, Soft_I2C_SDA,0)
#define Soft_I2C_SDA_1   	HAL_GPIO_WritePin(Soft_I2C_PORT, Soft_I2C_SDA,1)

/*等待超时时间*/
#define I2CT_FLAG_TIMEOUT         ((uint32_t)0x1000)
#define I2CT_LONG_TIMEOUT         ((uint32_t)(10 * I2CT_FLAG_TIMEOUT))


void I2C_Bus_Init(void);

void Set_I2C_Retry(unsigned short ml_sec);
unsigned short Get_I2C_Retry(void);
int Sensors_I2C_ReadRegister(unsigned char Address, unsigned char RegisterAddr, 
                                          unsigned short RegisterLen, unsigned char *RegisterValue);
int Sensors_I2C_WriteRegister(unsigned char Address, unsigned char RegisterAddr, 
                                           unsigned short RegisterLen, const unsigned char *RegisterValue);



#endif 

