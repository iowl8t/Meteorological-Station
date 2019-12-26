#include "ds1307.h"

#define DS1307_ADR	0b1101000
#define DS1307_ADRW	(DS1307_ADR<<1)
#define DS1307_ADRR	((DS1307_ADR<<1)|1)


//читає регістр за адресою в regAddr
uint8_t ds1307_ReadRegister(uint8_t regAddr)
{
	I2C_StartCondition();
	I2C_SendByte(DS1307_ADRW);
	I2C_SendByte(regAddr);
	I2C_StartCondition();
	I2C_SendByte(DS1307_ADRR);
	uint8_t value = I2C_ReceiveLastByte();
	return value;
}
//записує value в регістр за адресою в regAddr
void ds1307_WriteRegister(uint8_t regAddr, uint8_t value)
{
	I2C_StartCondition();
	I2C_SendByte(DS1307_ADRW);
	I2C_SendByte(regAddr);
	I2C_SendByte(value);
	I2C_StopCondition();
}


/* Встановлення часу */
void DS1307_SetTime(struct DS1307_Time *pT)
{
	I2C_StartCondition();
	I2C_SendByte(DS1307_ADRW);
	I2C_SendByte(0);
	I2C_SendByte(pT->sec);
	I2C_SendByte(pT->min);
	I2C_SendByte(pT->hour);
	I2C_SendByte(pT->day);
	I2C_SendByte(pT->data);
	I2C_SendByte(pT->mounth);
	I2C_SendByte(pT->year);
	I2C_StopCondition();
}
/* Читання часу */
void DS1307_ReadTime(struct DS1307_Time *pT)
{
	I2C_StartCondition();
	I2C_SendByte(DS1307_ADRW);
	I2C_SendByte(0);
	//_delay_ms(300);
	I2C_StartCondition();
	I2C_SendByte(DS1307_ADRR);
	pT->sec = I2C_ReceiveByte();
	pT->min = I2C_ReceiveByte();
	pT->hour = I2C_ReceiveByte();
	pT->day = I2C_ReceiveByte();
	pT->data = I2C_ReceiveByte();
	pT->mounth = I2C_ReceiveByte();
	pT->year = I2C_ReceiveLastByte();
	I2C_StopCondition();
}


uint8_t toBCD(uint8_t d)
{
	return ((d/10)<<4)|(d%10);
}
uint8_t toDEC(uint8_t d)
{
	return ((d>>4)*10)+(0b00001111&d);
}


void DS1307_AllfromDecToBcd(struct DS1307_Time *pT)
{
	pT->sec = toBCD(pT->sec);
	pT->min = toBCD(pT->min);
	pT->hour = toBCD(pT->hour);
	pT->day = toBCD(pT->day);
	pT->data = toBCD(pT->data);
	pT->mounth = toBCD(pT->mounth);
	pT->year = toBCD(pT->year);
}
void DS1307_AllfromBcdToDec(struct DS1307_Time *pT)
{
	pT->sec = toDEC(pT->sec);
	pT->min = toDEC(pT->min);
	pT->hour = toDEC(pT->hour);
	pT->day = toDEC(pT->day);
	pT->data = toDEC(pT->data);
	pT->mounth = toDEC(pT->mounth);
	pT->year = toDEC(pT->year);
}