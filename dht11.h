#ifndef DHT11_H_
#define DHT11_H_

#include "main.h"	//for F_CPU
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


#define DHT11_DDR	DDRC
#define DHT11_PORT	PORTC
#define DHT11_PIN	PINC
#define DHT11_POS	1


uint8_t DHT11_GetHumidity(void);
uint8_t DHT11_GetTemperature(void);

#endif /* DHT11_H_ */