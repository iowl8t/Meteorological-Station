#include "lcd.h"

/* Дворядковий 2х16 дисплей - 1206А */


// Для зручності, наглядності
#define RS_SET	((LCD_CONTROL_PORTX) |= (1<<PIN_RS))
#define RW_SET	((LCD_CONTROL_PORTX) |= (1<<PIN_RW))
#define E_SET	((LCD_CONTROL_PORTX) |= (1<<PIN_E))

#define RS_CLEAR	((LCD_CONTROL_PORTX) &= ~(1<<PIN_RS))
#define RW_CLEAR	((LCD_CONTROL_PORTX) &= ~(1<<PIN_RW))
#define E_CLEAR		((LCD_CONTROL_PORTX) &= ~(1<<PIN_E))

#define MASK_DATA_PINS	((1<<LCD_D7)|(1<<LCD_D6)|(1<<LCD_D5)|(1<<LCD_D4))
	
#define LCD_DATA	1
#define LCD_COMMAND	0
#define BF			(LCD_D7)


// Прототипи функцій
static void waitWhileBusy(void);
void LCD_SetPositionXY(const uint8_t x, const uint8_t y);
void LCD_Clear(void);


/* Ініціалізаці портів */
static void portsInit(void)
{
	LCD_CONTROL_DDRX	|= (1<<PIN_RS)|(1<<PIN_RW)|(1<<PIN_E);
	LCD_CONTROL_PORTX	&= ~((1<<PIN_RS)|(1<<PIN_RW)|(1<<PIN_E));
	#ifdef MODE_4BIT
	  LCD_DATA_DDRX		|= (1<<LCD_D7)|(1<<LCD_D6)|(1<<LCD_D5)|(1<<LCD_D4);
	  LCD_DATA_PORTX	&= ~((1<<LCD_D7)|(1<<LCD_D6)|(1<<LCD_D5)|(1<<LCD_D4));
	#else
	  LCD_DATA_DDRX		|= 0xFF;
	  LCD_DATA_PORTX	&= 0x00;
	#endif

}

/* Функція надсилання половини байта */
/* Половина байта розташовані у першій тетраді */
static void sendNibble(const uint8_t halfByte)
{
	LCD_DATA_PORTX &= ~MASK_DATA_PINS; //очистка порта
	LCD_DATA_PORTX |= (halfByte << LCD_D4);
	/* Зсув відбувається, якщо піни даних розташовані не на 0-3 позиції
	Наприклад, якщо D4-D7 є на пінах PORTx4-PORTx7 — зсув буде (halfByte<<4) 
	Тобто залежить від номера першого піна */
	E_SET;
	_delay_us(50);
	E_CLEAR;
}

/* Надсилає байт даних в lcd */
static void sendByte(const char byte)
{
   #ifdef MODE_4BIT
	uint8_t highPart = (byte>>4);
	RW_CLEAR;
	sendNibble(highPart);
	sendNibble(byte);
   #else
    LCD_DATA_PORTX = byte;
	E_SET;
	_delay_us(50);
	E_CLEAR;
   #endif
}

/* Надсилає дані */
void LCD_SendData(const char byte)
{
	waitWhileBusy();
	RS_SET;
	sendByte(byte);
}

/* Надсилає команду */
void LCD_SendCommand(const char byte)
{
	waitWhileBusy();
	RS_CLEAR;
	sendByte(byte);
}

/* Надсилає стрічку */
void LCD_SendString(const char * str)
{
	for(uint8_t i = 0; i <= (strlen(str)-1); i++) //"-1" — мінус нульовий символ '\0'
		LCD_SendData(str[i]);
}

/* Надсилає стрічку починаючи з заданого розташування */
void LCD_SendStringXY(const char * str, const uint8_t x, uint8_t y)
{
	LCD_SetPositionXY(x, y);
	LCD_SendString(str);
}

/* Ініціалізація lcd */
void LCD_Init(void)
{
	portsInit();
	#ifdef MODE_4BIT
		_delay_ms(15);		//more than 15 ms
		sendNibble(0x03);
		_delay_ms(4.1);		//more than 4.1 ms
		sendNibble(0x03);
		_delay_us(100);		//more than 100 us
		sendNibble(0x03);
		sendNibble(0x02);
		// Якщо не працює — додати ще одну команду: sendNibble(0x02);
		waitWhileBusy();
		LCD_SendCommand(0x28);	//N=1 - два рядка, F=0 - 5х8, D/I=0 - 4 бітна шина
	#else
		_delay_ms(15);
		LCD_SendCommand(0x30);
		_delay_ms(4.1);
		LCD_SendCommand(0x30);
		_delay_us(100);
		LCD_SendCommand(0x30);
		LCD_SendCommand(0x38);	//8-розрядна шина, дві стрічки
	#endif
	waitWhileBusy();
	LCD_SendCommand(0x08);	//D=0 - дисплей off, С=1 - курсор підкреслення
	waitWhileBusy();
	LCD_SendCommand(0x0C);	//display on, миготіння крсора
	waitWhileBusy();
	LCD_SendCommand(0x06);	//I/D=1 - зростання АС, S=0 - без зсуву екрана
}

