#ifndef DS1307_H_
#define DS1307_H_

#include "main.h"
#include "twi.h"


//запуск годинника б≥т CH = 0, секунди обнул€ютьс€
#define DS1307_Start()	DS1307_WriteRegister(0, 0)
//зупинка годинника б≥т CH = 1, секунди обнул€ютьс€
#define DS1307_Stop()	DS1307_WriteRegister(0, 0b10000000)


struct DS1307_Time
{
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t day;
	uint8_t data;
	uint8_t mounth;
	uint16_t year;
};


void DS1307_SetTime(struct DS1307_Time *pT);
void DS1307_ReadTime(struct DS1307_Time *pT);

void DS1307_WriteRegister(uint8_t regAddr, uint8_t value);
uint8_t DS1307_ReadRegister(uint8_t regAddr);

void DS1307_AllfromDecToBcd(struct DS1307_Time *pT);
void DS1307_AllfromBcdToDec(struct DS1307_Time *pT);


#endif /* DS1307_H_ */