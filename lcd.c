#include "lcd.h"

/* ����������� 2�16 ������� - 1206� */


// ��� ��������, ����������
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


// ��������� �������
static void waitWhileBusy(void);
void LCD_SetPositionXY(const uint8_t x, const uint8_t y);
void LCD_Clear(void);


/* ����������� ����� */
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

/* ������� ���������� �������� ����� */
/* �������� ����� ����������� � ������ ������ */
static void sendNibble(const uint8_t halfByte)
{
	LCD_DATA_PORTX &= ~MASK_DATA_PINS; //������� �����
	LCD_DATA_PORTX |= (halfByte << LCD_D4);
	/* ���� ����������, ���� ��� ����� ����������� �� �� 0-3 �������
	���������, ���� D4-D7 � �� ���� PORTx4-PORTx7 � ���� ���� (halfByte<<4) 
	����� �������� �� ������ ������� ��� */
	E_SET;
	_delay_us(50);
	E_CLEAR;
}

/* ������� ���� ����� � lcd */
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

/* ������� ���� */
void LCD_SendData(const char byte)
{
	waitWhileBusy();
	RS_SET;
	sendByte(byte);
}

/* ������� ������� */
void LCD_SendCommand(const char byte)
{
	waitWhileBusy();
	RS_CLEAR;
	sendByte(byte);
}

/* ������� ������ */
void LCD_SendString(const char * str)
{
	for(uint8_t i = 0; i <= (strlen(str)-1); i++) //"-1" � ���� �������� ������ '\0'
		LCD_SendData(str[i]);
}

/* ������� ������ ��������� � �������� ������������ */
void LCD_SendStringXY(const char * str, const uint8_t x, uint8_t y)
{
	LCD_SetPositionXY(x, y);
	LCD_SendString(str);
}

/* ������������ lcd */
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
		// ���� �� ������ � ������ �� ���� �������: sendNibble(0x02);
		waitWhileBusy();
		LCD_SendCommand(0x28);	//N=1 - ��� �����, F=0 - 5�8, D/I=0 - 4 ���� ����
	#else
		_delay_ms(15);
		LCD_SendCommand(0x30);
		_delay_ms(4.1);
		LCD_SendCommand(0x30);
		_delay_us(100);
		LCD_SendCommand(0x30);
		LCD_SendCommand(0x38);	//8-�������� ����, �� ������
	#endif
	waitWhileBusy();
	LCD_SendCommand(0x08);	//D=0 - ������� off, �=1 - ������ �����������
	waitWhileBusy();
	LCD_SendCommand(0x0C);	//display on, �������� ������
	waitWhileBusy();
	LCD_SendCommand(0x06);	//I/D=1 - ��������� ��, S=0 - ��� ����� ������
}

/* �������� �������� ��������� */
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
			E_SET; //������� �����, ������������� ������� �����
			_delay_us(2);  //����, ������ ��������, ������ ��������� BF �� ���� ������ ���'��(DDRAM or CGRAM, �� ������� ����� ���) ������� � ��������� �����
			E_CLEAR;
		} while (busyFlagTmp != 0); //���� ��������� lcd ������, �� ��������� = 1
		//_delay_us(1500); //���� �� ������ � ���������� ��������������
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

/* ������������ ������� � ������ ������� */
void LCD_SetPositionXY(const uint8_t x, const uint8_t y)
{
	uint8_t position;
	position = (0x40 * y + x)| 0x80;
	LCD_SendCommand(position);
}

/* �������� ������� */
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
/********* ������� ��� ������ ������� ���������� � ������, �� ���������� ���� *********/
/***************************************************************************************/

