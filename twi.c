#include "twi.h"

#define TWBR_VALUE	(((F_CPU)/(F_I2C) - 16) / 2) 

#if (TWBR_VALUE > 255) || (TWBR_VALUE < 0)
 #error	"TWBR_VALUE isn't correct"
#endif
/*
������������� ����������/��������:	extern, static, register, auto;
������������� ����(�������): const, restrict, volatile;
*/
//������������ I2C �� ���������� 
void I2C_Init(void)
{
	TWBR = TWBR_VALUE;
}
//����������� �������� start
void I2C_StartCondition(void)
{
	TWCR = (1<<TWEN)|(1<<TWSTA)|(1<<TWINT);
	while(!(TWCR & (1<<TWINT)));
}
//����������� �������� stop
void I2C_StopCondition(void)
{
	TWCR = (1<<TWEN)|(1<<TWSTO)|(1<<TWINT);
}
//����������� �����
void I2C_SendByte(const uint8_t byte)
{
	TWDR = byte;
	TWCR = (1<<TWEN)|(1<<TWINT);
	while(!(TWCR & (1<<TWINT)));
}
//��������� �����
uint8_t I2C_ReceiveByte(void)
{
	uint8_t receivedByte;
	TWCR = (1<<TWEN)|(1<<TWINT)|(1<<TWEA);
	while(!(TWCR & (1<<TWINT)));
	receivedByte = TWDR;
	return receivedByte;
}
//��������� ���������� �����
uint8_t I2C_ReceiveLastByte(void)
{
	uint8_t receivedByte;
	TWCR = (1<<TWEN)|(1<<TWINT);
	while(!(TWCR & (1<<TWINT)));
	receivedByte = TWDR;
	return receivedByte;
}
//����������� ������ ����� �� ������� SLA_W
void I2C_SendPacket(const uint8_t byte, uint8_t adr_rw)
{
	I2C_StartCondition();
	I2C_SendByte(adr_rw);
	I2C_SendByte(byte);
	I2C_StopCondition();
}
