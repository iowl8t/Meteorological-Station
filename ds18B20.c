
/*
Each DS18B20 has a unique 64-bit serial code, which allows multiple DS18B20s to function on the same 1-Wire bus.
*/

/* Послідовність обміну з давачем:
1. To initiate a temperature measurement and A-to-D conversion, the master must issue a Convert T [44h] command.
2. DS18B20 will respond by transmitting 0 while the temperature conversion is in progress and 1 when the conversion is done.
3. The DS18B20 output temperature data is calibrated in degrees Celsius.
4. The temperature data is stored as a 16-bit sign-extended two’s complement number in the temperature register. 
The sign bits (S) indicate if the temperature is positive or negative: for positive numbers S = 0 and for negative numbers S = 1.
If the DS18B20 is configured for 12-bit resolution, all bits in the temperature register will contain valid data.
For 11-bit resolution, bit 0 is undefined. For 10-bit resolution, bits 1 and 0 are undefined..

*
All data and commands are transmitted least significant bit first over the 1-Wire bus.


Initialization:
1. 
 */
#include "main.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "ds18b20.h"


#define SKIP_ROM_COMMAND	0xCC
#define CONVERT_TEMPERATURE	0x44
#define READ_SCRATCHPAD		0xBE

void DS18B20_Init(void)
{
	ONEWIRE_DDR &= ~(1<<ONEWIRE_POS);
	ONEWIRE_PORT &= ~(1<<ONEWIRE_POS);
}

uint8_t DS18B20_Check(void)
{
	char stekTemp = SREG;// зберігаємо значення стека, 
	//щоб відновити глобальні переривання, якщо вони є
	cli(); //забороняємо переривання
	uint8_t presenceFlag = 0;
	
	ONEWIRE_DDR |= (1<<ONEWIRE_POS); //master put bus low
	_delay_us(485); //>480us
	ONEWIRE_DDR &= ~(1<<ONEWIRE_POS); //master release bus
	_delay_us(65); //waiting 15-60us for a presence pulse
	if((ONEWIRE_PIN & (1<<ONEWIRE_POS)) == 0)
		presenceFlag = 1; //пристрій є
	else
		presenceFlag = 0; //пристрою нема
	_delay_us(235);
	SREG = stekTemp;// повертаємо значення стека
	return presenceFlag;
}

void DS18B20_SendBit(uint8_t bit)
{
	char stektemp=SREG;// сохраним значение стека
	cli(); //запрещаем прерывание
	{
		_delay_us(2); //мінімум 1µs відновлення між надсиланням бітів 
		ONEWIRE_DDR |= (1<<ONEWIRE_POS); //pulling bus low
		_delay_us(5); //>1us(waiting while bus is pulled low)
		if(bit) 
			ONEWIRE_DDR &= ~(1<<ONEWIRE_POS); // release bus(if(bit==0) put bus low)
		_delay_us(10); //15us = 5+10
		_delay_us(45); //15(TYP)us + 30(MAX)us
		ONEWIRE_DDR &= ~(1<<ONEWIRE_POS);
	}
	SREG = stektemp;// вернем значение стека
}
uint8_t DS18B20_ReadBit(void)
{
	char stektemp=SREG;// сохраним значение стека
	cli(); //запрещаем прерывание
	uint8_t bit;
	{
		_delay_us(2); //мінімум 1µs відновлення між надсиланням бітів
		ONEWIRE_DDR |= (1<<ONEWIRE_POS); //pulling bus low
		_delay_us(5); //>1us(waiting while bus is pulled low)
		ONEWIRE_DDR &= ~(1<<ONEWIRE_POS); //release bus
		_delay_us(10); //waiting before reading
		bit = (ONEWIRE_PIN & (1<<ONEWIRE_POS)) ? 1 : 0;	
		_delay_us(45);
	}
	SREG = stektemp;// вернем значение стека
	return bit;
}


void DS18B20_SendByte(uint8_t byte)
{
	uint8_t bit = 0;
	for (uint8_t i = 0; i < 8; i++)
	{
		bit = (byte & (1<<i))>>i;
		DS18B20_SendBit(bit);
	}

}
uint8_t DS18B20_ReadByte(void)
{
	uint8_t byte = 0;		
	for (uint8_t i = 0; i < 8; i++)
	{
		byte |=	(DS18B20_ReadBit()<<i);
	}
	return byte;
	
}

int8_t DS18B30_GetTemperature()
{
	uint8_t firstByte = 0;
	uint16_t result = 0;
	if(DS18B20_Check() == 1)
	{
		DS18B20_SendByte(SKIP_ROM_COMMAND);
		DS18B20_SendByte(CONVERT_TEMPERATURE);
		_delay_ms(750);
		DS18B20_Check();
		DS18B20_SendByte(SKIP_ROM_COMMAND);
		DS18B20_SendByte(READ_SCRATCHPAD);
		firstByte = DS18B20_ReadByte();
		result = DS18B20_ReadByte();
		result = (result<<8) | firstByte;
	}
	return ((int8_t) (result>>4)); //відкидуємо значення після коми
}


