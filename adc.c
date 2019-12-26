#include "adc.h"

//static uint8_t adc_low, adc_high;	//Значення ацп
static unsigned char adc_counter = 0, adc_sot;		//кількість усереднень adc_counter - для швидкого пп, adc_sot - для повільного пп
static unsigned int adc_summary = 0;	//сума n вибірок для усереднення
static unsigned int adc_temp = 0;		//усереднене значення ацп


//for Single Conversation Mode and Free Running Mode
void ADC_Init(void)
{
	#ifdef SingleConversationMode
		ADMUX	|= (1<<REFS1)|(1<<REFS0) //Джерело опорної напруги [01] - 5V AVcc; [11] - 2.56V Internal
				  |(0<<MUX3)|(0<<MUX2)|(0<<MUX1)|(0<<MUX0) //Вибір входу
				  |(0<<ADLAR); //ADC Right Adjust Result
		ADCSRA	|= (1<<ADEN) //Ввімкнення АЦП
				  |(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); //Вибір подільника		8MHz/128=62.5 kHz
	#elif FreeRunningMode
		ADMUX	|= (1<<REFS1)|(1<<REFS0) //Джерело опорної напруги
				  |(0<<ADLAR) //ADC Right Adjust Result
				  |(0<<MUX3)|(0<<MUX2)|(0<<MUX1)|(0<<MUX0);	//Вибір входу
		ADCSRA	|= (1<<ADEN) //Ввімкнення АЦП
				  |(1<<ADFR) //вибір Free Running Mode
				  |(1<<ADIE) //Дозвіл переривання ацп
				  |(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); //Вибір подільника		8MHz/128=62.5 kHz
		ADCSRA	|= (1<<ADSC); //Запуск неперервного перетворення
	#else 
		#error	"ADC mode wasn't chosen"
	#endif
}

unsigned int ADC_Read(unsigned char channel)
{
	channel &= 0b00000111;	//виділяємо три біти
	ADMUX |= ((ADMUX & 0b11111000)| channel);	//встановлення каналу для подачі на ацп
	
	ADCSRA |= (1<<ADSC);	//start conversation(if ADIF was set, it will be cleared)
	while(ADCSRA & (1<<ADSC));	//wait while converting in progress
	return (unsigned int) ADC;
}

ISR(ADC_vect)
{
	//adc_low	= ADCL;
	//adc_high	= ADCH;
	//if(adc_counter > 100)
	//{
		//adc_summary += adc_low + (adc_high * 256);	//adc_summary += ADCW;
		//adc_counter++;
	//}
	//else
		//adc_value = adc_summary / 100; 
		//adc_summary = 0;
		//adc_counter = 0;
	//Для швидкого перехідного процесу(якщо остання цифра мерехтить)
	adc_summary += ADC;
	adc_counter++;
	if(adc_counter == 10)
	{
		adc_summary /= 10;
		adc_counter = 0;
		//adc_value = adc_summary; //Якщо пропускаєтсья наступний кусок - розкомунтувати
		adc_sot++;
		if(adc_temp == adc_summary)	// для повільного пп(коли значення останньої цифри посередині(напр. 8,5), процес не встановлюєтсья(напр. то 8, то 9)
		{
			if(adc_sot == 20)	//запист числа, якщо виміряне значення не змінюється 20 ітерацій
			{
				adc_value = adc_summary;
			}
		}
		else
		{
			adc_temp = adc_summary;
			adc_sot = 0;
		}
		adc_summary = 0;
	}

}

/*

Why do VCC and AVCC have to be connected?

1\
Mainly, it has to be connected because the manufacturer says it should.

Aside from that, they should for full operation of the chip (all ports/pins),
to prevent floating pin issues on the AVCC side, to prevent noise on the digital side. 
There are issues where leaving the AVCC side unpowered causes parasitic power draw and 
can destablize the internal clock, or can prevent stable startup.
Atmel designers have decided that having a separate Analog VCC and Ground is the best 
way of allowing relatively noise free analog section, by allowing users to add filtering
and separation of the Digital and Analog Planes, even inside the ATmega. It's not just 
the ATMega8, afaik all ATMegas and even some ATTinys have this design.

2\
AVCC is specified as an independent pin because it connects to key analog components 
internally, and as such should have separate filtering capacitors.

Simple "blinkenlights" projects don't have noise and accuracy requirements.

Now if you mean if they should be connected to the same VOLTAGE, the answer is yes within +/- 0.3V of VCC

From the ATMega8 complete datasheet:

"The ADC has a separate analog supply voltage pin, AVCC. AVCC must not differ more than ±0.3V from VCC." 
and "AVCC is the supply voltage pin for the A/D Converter"
To recap: AVCC and VCC should be at the same voltage (within +/- 0.3 Volts), and it is identified 
as a separate pin to allow the designer to place extra filters on that input to keep noise out
of the sensitive A/D converter portion of the IC.

3\
Often times, digital supply and ground pins will end up with small amounts of noise on them. 
It's hard to eliminate all such noise when digital circuitry is switching significant amounts of current, 
and 150mV or so of power-supply noise is unlikely to affect the circuitry powered
by the digital supply pins. Having 150mV of noise on the analog supply pins, however, 
would make it very difficult or impossible for the analog circuitry to achieve fraction-of-a-percent accuracy.
The fact that the analog pins are separated means that one can take accurate readings even 
if there is 150mV of noise on the digital power supply, provided that the digital supply 
doesn't swing by more than 300mV and one has an analog supply which is somewhere within 300mV of both
extremes of the digital supply's range. Eliminating 99% of the noise from a power source that's 
only feeding the analog-supply pin, and ensuring that source voltage is close to the digital 
supply voltage, is often much easier than trying to eliminate all noise from the digital supply.

*/