#include "dht11.h"

/* 
������������ �����:
	1) ����� ��������������
		- pull down the bus for more then 18mS, then pull it up for	 40uS;
	2) ³������ dht11
		- 54uS low after 80uS high
	3) ������� �����
		- Data consists of decimal and integral parts. A complete data transmission is 40bit, and the sensor sends higher data bit first.
Data format: 8bit integral RH data + 8bit decimal RH data + 8bit integral T data + 8bit decimal T data + 8bit check sum

DHT11 - ������� ����� ���� �������(integer part) ���� ���������������� data[0] - �������� � data[2] - �����������

��� DHT22 ������������ ��������:
	- ֳ�� ������� 	- data[0] = 0000 0010;
	- ������� ������� - data[1] = 1000 0000;
	
	- 0000 0010 1000 0000 = 640 decimal
	- 640/256 = 2.5 decimal

* ��������� � �����������
*/

static uint8_t dhtData[5] = {0,0,0,0,0};

static uint8_t DHT11_ReadData(void)
{
	while(!(DHT11_PIN & (1<<DHT11_POS)));
	for (uint8_t i = 0; i < 5; i++)
	{
		dhtData[i] = 0;
	}
	// Request
	DHT11_DDR	|= (1<<DHT11_POS);	//output
	DHT11_PORT	&= ~(1<<DHT11_POS);	//in 0
	_delay_ms(18);
	DHT11_PORT	|= (1<<DHT11_POS);	//in 1
	_delay_us(40);
	
	// Response
	DHT11_DDR	&= ~(1<<DHT11_POS); //input
	if(DHT11_PIN & (1<<DHT11_POS)) {
		return 0;
	}
	_delay_us(80);
	
	if(!(DHT11_PIN & (1<<DHT11_POS))) {
		return 0;
	}
	// Receive Data(40 data bits)
	while(DHT11_PIN & (1<<DHT11_POS));
	
	for (uint8_t i = 0; i < 5; i++)
	{
		for (uint8_t j = 0; j < 8; j++)
		{
			cli();
			while(!(DHT11_PIN & (1<<DHT11_POS)));
			_delay_us(30);
			
			if (DHT11_PIN & (1<<DHT11_POS))
			{
				dhtData[i] |= (1<<(7-j));
			}
			
			while(DHT11_PIN & (1<<DHT11_POS));
			sei();
		}
	}
	
	if((dhtData[0] + dhtData[1] + dhtData[2] + dhtData[3]) == dhtData[4])	// check controlSum
	{
		return 1;
	}
	return 0;
	
}


uint8_t DHT11_GetHumidity(void)
{
	if(DHT11_ReadData())
	{
		return dhtData[0];
	}
	return 0;
}

uint8_t DHT11_GetTemperature(void)
{
	if(DHT11_ReadData())
	{
		return dhtData[2];
	}
	return 0;
}