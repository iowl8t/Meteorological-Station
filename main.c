/*
 * Meteorological_Station.c
 *
 * Created: 06.01.2018 0:19:19
 * Author : Owl
 */ 

#include "main.h"
#include <stdio.h> //for sprintf()
#include <avr/sleep.h>
/*
	1. Підключення дисплею;
	2. Підключення годинника реального часу;
	3. Підключення давача температури;
	4. Підключення давача освітлення;
	5. Підключення давача вологості;
	7. Режим сну: Power Down Mode;
	
	6. Підключення давача тиску;
	
*/
struct DS1307_Time time1;

void DS1307_FirstInit(void);
void LCD_USART_PrintTime(void);
void LCD_USART_PrintTemperature(void);
void LCD_USART_PrintLighting(void);
void LCD_USART_PrintHumidity(void);

ISR(INT1_vect)
{
	
}


int main(void)
{
				/*		START INIT		*/
	LCD_Init();
	LCD_SendStringXY("METEREOLOGICAL", 1, 0);
	LCD_SendStringXY("STATION", 5, 1);
	_delay_ms(500);
	LCD_Clear();
	
	USART_Init(0);
	I2C_Init();
	DS18B20_Init();
	ADC_Init();
	
					/* END INIT */
	DS1307_FirstInit();
	uint8_t powerDownModeCounter = 0;
	MCUCR |= (0<<ISC11)|(0<<ISC10);
	GICR |= (1<<INT1);
	
	sei();
	while (1)
	{
		/* В Таймер */
		LCD_USART_PrintTime();
		_delay_ms(2000);
		LCD_Clear();
		
		LCD_USART_PrintTemperature();
		_delay_ms(1500);
		LCD_Clear();
		
		LCD_USART_PrintLighting();
		_delay_ms(1000);
		LCD_Clear();
		
		LCD_USART_PrintHumidity();
		_delay_ms(1000);
		LCD_Clear();
		
		powerDownModeCounter++;
		if(powerDownModeCounter == 2) 
		{	
			powerDownModeCounter = 0;
			LCD_SendStringXY("Power Down", 3, 0);
			_delay_ms(1000);
			LCD_Clear();
			
			set_sleep_mode(SLEEP_MODE_PWR_DOWN);
			sleep_enable();
			sleep_cpu();
			/*
			- Визначити який спосіб прокидання вибрати
			- 
			*/
			LCD_SendStringXY("Power On", 4, 0);
			_delay_ms(1000);
			LCD_Clear();
		}

	}
}

void DS1307_FirstInit(void)
{
	time1.sec	= 47;
	time1.min	= 12;
	time1.hour	= 10;
	time1.day	= 3;
	time1.data	= 4;
	time1.mounth= 10;
	time1.year	= 17;
		
	DS1307_AllfromDecToBcd(&time1);
	DS1307_SetTime(&time1);
}