/*static uint8_t Kbig[][8]={	//������ ����� ������� ���������� ����
	{	0b11111,0b10000,0b10000,0b11110,0b10001,0b10001,0b11110,0b00000 },	//� (0)
	{	0b11111,0b10000,0b10000,0b10000,0b10000,0b10000,0b10000,0b00000 },	//� (1)
	{	0b01111,0b00101,0b00101,0b01001,0b10001,0b11111,0b10001,0b10001 },	//� (2)
	{	0b10101,0b10101,0b10101,0b01110,0b10101,0b10101,0b10101,0b00000 },	//� (3)
	{	0b11110,0b00001,0b00001,0b00110,0b00001,0b00001,0b11110,0b00000 },	//� (4)
	{	0b10001,0b10001,0b10011,0b10101,0b11001,0b10001,0b10001,0b00000 },	//� (5)
	{	0b00100,0b10101,0b10001,0b10011,0b10101,0b11001,0b10001,0b00000 },	//� (6)
	{	0b01111,0b00101,0b00101,0b00101,0b00101,0b10101,0b01001,0b00000 },	//� (7)
	{	0b11111,0b10001,0b10001,0b10001,0b10001,0b10001,0b10001,0b00000 },	//� (8)
	{	0b10001,0b10001,0b10001,0b01010,0b00100,0b01000,0b10000,0b00000 },	//� (9)
	{	0b01110,0b10101,0b10101,0b10101,0b01110,0b00100,0b00100,0b00000 },	//� (10)
	{	0b10001,0b10001,0b10001,0b10001,0b10001,0b10001,0b11111,0b00001 },	//� (11)
	{	0b10001,0b10001,0b10001,0b01111,0b00001,0b00001,0b00001,0b00000 },	//� (12)
	{	0b10101,0b10101,0b10101,0b10101,0b10101,0b10101,0b11111,0b00000 },	//� (13)
	{	0b10101,0b10101,0b10101,0b10101,0b10101,0b10101,0b11111,0b00001 },	//� (14)
	{	0b11000,0b01000,0b01000,0b01110,0b01001,0b01001,0b01110,0b00000 },	//� (15)
	{	0b10001,0b10001,0b10001,0b11001,0b10101,0b10101,0b11001,0b00000 },	//� (16)
	{	0b01110,0b10001,0b00101,0b01011,0b00001,0b10001,0b01110,0b00000 },	//� (17)
	{	0b10010,0b10101,0b10101,0b11101,0b10101,0b10101,0b10010,0b00000 },	//� (18)
	{	0b01111,0b10001,0b10001,0b01111,0b00101,0b01001,0b10001,0b00000 },	//� (19)
	{   0b00000,0b00000,0b00100,0b00010,0b11111,0b00010,0b00100,0b00000 },	// ������ ->(20)
};*/
static uint8_t Kmal[][8]={	//������ ����� ��������� ���������� ����
	{	0b00000,0b01111,0b00001,0b01111,0b10001,0b10001,0b11111,0b00000 },	// � (0)
	{	0b00110,0b01000,0b10000,0b11110,0b10001,0b10001,0b10001,0b01110 },	// � (0)
	{	0b00000,0b00000,0b11100,0b10010,0b11100,0b10010,0b11100,0b00000 },	// � (1)
	{	0b00000,0b00000,0b11110,0b10010,0b10000,0b10000,0b10000,0b00000 },	// � (2)
	{   0b00000,0b00000,0b01110,0b01010,0b01010,0b01010,0b11111,0b10001 },  // � (3)
	{	0b01010,0b00000,0b11110,0b10000,0b11100,0b10000,0b11110,0b00000 },	// � (4)
	{	0b01010,0b00100,0b10001,0b10011,0b10101,0b11001,0b10001,0b00000 },	// � (5)
	{	0b00000,0b00000,0b10101,0b10101,0b01110,0b10101,0b10101,0b00000 },	// � (6)
	{	0b00000,0b00000,0b01110,0b00001,0b00110,0b00001,0b01110,0b00000 },	// � (7)
	{	0b00000,0b00000,0b10001,0b10011,0b10101,0b11001,0b10001,0b00000 },	// � (8)
	{	0b00000,0b00000,0b10010,0b10100,0b11000,0b10100,0b10010,0b00000 },	// � (9)
	{	0b00000,0b00000,0b00111,0b01001,0b01001,0b01001,0b11001,0b00000 },	// � (10)
	{	0b00000,0b00000,0b10001,0b11011,0b10101,0b10001,0b10001,0b00000 },	// � (11)
	{	0b00000,0b00000,0b10001,0b10001,0b11111,0b10001,0b10001,0b00000 },	// � (12)
	{	0b00000,0b00000,0b11111,0b10001,0b10001,0b10001,0b10001,0b00000 },	// � (13)
	{	0b00000,0b00000,0b11111,0b00100,0b00100,0b00100,0b00100,0b00000 },	// � (14)
	{	0b00000,0b00000,0b10001,0b10001,0b11111,0b00001,0b00001,0b00000 },	// � (15)

};
// ������� ��������� �����������
static uint8_t x[][16]={
	{ 0x00, 0x6F, 0x01, 0x6F, 0x02, 0x03, 0x04, 0x05, 0x20, 0x02, 0x61, 0x63, 0x20, 0x20, 0x20, 0x20 }, // 0  �������� ���
	{ 0xB0, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x2D, 0xB8, 0xBD, 0xB3, 0x65, 0x63, 0xBF, 0x20 }, // 1  ��������-������
	{ 0x61, 0xB3, 0xBF, 0x20, 0x20, 0x20, 0x70, 0x79, 0xC0, 0x20, 0x20, 0xBD, 0x61, 0x63, 0xBF, 0x70 }, // 2  ���  ���  �����
	{ 0x6F, 0x63, 0xBD, 0x6F, 0xB3, 0xBD, 0x6F, 0xB9, 0x20, 0xBD, 0x61, 0x63, 0x6F, 0x63, 0x48, 0x20 }, // 3  �������� ������
	{ 0xE3, 0x61, 0xBF, 0xC0, 0x20, 0x63, 0x79, 0x78, 0x78, 0x6F, 0xE3, 0x61, 0x20, 0x48, 0x31, 0x32 }, // 4  ���� �������H12
	{ 0x20, 0x50, 0xD9, 0x20, 0xBA, 0x6F, 0xBD, 0xBF, 0x20, 0x20, 0x20, 0x20, 0x20, 0xBC, 0xBA, 0xBD }, // 5  �^ ����.     ���
	{ 0x79, 0x63, 0xBF, 0x61, 0xBD, 0x2E, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 }, // 6  �����.  �� 00:00
	{ 0xB3, 0x70, 0x65, 0x6D, 0xC7, 0x20, 0x70, 0x61, 0xB2, 0x6F, 0xBF, 0xC3, 0x20, 0x20, 0x48, 0x20 }, // 7  ����� ������  �
	{ 0x20, 0x20, 0xBC, 0x6F, 0xBF, 0x6F, 0x70, 0x65, 0x63, 0x79, 0x70, 0x63, 0x20, 0x20, 0x48, 0x20 }, // 8    ����������  �
	{ 0x6F, 0xB2, 0xBD, 0x2E, 0x20, 0xBC, 0x6F, 0xBF, 0x6F, 0x70, 0x65, 0x63, 0x2E, 0x20, 0x48, 0x20 }, // 9  ���. �������. �
	{ 0x70, 0x79, 0xC0, 0x20, 0x70, 0x65, 0xB6, 0xB8, 0xBC, 0x20, 0x48, 0x31, 0x20, 0x32, 0x20, 0x33 }, // 10 ��� ����� �1 2 3
	{ 0xB3, 0x61, 0xBB, 0x6F, 0xBE, 0x6F, 0xB3, 0x6F, 0x70, 0x6F, 0xBF, 0x20, 0x48, 0x31, 0x32, 0x33 }, // 11 ����������� �123
	{ 0xB3, 0x70, 0x65, 0x6D, 0xC7, 0x20, 0xBD, 0x65, 0x20, 0xB7, 0x61, 0xE3, 0x61, 0xBD, 0x6F, 0x20 }, // 12 ����� �� ������
	{ 0x20, 0x20, 0xBD, 0x61, 0xC1, 0x20, 0xBF, 0x65, 0xBB, 0x65, 0xAA, 0x6F, 0xBD, 0x20, 0x20, 0x20 }, // 13   ��� �������
	{ 0x20, 0x30, 0x34, 0x34, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 }, // 14  044 xxx-xx-xx
};
// ����� ������ ����(� �mal) ��� ���������� ���������� � �[][16], ��� ������ � CGRAM
// ���� �� ������� ������ ���� - �������� ����-�� �����
static uint8_t cyrillicX[][8] = {
	{ 13, 14, 15, 12, 8, 5, 0, 0},	//�������� ��������� ����� � CGRAM ��� ������ ������� ����������� x[0]
	{ 0, 0, 0, 0, 0, 0, 0, 0},	//�������� ��������� ����� � CGRAM ��� ������ ������� ����������� x[1]
	{ 0, 0, 0, 0, 0, 0, 0, 0},	//�������� ��������� ����� � CGRAM ��� ������ �������� ����������� x[2]
	{ 0, 0, 0, 0, 0, 0, 0, 0},
	{ 0, 0, 0, 0, 0, 0, 0, 0},
	{ 0, 0, 0, 0, 0, 0, 0, 0},
	{ 0, 0, 0, 0, 0, 0, 0, 0},
};

