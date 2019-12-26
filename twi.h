#ifndef NWO_TWI_H_
#define NWO_TWI_H_

#include "main.h"

#define F_I2C	100000UL

void I2C_Init(void);
void I2C_StartCondition();
void I2C_StopCondition();
void I2C_SendByte(const uint8_t byte);
uint8_t I2C_ReceiveByte(void);
uint8_t I2C_ReceiveLastByte(void);
void I2C_SendPacket(const uint8_t byte, uint8_t adr_rw);

#endif /* NWO_TWI_H_ */