void LCD_USART_PrintTime(void)
{
	DS1307_ReadTime(&time1);
	DS1307_AllfromBcdToDec(&time1);

	LCD_SetPositionXY(4,0);
	LCD_SendData(time1.hour/10 + 0x30);
	LCD_SendData(time1.hour%10 + 0x30);
	LCD_SendData(':');
	LCD_SendData(time1.min/10 + 0x30);
	LCD_SendData(time1.min%10 + 0x30);
	LCD_SendData(':');
	LCD_SendData(time1.sec/10 + 0x30);
	LCD_SendData(time1.sec%10+ 0x30);

	LCD_SetPositionXY(3,1);
	LCD_SendData(time1.data/10 + 0x30);
	LCD_SendData(time1.data%10 + 0x30);
	LCD_SendData('/');
	LCD_SendData(time1.mounth/10 + 0x30);
	LCD_SendData(time1.mounth%10 + 0x30);
	LCD_SendData('/');
	LCD_SendData(time1.year/10 + 0x30);
	LCD_SendData(time1.year%10 + 0x30);
	LCD_SendString("--");
	LCD_SendData(time1.day + 0x30);
	
	
	USART_PutByteNoInterrupt(0x0D);
	USART_PutByteNoInterrupt(time1.hour/10 + 0x30);
	USART_PutByteNoInterrupt(time1.hour%10 + 0x30);
	USART_PutByteNoInterrupt(':');
	USART_PutByteNoInterrupt(time1.min/10 + 0x30);
	USART_PutByteNoInterrupt(time1.min%10 + 0x30);
	USART_PutByteNoInterrupt(':');
	USART_PutByteNoInterrupt(time1.sec/10 + 0x30);
	USART_PutByteNoInterrupt(time1.sec%10 + 0x30);
	USART_PutByteNoInterrupt('-');
	USART_PutByteNoInterrupt('-');
	USART_PutByteNoInterrupt('-');
	USART_PutByteNoInterrupt(time1.data/10 + 0x30);
	USART_PutByteNoInterrupt(time1.data%10 + 0x30);
	USART_PutByteNoInterrupt('/');
	USART_PutByteNoInterrupt(time1.mounth/10 + 0x30);
	USART_PutByteNoInterrupt(time1.mounth%10 + 0x30);
	USART_PutByteNoInterrupt('/');
	USART_PutByteNoInterrupt(time1.year/10 + 0x30);
	USART_PutByteNoInterrupt(time1.year%10 + 0x30);
	USART_PutByteNoInterrupt('-');
	USART_PutByteNoInterrupt('-');
	USART_PutByteNoInterrupt(time1.day + 0x30);
}

void LCD_USART_PrintTemperature(void)
{
	static char str[5];
	
	sprintf(str, "%d  " , DS18B30_GetTemperature());
	
	strcat(str, "*C");
	LCD_SendStringXY("Temperature", 3, 0);
	LCD_SendStringXY(str, 6, 1);
	//LCD_SendStringXY("*C", 1, 10);
	
	USART_PutByteNoInterrupt(0x0D); //кинути в наступний рядок
	USART_PutStr("Temperature: ");
	for (uint8_t i = 0; i < 2; i++) //i < strlen(str);
	{
		USART_PutByteNoInterrupt(str[i]);
	}
	USART_PutStr("*C");
}

void LCD_USART_PrintLighting(void)
{
	unsigned int adcCode; // можна викинути
	float adcVoltage;
	
	adcCode = ADC_Read(0);
	
	// Значення
	/*	Перетвореня Voltage	= (5 / 1024)* adcCode */
	adcVoltage = (float) adcCode * (2.56f/1024.f);
	
	LCD_SendStringXY("Illuminance", 3, 0);
	LCD_SetPositionXY(5, 1);
	LCD_SendData((uint8_t) adcVoltage + 0x30);
	LCD_SendData('.');
	LCD_SendData((uint8_t)(adcVoltage*10)%10 + 0x30);
	LCD_SendData((uint8_t)(adcVoltage*100)%10 + 0x30);
	LCD_SendString(" lux");
	
	USART_PutByteNoInterrupt(0x0D);
	USART_PutStr("Illuminance: ");
	USART_PutByteNoInterrupt((uint8_t) adcVoltage + 0x30);
	USART_PutByteNoInterrupt('.');
	USART_PutByteNoInterrupt((uint8_t)(adcVoltage*10)%10 + 0x30);
	USART_PutByteNoInterrupt((uint8_t)(adcVoltage*100)%10 + 0x30);
	USART_PutStr(" lux");
	USART_PutByteNoInterrupt(0x0D);
}

void LCD_USART_PrintHumidity(void)
{
	uint8_t humidity;
	humidity = DHT11_GetHumidity();
		
	LCD_SendStringXY("Humidity", 4, 0);
	LCD_SetPositionXY(7, 1);
	LCD_SendData(humidity/10 + 0x30);
	LCD_SendData(humidity%10 + 0x30);
	LCD_SendData('%');
	
	USART_PutStr("Humidity: ");
	USART_PutByteNoInterrupt(humidity/10 + 0x30);
	USART_PutByteNoInterrupt(humidity%10 + 0x30);
	USART_PutByteNoInterrupt('%');
	USART_PutByteNoInterrupt(0x0D);	
}