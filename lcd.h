#ifndef LCD_H_
#define LCD_H_

#include "main.h"
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

#define MODE_4BIT
//#define MODE_8BIT

// Порт керування
#define	LCD_CONTROL_DDRX	DDRB
#define	LCD_CONTROL_PORTX	PORTB
#define	LCD_CONTROL_PINX	PINB

// Порт даних
#define	LCD_DATA_DDRX	DDRD
#define	LCD_DATA_PORTX	PORTD
#define	LCD_DATA_PINX	PIND

// Піни керування
#define PIN_RS	0
#define PIN_RW	1
#define PIN_E	2

// Піни даних
/* повинні бути підряд і по-порядку */
#define LCD_D4	4
#define LCD_D5	5
#define LCD_D6	6
#define LCD_D7	7

#define CHECK_FLAG_BF

/* Ініціалізація lcd */
void LCD_Init(void);

/* Надсилає дані */
void LCD_SendData(const char byte);

/* Надсилає команду */
void LCD_SendCommand(const char byte);

/* Надсилає стрічку */
void LCD_SendString(const char * str);

/* Встановлення курсора в задану позицію */
void LCD_SetPositionXY(const uint8_t x, const uint8_t y);

/* Надсилає стрічку починаючи з заданого розташування */
void LCD_SendStringXY(const char * str, const uint8_t x, uint8_t y);

/* Виводить повідомлення n з масиву готових повідомлень "x[][16]". n - індекс повідомлення */
void LCD_SendMassageXY(const uint8_t n, const uint8_t x, const uint8_t y);

/* Очищення дисплея */
void LCD_Clear(void);



#endif /* LCD_H_ */