/* �������� 1 ����� � Kmal � ����� � 8 ���� � CGRAM */
static void writeLetterInCGRAM(const uint8_t n)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		LCD_SendData(Kmal[n][i]);
	}
}

//n - ����� �����������
/* �������� 8 ���� � Kmal � CGRAM(0-7), ������� ���� ������� � cyrillicX */
static void writePocketInCGRAM(const uint8_t n)
{
	LCD_SendCommand(0x40);	//�������� ����� ��������� � CGRAM
	for (uint8_t i = 0; i < 8; i++)
	{
		writeLetterInCGRAM(cyrillicX[n][i]);
	}
}

//writeLetterInCGRAM � writePocketInCGRAM � ����� �������
/*static void writeLetterAndPocketInCGRAM(uint8_t n)
{
	LCD_PutCommand(0x40);	// �������� ����� ��������� � CGRAM
	for (uint8_t i = 0; i < 8; i++)
	{
		for (uint8_t j = 0; j < 8; j++)
		{
			//LCD_PutValue(Kmal[cyrillicX[n][i]][j]);			
		}
	}
}*/

/* ������� ������ ���������� � ������ x[][16] */
void LCD_SendMassageXY(uint8_t n, uint8_t xAxis, uint8_t yAxis)
{
	writePocketInCGRAM(n);
	LCD_SendCommand(0x80);	//�������� ����� ��������� � DDRAM
	LCD_SetPositionXY(xAxis,yAxis);
	for (uint8_t i = 0; i < 16; i++)
	{
		LCD_SendData(x[n][i]);
	}
}
