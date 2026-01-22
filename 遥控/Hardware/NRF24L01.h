//#ifndef _NRF24L01_H
//#define _NRF24L01_H

//void W_Reg(uint8_t Reg,uint8_t Value);
//uint8_t R_Reg(uint8_t Reg);
//void W_Buf(uint8_t Reg , uint8_t* Buf, uint8_t Len);
//void R_Buf(uint8_t Reg , uint8_t* Buf, uint8_t Len);
//void NRF24L01_Init(void);
//void Receive(uint8_t* Buf);
//uint8_t Send(uint8_t* Buf);
//void TIM2_Init(void);
//void TIM3_Init(void);

//#endif
#ifndef __NRF24L01_H
#define __NRF24L01_H

#include "NRF24L01_Define.h"

/*外部可调用全局数组***********/

extern uint8_t NRF24L01_TxAddress[];
extern uint8_t NRF24L01_TxPacket[];

extern uint8_t NRF24L01_RxAddress[];
extern uint8_t NRF24L01_RxPacket[];

/***********外部可调用全局数组*/


/*函数声明*********************/

/*指令实现*/
uint8_t NRF24L01_ReadReg(uint8_t RegAddress);
void NRF24L01_ReadRegs(uint8_t RegAddress, uint8_t *DataArray, uint8_t Count);
void NRF24L01_WriteReg(uint8_t RegAddress, uint8_t Data);
void NRF24L01_WriteRegs(uint8_t RegAddress, uint8_t *DataArray, uint8_t Count);
void NRF24L01_ReadRxPayload(uint8_t *DataArray, uint8_t Count);
void NRF24L01_WriteTxPayload(uint8_t *DataArray, uint8_t Count);
void NRF24L01_FlushTx(void);
void NRF24L01_FlushRx(void);
uint8_t NRF24L01_ReadStatus(void);

/*功能函数*/
void NRF24L01_PowerDown(void);
void NRF24L01_StandbyI(void);
void NRF24L01_Rx(void);
void NRF24L01_Tx(void);

void NRF24L01_Init(void);
uint8_t NRF24L01_Send(void);
uint8_t NRF24L01_Receive(void);
void NRF24L01_UpdateRxAddress(void);
void TIM2_Init();
void TIM3_Init();

/*********************函数声明*/


#endif


/*****************江协科技|版权所有****************/
/*****************jiangxiekeji.com*****************/
