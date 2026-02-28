#include "main.h"
#include "tim.h"
#include "gpio.h"
#include "my_iic.h"
#include "stdio.h"


///*********************仅配置I2C硬件（PB6=SCL, PB7=SDA）****************************/
//#define I2C_SDA_PIN    GPIO_PIN_9
//#define I2C_SCL_PIN    GPIO_PIN_8
//#define I2C_PORT       GPIOB

//// I2C基础操作宏（极简）
//#define I2C_SDA_H()    HAL_GPIO_WritePin(I2C_PORT, I2C_SDA_PIN, GPIO_PIN_SET)
//#define I2C_SDA_L()    HAL_GPIO_WritePin(I2C_PORT, I2C_SDA_PIN, GPIO_PIN_RESET)
//#define I2C_SCL_H()    HAL_GPIO_WritePin(I2C_PORT, I2C_SCL_PIN, GPIO_PIN_SET)
//#define I2C_SCL_L()    HAL_GPIO_WritePin(I2C_PORT, I2C_SCL_PIN, GPIO_PIN_RESET)
//#define I2C_SDA_READ() HAL_GPIO_ReadPin(I2C_PORT, I2C_SDA_PIN)

///*********************你指定的状态宏（完全保留）****************************/
//#define Soft_I2C_READY		0x00
//#define Soft_I2C_BUS_BUSY	0x01
//#define Soft_I2C_BUS_ERROR	0x02

///*********************极简延时（适配BMP280时序，168MHz系统时钟）****************************/
//// 微秒延时（无需TIM6，直接基于空指令）
//static void I2C_Delay_Us(uint32_t us)
//{
//    us *= (SystemCoreClock / 1000000) / 5;
//    while(us--) { __NOP(); }
//}

///*********************阻塞式I2C基础操作（标准时序，适配BMP280）****************************/
//// 1. I2C起始信号（SDA先降，SCL后降）
//static void I2C_Start(void)
//{
//    I2C_SDA_H();
//    I2C_SCL_H();
//    I2C_Delay_Us(2);
//    I2C_SDA_L(); // SDA降沿（起始信号）
//    I2C_Delay_Us(2);
//    I2C_SCL_L(); // SCL降，准备发数据
//}

//// 2. I2C停止信号（SDA后升，SCL先升）
//static void I2C_Stop(void)
//{
//    I2C_SDA_L();
//    I2C_SCL_L();
//    I2C_Delay_Us(2);
//    I2C_SCL_H(); // SCL升
//    I2C_Delay_Us(2);
//    I2C_SDA_H(); // SDA升沿（停止信号）
//    I2C_Delay_Us(2);
//}

//// 3. 等待ACK（返回1=无ACK，0=有ACK）
//static uint8_t I2C_Wait_Ack(void)
//{
//    uint8_t ack_flag = 1;
//    I2C_SDA_H(); // 释放SDA，由从机拉低
//    I2C_Delay_Us(1);
//    I2C_SCL_H(); // SCL升，检测ACK
//    I2C_Delay_Us(2);
//    
//    if(I2C_SDA_READ() == GPIO_PIN_RESET) {
//        ack_flag = 0; // 收到ACK
//    }
//    I2C_SCL_L();
//    I2C_Delay_Us(1);
//    return ack_flag;
//}

//// 4. 发送ACK（主机拉低SDA）
//static void I2C_Send_Ack(void)
//{
//    I2C_SDA_L();
//    I2C_Delay_Us(1);
//    I2C_SCL_H();
//    I2C_Delay_Us(2);
//    I2C_SCL_L();
//    I2C_Delay_Us(1);
//    I2C_SDA_H();
//}

//// 5. 发送NACK（主机拉高SDA）
//static void I2C_Send_Nack(void)
//{
//    I2C_SDA_H();
//    I2C_Delay_Us(1);
//    I2C_SCL_H();
//    I2C_Delay_Us(2);
//    I2C_SCL_L();
//    I2C_Delay_Us(1);
//}