/* Перевірка прапорця зайнятості */
static void waitWhileBusy(void)
{
	#ifdef CHECK_FLAG_BF
	   #ifdef MODE_4BIT
		uint8_t busyFlagTmp = 0;
		
		LCD_DATA_DDRX	&= ~MASK_DATA_PINS;
		LCD_DATA_PORTX	|= MASK_DATA_PINS;
		RS_CLEAR;
		RW_SET;
		
		do{
			E_SET;
			_delay_us(2);
			busyFlagTmp = (LCD_DATA_PINX & (1<<BF));
			E_CLEAR;
			E_SET; //зчитуємо другу, неінформативну частину байта
			_delay_us(2);  //байт, котрий читається, містить прапорець BF та вміст комірки пам'яті(DDRAM or CGRAM, як вибрано перед тим) вказаної в лічильнику адрес
			E_CLEAR;
		} while (busyFlagTmp != 0); //поки контролер lcd працює, біт зайнятості = 1
		//_delay_us(1500); //якщо не працює — спробувати розкоментувати
		LCD_DATA_DDRX	|= MASK_DATA_PINS;
		LCD_DATA_PORTX	&= ~MASK_DATA_PINS;
		RW_CLEAR;
	   #else
	   	uint8_t busyFlagTmp = 0;
	   	LCD_DATA_DDRX = 0x00;
	   	LCD_DATA_PORTX = 0xFF;
	   	
	   	RS_CLEAR;
	   	RW_SET;
	   	do
	   	{
		   	E_SET;
		   	_delay_us(2);
		   	busyFlagTmp = LCD_DATA_PINX & 0x80;
		   	E_CLEAR;
	   	} while (busyFlagTmp != 0);
	   	RW_CLEAR;
	   	LCD_DATA_DDRX = 0xFF;
	   	#endif
	#else 
		_delay_us(40);
	#endif
}

/* Встановлення курсора в задану позицію */
void LCD_SetPositionXY(const uint8_t x, const uint8_t y)
{
	uint8_t position;
	position = (0x40 * y + x)| 0x80;
	LCD_SendCommand(position);
}

/* Очищення дисплея */
void LCD_Clear(void)
{
	LCD_SendCommand(0x01);
   #ifdef CHECK_FLAG_BF
	waitWhileBusy();
   #else
	_delay_us(1500);
   #endif
}


/***************************************************************************************/
/********* Функції для виводу готових повідомлень з масиву, та кириличних літер *********/
/***************************************************************************************/

