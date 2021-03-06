#ifndef LCD_H_
#define LCD_H_

#include "main.h"
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

#define MODE_4BIT
//#define MODE_8BIT

// ���� ���������
#define	LCD_CONTROL_DDRX	DDRB
#define	LCD_CONTROL_PORTX	PORTB
#define	LCD_CONTROL_PINX	PINB

// ���� �����
#define	LCD_DATA_DDRX	DDRD
#define	LCD_DATA_PORTX	PORTD
#define	LCD_DATA_PINX	PIND

// ϳ�� ���������
#define PIN_RS	0
#define PIN_RW	1
#define PIN_E	2

// ϳ�� �����
/* ������� ���� ����� � ��-������� */
#define LCD_D4	4
#define LCD_D5	5
#define LCD_D6	6
#define LCD_D7	7

#define CHECK_FLAG_BF

/* ������������ lcd */
void LCD_Init(void);

/* ������� ���� */
void LCD_SendData(const char byte);

/* ������� ������� */
void LCD_SendCommand(const char byte);

/* ������� ������ */
void LCD_SendString(const char * str);

/* ������������ ������� � ������ ������� */
void LCD_SetPositionXY(const uint8_t x, const uint8_t y);

/* ������� ������ ��������� � �������� ������������ */
void LCD_SendStringXY(const char * str, const uint8_t x, uint8_t y);

/* �������� ����������� n � ������ ������� ���������� "x[][16]". n - ������ ����������� */
void LCD_SendMassageXY(const uint8_t n, const uint8_t x, const uint8_t y);

/* �������� ������� */
void LCD_Clear(void);



#endif /* LCD_H_ */