//// 6. 发送1字节（返回1=无ACK，0=有ACK）
//static uint8_t I2C_Send_Byte(uint8_t byte)
//{
//    uint8_t i;
//    for(i=0; i<8; i++) {
//        if(byte & 0x80) { I2C_SDA_H(); }
//        else { I2C_SDA_L(); }
//        byte <<= 1;
//        I2C_Delay_Us(1);
//        I2C_SCL_H(); // SCL升，锁存数据
//        I2C_Delay_Us(2);
//        I2C_SCL_L(); // SCL降，准备下一位
//        I2C_Delay_Us(1);
//    }
//    return I2C_Wait_Ack(); // 等待从机ACK
//}

//// 7. 接收1字节（last=1：最后1字节，发NACK；0：发ACK）
//static uint8_t I2C_Recv_Byte(uint8_t last)
//{
//    uint8_t i, byte = 0;
//    I2C_SDA_H(); // 释放SDA，由从机发数据
//    for(i=0; i<8; i++) {
//        byte <<= 1;
//        I2C_SCL_L();
//        I2C_Delay_Us(1);
//        I2C_SCL_H(); // SCL升，读取数据
//        I2C_Delay_Us(2);
//        if(I2C_SDA_READ() == GPIO_PIN_SET) {
//            byte |= 0x01;
//        }
//    }
//    I2C_SCL_L();
//    // 最后1字节发NACK，否则发ACK
//    if(last) { I2C_Send_Nack(); }
//    else { I2C_Send_Ack(); }
//    return byte;
//}

///*********************你指定的5个对外函数（内部改用阻塞式I2C）****************************/
//// 1. I2C初始化（仅配置GPIO为开漏输出+上拉）
//void I2C_Bus_Init(void)
//{
//    GPIO_InitTypeDef gpio_cfg = {0};
//    
//    // 使能GPIOB时钟
//    __HAL_RCC_GPIOB_CLK_ENABLE();
//    
//    // 配置SCL/SDA为开漏输出+上拉（I2C标准配置）
//    gpio_cfg.Pin = I2C_SCL_PIN | I2C_SDA_PIN;
//    gpio_cfg.Mode = GPIO_MODE_OUTPUT_OD;    // 开漏输出
//    gpio_cfg.Pull = GPIO_PULLUP;            // 内部上拉
//    gpio_cfg.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//    
//    HAL_GPIO_Init(I2C_PORT, &gpio_cfg);
//    
//    // 初始化总线为空闲状态
//    I2C_SDA_H();
//    I2C_SCL_H();
//}

//// 2. 设置重试时间（空实现，阻塞式无需重试）
//void Set_I2C_Retry(unsigned short ml_sec)
//{
//    (void)ml_sec; // 兼容你的调用，无实际作用
//}

//// 3. 获取重试时间（空实现）
//unsigned short Get_I2C_Retry(void)
//{
//    return 5; // 返回默认值，兼容你的调用
//}

//// 4. 读寄存器（阻塞式，适配BMP280 7位地址）
//int Sensors_I2C_ReadRegister(unsigned char Address, unsigned char RegisterAddr, 
//                                          unsigned short RegisterLen, unsigned char *RegisterValue)
//{
//    uint8_t i;
//    // 步骤1：起始信号
//    I2C_Start();
//    // 步骤2：发设备地址+写（7位地址<<1 | 0）
//    if(I2C_Send_Byte((Address << 1) | 0)) {
//        I2C_Stop();
//        return Soft_I2C_BUS_ERROR; // 无ACK，返回错误
//    }
//    // 步骤3：发寄存器地址
//    if(I2C_Send_Byte(RegisterAddr)) {
//        I2C_Stop();
//        return Soft_I2C_BUS_ERROR;
//    }
//    // 步骤4：重复起始（读操作）
//    I2C_Start();
//    // 步骤5：发设备地址+读（7位地址<<1 | 1）
//    if(I2C_Send_Byte((Address << 1) | 1)) {
//        I2C_Stop();
//        return Soft_I2C_BUS_ERROR;
//    }
//    // 步骤6：接收数据
//    for(i=0; i<RegisterLen; i++) {
//        // 最后1字节发NACK，其余发ACK
//        RegisterValue[i] = I2C_Recv_Byte((i == RegisterLen-1) ? 1 : 0);
//    }
//    // 步骤7：停止信号
//    I2C_Stop();
//    return Soft_I2C_READY; // 读取成功
//}

