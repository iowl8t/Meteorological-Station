#ifndef DS18B20_H_
#define DS18B20_H_

#define ONEWIRE_PIN_PORT	D
#define ONEWIRE_POS			2


#define _CONCAT(a,b)	a##b
#define DDR(x)		_CONCAT(DDR,x)
#define PORT(x)		_CONCAT(PORT,x)
#define PIN(x)		_CONCAT(PIN,x)

#define ONEWIRE_DDR		DDR(ONEWIRE_PIN_PORT)
#define ONEWIRE_PORT	PORT(ONEWIRE_PIN_PORT)
#define ONEWIRE_PIN		PIN(ONEWIRE_PIN_PORT)


void DS18B20_Init(void);
uint8_t DS18B20_Check(void);

void DS18B20_SendBit(uint8_t bit);
uint8_t DS18B20_ReadBit(void);

void DS18B20_SendByte(uint8_t byte);
uint8_t DS18B20_ReadByte(void);

int8_t DS18B30_GetTemperature();

#endif /* DS18B20_H_ */