//#ifndef __NRF24L01_DEFINE_H
//#define __NRF24L01_DEFINE_H

////通信引脚
//#define IRQ_Port   GPIOB
//#define CE_Port    GPIOB
//#define MISO_Port  GPIOA
//#define MOSI_Port  GPIOA
//#define SCK_Port   GPIOA
//#define CSN_Port   GPIOA

//#define IRQ_Pin    GPIO_Pin_1
//#define CE_Pin     GPIO_Pin_0
//#define MOSI_Pin   GPIO_Pin_7
//#define MISO_Pin   GPIO_Pin_6
//#define SCK_Pin    GPIO_Pin_5
//#define CSN_Pin    GPIO_Pin_4

////寄存器地址代码
//#define CONFIG      0x00
//#define EN_AA       0x01
//#define EN_RXADDR   0x02
//#define SETUP_AW    0x03
//#define SETUP_RETR  0x04
//#define RF_CH       0x05
//#define RF_SETUP    0x06
//#define STATUS      0x07
//#define OBSERVE_TX  0x08
//#define CD          0x09
//#define RX_ADDR_P0  0x0A
//#define RX_ADDR_P1  0x0B
//#define RX_ADDR_P2  0x0C
//#define RX_ADDR_P3  0x0D
//#define RX_ADDR_P4  0x0E
//#define RX_ADDR_P5  0x0F
//#define TX_ADDR     0x10
//#define RX_PW_P0    0x11
//#define RX_PW_P1    0x12
//#define RX_PW_P2    0x13
//#define RX_PW_P3    0x14
//#define RX_PW_P4    0x15
//#define RX_PW_P5    0x16
//#define FIFO_STATUS 0x17
//#define DYNPD       0x1C
//#define FEATURE     0x1D

////操作指令代码
//#define R_REGISTER    0x00
//#define W_REGISTER    0x20
//#define R_RX_PAYLOAD  0x61
//#define W_TX_PAYLOAD  0xA0
//#define FLUSH_TX      0xE1
//#define FLUSH_RX      0xE2
//#define NOP           0xFF

////状态
//#define RX_OK       0x40
//#define TX_OK       0x20
//#define MAX_TX      0x10


//#endif

#ifndef __NRF24L01_DEFINE_H
#define __NRF24L01_DEFINE_H

/*NRF24L01指令宏定义*/
#define NRF24L01_R_REGISTER			0x00	//读寄存器，高3位为指令码，低5位为寄存器地址，后续跟1~5字节读数据
#define NRF24L01_W_REGISTER			0x20	//写寄存器，高3位为指令码，低5位为寄存器地址，后续跟1~5字节写数据
#define NRF24L01_R_RX_PAYLOAD		0x61	//读Rx有效载荷，后续跟1~32字节读数据
#define NRF24L01_W_TX_PAYLOAD		0xA0	//写Tx有效载荷，后续跟1~32字节写数据
#define NRF24L01_FLUSH_TX			0xE1	//清空Tx FIFO所有数据，单独指令
#define NRF24L01_FLUSH_RX			0xE2	//清空Rx FIFO所有数据，单独指令
#define NRF24L01_REUSE_TX_PL		0xE3	//重新使用最后一次发送的有效载荷，单独指令
#define NRF24L01_R_RX_PL_WID		0x60	//读取Rx FIFO最前面一个数据包的宽度，后续跟1字节读数据，仅适用于动态包长模式
#define NRF24L01_W_ACK_PAYLOAD		0xA8	//写应答附带的有效载荷，高5位为指令码，低3位为通道号，后续跟1~32字节写数据，仅适用于应答附带载荷模式
#define NRF24L01_W_TX_PAYLOAD_NOACK	0xB0	//写Tx有效载荷，不要求应答，后续跟1~32字节写数据，仅适用于不要求应答模式
#define NRF24L01_NOP				0xFF	//空操作，单独指令，可以用读取状态寄存器

/*NRF24L01寄存器地址宏定义*/
#define NRF24L01_CONFIG				0x00	//配置寄存器，1字节
#define NRF24L01_EN_AA				0x01	//使能自动应答，1字节
#define NRF24L01_EN_RXADDR			0x02	//使能接收通道，1字节
#define NRF24L01_SETUP_AW			0x03	//设置地址宽度，1字节
#define NRF24L01_SETUP_RETR			0x04	//设置自动重传，1字节
#define NRF24L01_RF_CH				0x05	//射频通道，1字节
#define NRF24L01_RF_SETUP			0x06	//射频相关参数设置，1字节
#define NRF24L01_STATUS				0x07	//状态寄存器，1字节
#define NRF24L01_OBSERVE_TX			0x08	//发送观察寄存器，1字节
#define NRF24L01_RPD				0x09	//接收功率检测，1字节
#define NRF24L01_RX_ADDR_P0			0x0A	//接收通道0地址，5字节
#define NRF24L01_RX_ADDR_P1			0x0B	//接收通道1地址，5字节
#define NRF24L01_RX_ADDR_P2			0x0C	//接收通道2地址，1字节，高位地址与接收通道1相同
#define NRF24L01_RX_ADDR_P3			0x0D	//接收通道3地址，1字节，高位地址与接收通道1相同
#define NRF24L01_RX_ADDR_P4			0x0E	//接收通道4地址，1字节，高位地址与接收通道1相同
#define NRF24L01_RX_ADDR_P5			0x0F	//接收通道5地址，1字节，高位地址与接收通道1相同
#define NRF24L01_TX_ADDR			0x10	//发送地址，5字节
#define NRF24L01_RX_PW_P0			0x11	//接收通道0有效载荷数据宽度，1字节
#define NRF24L01_RX_PW_P1			0x12	//接收通道1有效载荷的数据宽度，1字节
#define NRF24L01_RX_PW_P2			0x13	//接收通道2有效载荷的数据宽度，1字节
#define NRF24L01_RX_PW_P3			0x14	//接收通道3有效载荷的数据宽度，1字节
#define NRF24L01_RX_PW_P4			0x15	//接收通道4有效载荷的数据宽度，1字节
#define NRF24L01_RX_PW_P5			0x16	//接收通道5有效载荷的数据宽度，1字节
#define NRF24L01_FIFO_STATUS		0x17	//发送和接收FIFO状态，1字节
#define NRF24L01_DYNPD				0x1C	//使能接收通道的动态包长模式，1字节
#define NRF24L01_FEATURE			0x1D	//使能高级功能，1字节

#endif


/*****************江协科技|版权所有****************/
/*****************jiangxiekeji.com*****************/