//// 5. 写寄存器（阻塞式，适配BMP280）
//int Sensors_I2C_WriteRegister(unsigned char Address, unsigned char RegisterAddr, 
//                                           unsigned short RegisterLen, const unsigned char *RegisterValue)
//{
//    uint8_t i;
//    // 步骤1：起始信号
//    I2C_Start();
//    // 步骤2：发设备地址+写
//    if(I2C_Send_Byte((Address << 1) | 0)) {
//        I2C_Stop();
//        return Soft_I2C_BUS_ERROR;
//    }
//    // 步骤3：发寄存器地址
//    if(I2C_Send_Byte(RegisterAddr)) {
//        I2C_Stop();
//        return Soft_I2C_BUS_ERROR;
//    }
//    // 步骤4：发数据
//    for(i=0; i<RegisterLen; i++) {
//        if(I2C_Send_Byte(RegisterValue[i])) {
//            I2C_Stop();
//            return Soft_I2C_BUS_ERROR;
//        }
//    }
//    // 步骤5：停止信号
//    I2C_Stop();
//    return Soft_I2C_READY; // 写入成功
//}

//// 空实现（兼容你的主循环调用，无实际作用）
//void I2C_Process_NonBlocking(void)
//{
//    // 阻塞式无需轮询，空函数即可
//}

////////////////////////////////////////////////////////////////////到此为止
/*
		|        |
     |      |
      |    |
       |  |
        ||
*/

#define  I2C_Direction_Receiver         ((uint8_t)0x01)
#define  I2C_Direction_Transmitter      ((uint8_t)0x00)
#define mdelay HAL_Delay

#define Soft_I2C_SDA_STATE   	HAL_GPIO_ReadPin(Soft_I2C_PORT, Soft_I2C_SDA)
#define Soft_I2C_DELAY 			Soft_I2C_Delay(100)//1000000
#define Soft_I2C_NOP			Soft_I2C_Delay(20)//100

#define Soft_I2C_READY		0x00
#define Soft_I2C_BUS_BUSY	0x01
#define Soft_I2C_BUS_ERROR	0x02

#define Soft_I2C_NACK  	    0x00
#define Soft_I2C_ACK		0x01

static void Soft_I2C_Delay(uint32_t dly)
{
	while(--dly);	//dly=100: 8.75us; dly=100: 85.58 us (SYSCLK=72MHz)
}

static unsigned short RETRY_IN_MLSEC  = 35;

/**
  * @brief  设置iic重试时间
  * @param  ml_sec：重试的时间，单位毫秒
  * @retval 重试的时间，单位毫秒
  */
void Set_I2C_Retry(unsigned short ml_sec)
{
    RETRY_IN_MLSEC = ml_sec;
}

/**
  * @brief  获取设置的iic重试时间
  * @param  none
  * @retval none
  */
unsigned short Get_I2C_Retry(void)
{
    return RETRY_IN_MLSEC;
}

static void Soft_I2C_Configuration(void)
{
	Soft_I2C_SCL_1;
	Soft_I2C_SDA_1;
	Soft_I2C_DELAY;
}

void I2C_Bus_Init(void)
{
	Set_I2C_Retry(5);
	Soft_I2C_Configuration();
}