/*static uint8_t Kbig[][8]={	//бітовий масив великих кириличних букв
	{	0b11111,0b10000,0b10000,0b11110,0b10001,0b10001,0b11110,0b00000 },	//Б (0)
	{	0b11111,0b10000,0b10000,0b10000,0b10000,0b10000,0b10000,0b00000 },	//Г (1)
	{	0b01111,0b00101,0b00101,0b01001,0b10001,0b11111,0b10001,0b10001 },	//Д (2)
	{	0b10101,0b10101,0b10101,0b01110,0b10101,0b10101,0b10101,0b00000 },	//Ж (3)
	{	0b11110,0b00001,0b00001,0b00110,0b00001,0b00001,0b11110,0b00000 },	//З (4)
	{	0b10001,0b10001,0b10011,0b10101,0b11001,0b10001,0b10001,0b00000 },	//И (5)
	{	0b00100,0b10101,0b10001,0b10011,0b10101,0b11001,0b10001,0b00000 },	//Й (6)
	{	0b01111,0b00101,0b00101,0b00101,0b00101,0b10101,0b01001,0b00000 },	//Л (7)
	{	0b11111,0b10001,0b10001,0b10001,0b10001,0b10001,0b10001,0b00000 },	//П (8)
	{	0b10001,0b10001,0b10001,0b01010,0b00100,0b01000,0b10000,0b00000 },	//У (9)
	{	0b01110,0b10101,0b10101,0b10101,0b01110,0b00100,0b00100,0b00000 },	//Ф (10)
	{	0b10001,0b10001,0b10001,0b10001,0b10001,0b10001,0b11111,0b00001 },	//Ц (11)
	{	0b10001,0b10001,0b10001,0b01111,0b00001,0b00001,0b00001,0b00000 },	//Ч (12)
	{	0b10101,0b10101,0b10101,0b10101,0b10101,0b10101,0b11111,0b00000 },	//Ш (13)
	{	0b10101,0b10101,0b10101,0b10101,0b10101,0b10101,0b11111,0b00001 },	//Щ (14)
	{	0b11000,0b01000,0b01000,0b01110,0b01001,0b01001,0b01110,0b00000 },	//Ъ (15)
	{	0b10001,0b10001,0b10001,0b11001,0b10101,0b10101,0b11001,0b00000 },	//Ы (16)
	{	0b01110,0b10001,0b00101,0b01011,0b00001,0b10001,0b01110,0b00000 },	//Э (17)
	{	0b10010,0b10101,0b10101,0b11101,0b10101,0b10101,0b10010,0b00000 },	//Ю (18)
	{	0b01111,0b10001,0b10001,0b01111,0b00101,0b01001,0b10001,0b00000 },	//Я (19)
	{   0b00000,0b00000,0b00100,0b00010,0b11111,0b00010,0b00100,0b00000 },	// стрілка ->(20)
};*/
static uint8_t Kmal[][8]={	//бітовий масив маленьких кириличних букв
	{	0b00000,0b01111,0b00001,0b01111,0b10001,0b10001,0b11111,0b00000 },	// а (0)
	{	0b00110,0b01000,0b10000,0b11110,0b10001,0b10001,0b10001,0b01110 },	// б (0)
	{	0b00000,0b00000,0b11100,0b10010,0b11100,0b10010,0b11100,0b00000 },	// в (1)
	{	0b00000,0b00000,0b11110,0b10010,0b10000,0b10000,0b10000,0b00000 },	// г (2)
	{   0b00000,0b00000,0b01110,0b01010,0b01010,0b01010,0b11111,0b10001 },  // Д (3)
	{	0b01010,0b00000,0b11110,0b10000,0b11100,0b10000,0b11110,0b00000 },	// ё (4)
	{	0b01010,0b00100,0b10001,0b10011,0b10101,0b11001,0b10001,0b00000 },	// й (5)
	{	0b00000,0b00000,0b10101,0b10101,0b01110,0b10101,0b10101,0b00000 },	// ж (6)
	{	0b00000,0b00000,0b01110,0b00001,0b00110,0b00001,0b01110,0b00000 },	// з (7)
	{	0b00000,0b00000,0b10001,0b10011,0b10101,0b11001,0b10001,0b00000 },	// и (8)
	{	0b00000,0b00000,0b10010,0b10100,0b11000,0b10100,0b10010,0b00000 },	// к (9)
	{	0b00000,0b00000,0b00111,0b01001,0b01001,0b01001,0b11001,0b00000 },	// л (10)
	{	0b00000,0b00000,0b10001,0b11011,0b10101,0b10001,0b10001,0b00000 },	// м (11)
	{	0b00000,0b00000,0b10001,0b10001,0b11111,0b10001,0b10001,0b00000 },	// н (12)
	{	0b00000,0b00000,0b11111,0b10001,0b10001,0b10001,0b10001,0b00000 },	// п (13)
	{	0b00000,0b00000,0b11111,0b00100,0b00100,0b00100,0b00100,0b00000 },	// т (14)
	{	0b00000,0b00000,0b10001,0b10001,0b11111,0b00001,0b00001,0b00000 },	// ч (15)

};
// Вносимо необхідні повідомлення
static uint8_t x[][16]={
	{ 0x00, 0x6F, 0x01, 0x6F, 0x02, 0x03, 0x04, 0x05, 0x20, 0x02, 0x61, 0x63, 0x20, 0x20, 0x20, 0x20 }, // 0  Поточний час
	{ 0xB0, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x2D, 0xB8, 0xBD, 0xB3, 0x65, 0x63, 0xBF, 0x20 }, // 1  коммунал-инвест
	{ 0x61, 0xB3, 0xBF, 0x20, 0x20, 0x20, 0x70, 0x79, 0xC0, 0x20, 0x20, 0xBD, 0x61, 0x63, 0xBF, 0x70 }, // 2  авт  руч  настр
	{ 0x6F, 0x63, 0xBD, 0x6F, 0xB3, 0xBD, 0x6F, 0xB9, 0x20, 0xBD, 0x61, 0x63, 0x6F, 0x63, 0x48, 0x20 }, // 3  основной насосН
	{ 0xE3, 0x61, 0xBF, 0xC0, 0x20, 0x63, 0x79, 0x78, 0x78, 0x6F, 0xE3, 0x61, 0x20, 0x48, 0x31, 0x32 }, // 4  датч сухходаH12
	{ 0x20, 0x50, 0xD9, 0x20, 0xBA, 0x6F, 0xBD, 0xBF, 0x20, 0x20, 0x20, 0x20, 0x20, 0xBC, 0xBA, 0xBD }, // 5  Р^ конт.     мкн
	{ 0x79, 0x63, 0xBF, 0x61, 0xBD, 0x2E, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 }, // 6  устан.  ВС 00:00
	{ 0xB3, 0x70, 0x65, 0x6D, 0xC7, 0x20, 0x70, 0x61, 0xB2, 0x6F, 0xBF, 0xC3, 0x20, 0x20, 0x48, 0x20 }, // 7  время работы  Н
	{ 0x20, 0x20, 0xBC, 0x6F, 0xBF, 0x6F, 0x70, 0x65, 0x63, 0x79, 0x70, 0x63, 0x20, 0x20, 0x48, 0x20 }, // 8    моторесурс  Н
	{ 0x6F, 0xB2, 0xBD, 0x2E, 0x20, 0xBC, 0x6F, 0xBF, 0x6F, 0x70, 0x65, 0x63, 0x2E, 0x20, 0x48, 0x20 }, // 9  обн. моторес. Н
	{ 0x70, 0x79, 0xC0, 0x20, 0x70, 0x65, 0xB6, 0xB8, 0xBC, 0x20, 0x48, 0x31, 0x20, 0x32, 0x20, 0x33 }, // 10 руч режим Н1 2 3
	{ 0xB3, 0x61, 0xBB, 0x6F, 0xBE, 0x6F, 0xB3, 0x6F, 0x70, 0x6F, 0xBF, 0x20, 0x48, 0x31, 0x32, 0x33 }, // 11 валоповорот Н123
	{ 0xB3, 0x70, 0x65, 0x6D, 0xC7, 0x20, 0xBD, 0x65, 0x20, 0xB7, 0x61, 0xE3, 0x61, 0xBD, 0x6F, 0x20 }, // 12 время не задано
	{ 0x20, 0x20, 0xBD, 0x61, 0xC1, 0x20, 0xBF, 0x65, 0xBB, 0x65, 0xAA, 0x6F, 0xBD, 0x20, 0x20, 0x20 }, // 13   наш телефон
	{ 0x20, 0x30, 0x34, 0x34, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 }, // 14  044 xxx-xx-xx
};
// Масив номерів букв(з Кmal) для конкретних повідомлень з х[][16], для запису в CGRAM
// Якщо не потрібно багато букв - записуємо будь-які літери
static uint8_t cyrillicX[][8] = {
	{ 13, 14, 15, 12, 8, 5, 0, 0},	//записуємо кириличні літери в CGRAM для виводу першого повідомлення x[0]
	{ 0, 0, 0, 0, 0, 0, 0, 0},	//записуємо кириличні літери в CGRAM для виводу другого повідомлення x[1]
	{ 0, 0, 0, 0, 0, 0, 0, 0},	//записуємо кириличні літери в CGRAM для виводу третього повідомлення x[2]
	{ 0, 0, 0, 0, 0, 0, 0, 0},
	{ 0, 0, 0, 0, 0, 0, 0, 0},
	{ 0, 0, 0, 0, 0, 0, 0, 0},
	{ 0, 0, 0, 0, 0, 0, 0, 0},
};

