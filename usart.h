#ifndef USART_H_
#define USART_H_

#include "main.h"
#include <util/atomic.h>

#define USART_BUF_SIZE	0b00000001
#define USART_BAUDRATE	57600

/* ���� �������� �������� � "USART_BAUDRATE", �� ������ "speed(�� ubrr)" ������� ����-��� ����� */
void USART_Init(uint16_t speed);

/* �������� ������ ���������� */
void USART_FlashTBuff(void);

void USART_PutByte(const uint8_t ch);
void USART_PutByteNoInterrupt(const uint8_t ch);
void USART_PutStr(const char *str);

uint8_t USART_GetByte(void);


#endif /* USART_H_ */