static uint8_t Soft_I2C_START(void)
{
	Soft_I2C_SDA_1;
 	Soft_I2C_NOP;

 	Soft_I2C_SCL_1;
 	Soft_I2C_NOP;

 	if(!Soft_I2C_SDA_STATE)
        return Soft_I2C_BUS_BUSY;

 	Soft_I2C_SDA_0;
 	Soft_I2C_NOP;

 	Soft_I2C_SCL_0;
 	Soft_I2C_NOP;

 	if(Soft_I2C_SDA_STATE)
        return Soft_I2C_BUS_ERROR;

 	return Soft_I2C_READY;
}

static void Soft_I2C_STOP(void)
{
 	Soft_I2C_SDA_0;
 	Soft_I2C_NOP;

 	Soft_I2C_SCL_1;
 	Soft_I2C_NOP;

 	Soft_I2C_SDA_1;
 	Soft_I2C_NOP;
}

static void Soft_I2C_SendACK(void)
{
 	Soft_I2C_SDA_0;
 	Soft_I2C_NOP;
 	Soft_I2C_SCL_1;
 	Soft_I2C_NOP;
 	Soft_I2C_SCL_0;
 	Soft_I2C_NOP;
}

static void Soft_I2C_SendNACK(void)
{
	Soft_I2C_SDA_1;
	Soft_I2C_NOP;
	Soft_I2C_SCL_1;
	Soft_I2C_NOP;
	Soft_I2C_SCL_0;
	Soft_I2C_NOP;
}

/**
  * @brief  等待应答信号到来
  * @retval 返回值：1，接收应答失败
	*									0，接收应答成功
  */
uint8_t Soft_I2C_Wait_Ack(void)
{
	uint8_t ucErrTime=0;

	Soft_I2C_SDA_1;
	Soft_I2C_NOP;
	Soft_I2C_SCL_1;
	Soft_I2C_NOP;

	while(Soft_I2C_SDA_STATE)
	{
		ucErrTime ++;
		if(ucErrTime > 250)
		{
			Soft_I2C_STOP();
			return Soft_I2C_BUS_ERROR;
		}
	}
	Soft_I2C_SCL_0;//时钟输出0
	return 0;
}

static uint8_t Soft_I2C_SendByte(uint8_t soft_i2c_data)
{
 	uint8_t i;

	Soft_I2C_SCL_0;
 	for(i=0; i<8; i++)
 	{
  		if(soft_i2c_data & 0x80)
            Soft_I2C_SDA_1;
   		else
            Soft_I2C_SDA_0;
  		soft_i2c_data <<= 1;
  		Soft_I2C_NOP;

  		Soft_I2C_SCL_1;
  		Soft_I2C_NOP;
  		Soft_I2C_SCL_0;
  		Soft_I2C_NOP;
 	}
	return Soft_I2C_Wait_Ack();
}

static uint8_t Soft_I2C_ReceiveByte(void)
{
	uint8_t i,soft_i2c_data;

 	Soft_I2C_SDA_1;
 	Soft_I2C_SCL_0;
 	soft_i2c_data = 0;

 	for(i=0; i<8; i++)
 	{
  		Soft_I2C_SCL_1;
  		Soft_I2C_NOP;
  		soft_i2c_data <<= 1;

  		if(Soft_I2C_SDA_STATE)
  			soft_i2c_data |= 0x01;

  		Soft_I2C_SCL_0;
  		Soft_I2C_NOP;
 	}
	Soft_I2C_SendNACK();
 	return soft_i2c_data;
}

static uint8_t Soft_I2C_ReceiveByte_WithACK(void)
{
	uint8_t i,soft_i2c_data;

 	Soft_I2C_SDA_1;
 	Soft_I2C_SCL_0;
 	soft_i2c_data = 0;

 	for(i=0; i<8; i++)
 	{
  		Soft_I2C_SCL_1;
  		Soft_I2C_NOP;
  		soft_i2c_data <<= 1;

  		if(Soft_I2C_SDA_STATE)
  			soft_i2c_data |= 0x01;

  		Soft_I2C_SCL_0;
  		Soft_I2C_NOP;
 	}
	Soft_I2C_SendACK();
 	return soft_i2c_data;
}