/* Заносить 1 букву з Kmal в одене з 8 місць в CGRAM */
static void writeLetterInCGRAM(const uint8_t n)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		LCD_SendData(Kmal[n][i]);
	}
}

//n - номер повідомлення
/* Заносить 8 букв з Kmal в CGRAM(0-7), індекси букв вказані в cyrillicX */
static void writePocketInCGRAM(const uint8_t n)
{
	LCD_SendCommand(0x40);	//нульовий адрес лічильника в CGRAM
	for (uint8_t i = 0; i < 8; i++)
	{
		writeLetterInCGRAM(cyrillicX[n][i]);
	}
}

//writeLetterInCGRAM і writePocketInCGRAM в одній функції
/*static void writeLetterAndPocketInCGRAM(uint8_t n)
{
	LCD_PutCommand(0x40);	// Нульовий адрес лічильника в CGRAM
	for (uint8_t i = 0; i < 8; i++)
	{
		for (uint8_t j = 0; j < 8; j++)
		{
			//LCD_PutValue(Kmal[cyrillicX[n][i]][j]);			
		}
	}
}*/

/* Функція виводу повідомлень з масиву x[][16] */
void LCD_SendMassageXY(uint8_t n, uint8_t xAxis, uint8_t yAxis)
{
	writePocketInCGRAM(n);
	LCD_SendCommand(0x80);	//нульовий адрес лічильника в DDRAM
	LCD_SetPositionXY(xAxis,yAxis);
	for (uint8_t i = 0; i < 16; i++)
	{
		LCD_SendData(x[n][i]);
	}
}
