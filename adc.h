#ifndef ADC_H_
#define ADC_H_

#include "main.h"
#include <avr/interrupt.h>

#define SingleConversationMode 1
//#define FreeRunningMode	1

unsigned int adc_value;


void ADC_Init(void);

unsigned int ADC_Read(unsigned char channel);


#endif /* ADC_H_ */