static uint8_t Soft_DMP_I2C_Write(uint8_t soft_dev_addr, uint8_t soft_reg_addr, uint8_t soft_i2c_len,unsigned char *soft_i2c_data_buf)
{
    uint8_t i, result = 0;
	Soft_I2C_START();
	result = Soft_I2C_SendByte(soft_dev_addr << 1 | I2C_Direction_Transmitter);
	if(result != 0) return result;

	result = Soft_I2C_SendByte(soft_reg_addr);
	if(result != 0) return result;

	for (i=0; i<soft_i2c_len; i++)
	{
		result = Soft_I2C_SendByte(soft_i2c_data_buf[i]);
		if (result != 0) return result;
	}
	Soft_I2C_STOP();
	return 0x00;
}

static uint8_t Soft_DMP_I2C_Read(uint8_t soft_dev_addr, uint8_t soft_reg_addr, uint8_t soft_i2c_len,unsigned char *soft_i2c_data_buf)
{
	uint8_t result;

	Soft_I2C_START();
	result  = Soft_I2C_SendByte(soft_dev_addr << 1 | I2C_Direction_Transmitter);
	if(result != 0) return result;

	result = Soft_I2C_SendByte(soft_reg_addr);
   //printf("addr:0x%x\n",result);
	if(result != 0) return result;

	Soft_I2C_START();
	result = Soft_I2C_SendByte(soft_dev_addr << 1 | I2C_Direction_Receiver);
	if(result != 0) return result;

    while (soft_i2c_len)
	{
		if (soft_i2c_len == 1)
			*soft_i2c_data_buf = Soft_I2C_ReceiveByte();
        else
        	*soft_i2c_data_buf = Soft_I2C_ReceiveByte_WithACK();
        soft_i2c_data_buf ++;
        soft_i2c_len --;
    }
	Soft_I2C_STOP();
    return 0x00;
}

/**
  * @brief  向IIC设备的寄存器连续写入数据，带超时重试设置，供mpu接口调用
  * @param  Address: IIC设备地址
  * @param  RegisterAddr: 寄存器地址
  * @param  RegisterLen: 要写入数据的长度
  * @param  RegisterValue: 要指向写入数据的指针
  * @retval 0正常，非0异常
  */
int Sensors_I2C_WriteRegister(unsigned char slave_addr,
                                        unsigned char reg_addr,
                                        unsigned short len,
                                        const unsigned char *data_ptr)
{
    char retries = 0;
    int ret = 0;
    unsigned short retry_in_mlsec = Get_I2C_Retry();

tryWriteAgain:
    ret = 0;
    ret = Soft_DMP_I2C_Write( slave_addr, reg_addr, len, ( unsigned char *)data_ptr);

    if(ret && retry_in_mlsec)
    {
        if( retries++ > 4 )
            return ret;

        mdelay(retry_in_mlsec);
        goto tryWriteAgain;
    }
    return ret;
}

/**
  * @brief  向IIC设备的寄存器连续读出数据,带超时重试设置，供mpu接口调用
  * @param  Address: IIC设备地址
  * @param  RegisterAddr: 寄存器地址
  * @param  RegisterLen: 要读取的数据长度
  * @param  RegisterValue: 指向存储读出数据的指针
  * @retval 0正常，非0异常
  */
int Sensors_I2C_ReadRegister(unsigned char slave_addr,
                                       unsigned char reg_addr,
                                       unsigned short len,
                                       unsigned char *data_ptr)
{
    char retries = 0;
    int ret = 0;
    unsigned short retry_in_mlsec = Get_I2C_Retry();

tryReadAgain:
    ret = 0;
    ret = Soft_DMP_I2C_Read( slave_addr, reg_addr, len, ( unsigned char *)data_ptr);

    if(ret && retry_in_mlsec)
    {
        if( retries++ > 4 )
            return ret;

        mdelay(retry_in_mlsec);
        goto tryReadAgain;
    }
    return ret;
}

