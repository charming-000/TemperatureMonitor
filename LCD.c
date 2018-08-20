#include <reg51.h>
#include <intrins.h>
#include <stdio.h>

typedef unsigned int uint;
typedef unsigned char uchar;

/********************���Ŷ���********************/
#define LCD_DATA P0
sbit LCD_RS = P2^0;
sbit LCD_RW = P2^1;
sbit LCD_EN = P2^2;
sbit KEY_1 = P1^7;			//���ܼ�
sbit KEY_2 = P1^4;			//���ټ�
sbit KEY_3 = P1^2;			//���Ӱ���
sbit DS18B20 = P2^6;
/************************************************/

/********************ȫ�ֱ���********************/
#define KeyTime 200					//200 * 1ms = 200ms
#define DispTime 100				//100 * 1ms = 100ms
#define UartTime 150				//150 * 1ms = 150ms
volatile uchar KEY_1ms = 0;
volatile uchar Disp_1ms = 0;
volatile uchar Uart_1ms = 0;
volatile uchar Menu = 0;		//0(��ʾ��ǰ�¶�), 1(��������¶�), 2(��������¶�)
volatile int MaxTemp = 40;
volatile int MinTemp = -10;
volatile float Temperature = 0;
/************************************************/

/********************��������********************/
void Delay_us(uchar x);
void Delay_ms(uchar x);
void Timer0Init(void);
void KEY_Proc(void);
void DispTemp(void);
void LCD_WriteCommand(uchar com);
void LCD_WriteData(uchar dat);
void LCD_Init(void);
void LCD_Show_Home(void);
void LCD_Show_Setting(void);
void DS18B20_Init(void);
uchar DS18B20_ReadByte(void);
void DS18B20_WriteByte(uchar dat);
float DS18B20_ReadTmp(void);
void UartInit(void);
void Uart_Proc(void);
/************************************************/

/*********************������*********************/
int main(void)
{
	LCD_Init();
	Temperature = DS18B20_ReadTmp();
	Delay_ms(200);
	Delay_ms(200);
	UartInit();
	Timer0Init();
	LCD_Show_Home();
	while(1)
	{
		DispTemp();
		KEY_Proc();
		Uart_Proc();
	}
}
/************************************************/

/*****************��ʱ����******************/
void Delay_us(uchar x)		//@12.000MHz
{
	while(x--)
	{
		_nop_();
		_nop_();
		_nop_();
		_nop_();
	}
}

void Delay_ms(uchar x)		//@12.000MHz
{
	uchar i, j;
	while(x--)
	{
		_nop_();
		_nop_();
		i = 12;
		j = 169;
		do
		{
			while (--j);
		} while (--i);
	}
}
/************************************************/

/*******************������0ģ��*******************/
void Timer0Init(void)
{
	TMOD |= 0x01;	//ѡ��Ϊ��ʱ��0ģʽ��������ʽ1������TR0��������
	TH0 = 0xFC;		//����ʱ������ֵ����ʱ1ms
	TL0 = 0x18;	
	ET0 = 1;			//�򿪶�ʱ��0�ж�����
	EA = 1;				//�����ж�
	TR0 = 1;			//�򿪶�ʱ��
}

void Timer0(void) interrupt 1
{
	TH0 = 0xFC;		//����ʱ������ֵ����ʱ1ms
	TL0 = 0x18;
	KEY_1ms ++;
	Disp_1ms ++;
	Uart_1ms ++;
}
/************************************************/

/*****************����ģ��******************/
void KEY_Proc(void)
{
	if (KEY_1ms >= KeyTime)
	{
		KEY_1ms = 0;
		if (KEY_1 == 0)
		{
			Menu ++;
			LCD_Show_Setting();
			if (Menu == 3)
			{
				Menu = 0;
				LCD_Show_Home();
			}
		}
		else if (KEY_2 == 0)
		{
			if (Menu == 1)
			{
				MinTemp --;
			}
			else if (Menu == 2)
			{
				MaxTemp --;
				if ((MaxTemp - 1) == MinTemp)
				{
					MaxTemp ++;
				}
			}
		}
		else if (KEY_3 == 0)
		{
			if (Menu == 1)
			{
				MinTemp ++;
				if ((MaxTemp - 1) == MinTemp)
				{
					MinTemp --;
				}
			}
			else if (Menu == 2)
			{
				MaxTemp ++;
			}
		}
	}
}
/************************************************/

/*****************�¶���ʾ����******************/
void DispTemp(void)
{
	if (Disp_1ms >= DispTime)
	{
		uchar i;
		uchar Disp[6] = "      ";
		uchar Disp1[4] = "    ";
		Disp_1ms = 0;
		if (Menu == 0)
		{
			LCD_WriteCommand(0xC7);
			Temperature = DS18B20_ReadTmp();
			sprintf(Disp, "%.1f", Temperature);
			for(i=0; i<6; i++)
			{
				LCD_WriteData(Disp[i]); 
			}
		}
		else if (Menu == 1)
		{
			LCD_WriteCommand(0x80);
			LCD_WriteData('>');
			LCD_WriteCommand(0xC0);
			LCD_WriteData(' ');
			LCD_WriteCommand(0x8B);
			sprintf(Disp, "%d", MinTemp);
			for(i=0; i<4; i++)
			{
				LCD_WriteData(Disp[i]); 
			}
		}
		else if (Menu == 2)
		{
			LCD_WriteCommand(0x80);
			LCD_WriteData(' ');
			LCD_WriteCommand(0xC0);
			LCD_WriteData('>');
			LCD_WriteCommand(0xCB);
			sprintf(Disp, "%d", MaxTemp);
			for(i=0; i<4; i++)
			{
				LCD_WriteData(Disp[i]); 
			}
		}
	}
}
/************************************************/

/*******************LCDģ��*******************/
void LCD_WriteCommand(uchar com)
{
	LCD_EN = 0;     //ʹ��
	LCD_RS = 0;	  	//ѡ��������
	LCD_RW = 0;	 	  //ѡ��д��
	
	LCD_DATA = com; //��������
	Delay_us(1);		//�ȴ������ȶ�

	LCD_EN = 1;	    //д��ʱ��
	Delay_us(5);	  //����ʱ��
	LCD_EN = 0;
}
	   
	   
void LCD_WriteData(uchar dat)
{
	LCD_EN = 0;			//ʹ������
	LCD_RS = 1;			//ѡ����������
	LCD_RW = 0;			//ѡ��д��

	LCD_DATA = dat; //д������
	Delay_us(1);

	LCD_EN = 1;   	//д��ʱ��
	Delay_us(5);  	//����ʱ��
	LCD_EN = 0;
}

void LCD_Init(void)
{
 	LCD_WriteCommand(0x38);  //����ʾ
	LCD_WriteCommand(0x0C);  //����ʾ����ʾ���
	LCD_WriteCommand(0x06);  //дһ��ָ���1
	LCD_WriteCommand(0x01);  //����
	LCD_WriteCommand(0x80);  //��������ָ�����
}

void LCD_Show_Home(void)
{
	uchar i;
	uchar Disp[16];
	LCD_WriteCommand(0x80);
	sprintf(Disp, " Current Temp:  ");
	for(i=0; i<16; i++)
	{
		LCD_WriteData(Disp[i]); 
	}
	LCD_WriteCommand(0xC0);
	sprintf(Disp, " Temp:        C ");
	for(i=0; i<16; i++)
	{
		LCD_WriteData(Disp[i]); 
	}
	LCD_WriteCommand(0xCD);
	LCD_WriteData(0xDF);
}

void LCD_Show_Setting(void)
{
	uchar i;
	uchar Disp[16];
	LCD_WriteCommand(0x80);
	sprintf(Disp, " Min Temp: %d   ", MinTemp);
	for(i=0; i<16; i++)
	{
		LCD_WriteData(Disp[i]); 
	}
	LCD_WriteCommand(0xC0);
	sprintf(Disp, " Max Temp: %d   ", MaxTemp);
	for(i=0; i<16; i++)
	{
		LCD_WriteData(Disp[i]); 
	}
}
/************************************************/

/*******************DS18B20ģ��*******************/
void DS18B20_Init(void)
{
	DS18B20 = 1;
	Delay_us(1);
	DS18B20 = 0;
	Delay_us(40);
	DS18B20 = 1;
	Delay_us(11);
}

uchar DS18B20_ReadByte(void)
{
 	uchar i,dat=0;
	DS18B20 = 1;
	for(i=0;i<8;i++)
	{
		DS18B20 = 1;
		Delay_us(1);
	 	DS18B20 = 0;
		dat >>= 1;
		DS18B20 = 1;
		if(DS18B20)
			dat |= 0X80;
		Delay_us(2);
	}
	return dat;
}

void DS18B20_WriteByte(uchar dat)
{
 	uchar i;
	for(i=0;i<8;i++)
	{
	 	DS18B20 = 0;
		DS18B20 = dat& 0x01;
		Delay_us(2);
		DS18B20 = 1;
		dat >>= 1;
	}
	Delay_us(2);
}

float DS18B20_ReadTmp(void)
{
	float Temp;
	uint Temp_Value[]={0, 0};
	DS18B20_Init();
	DS18B20_WriteByte(0xCC);
	DS18B20_WriteByte(0x44);
	Delay_us(20);
	DS18B20_Init();
	DS18B20_WriteByte(0xCC);
	DS18B20_WriteByte(0xBE);
	Temp_Value[0] = DS18B20_ReadByte(); 
	Temp_Value[1] = DS18B20_ReadByte();
	if((Temp_Value[1]&0xF8)==0xF8)
	{
		Temp = -1 * (128 - 0.0625 * (Temp_Value[0] | ((Temp_Value[1]&0x07)<<8)));
	}
	else 
	{
		Temp = 0.0625 * (Temp_Value[0] | ((Temp_Value[1]&0x07)<<8));
	}
	return Temp;
}
/************************************************/

/*******************����ͨ��ģ��*******************/
void UartInit(void)
{
	SCON = 0x50;		//����Ϊ������ʽ1
	TMOD |= 0x20;		//���ü�����������ʽ2
	PCON = 0x80;		//�����ʼӱ�
	TH1=0xF3;				//��������ʼֵ���ã�ע�Ⲩ������4800��
	TL1=0xF3;
//	ES=1;						//�򿪽����ж�
//	EA=1;						//�����ж�
	TR1=1;					//�򿪼�����
}

void Uart_Proc(void)
{
//	if (Uart_1ms >= UartTime)
//	{
//		Uart_1ms = 0;
		if (Temperature < MinTemp)
		{
			SBUF = 'A';
			while(!TI);
			TI = 0;
		}
		else if (Temperature > MaxTemp)
		{
			SBUF = 'B';
			while(!TI);
			TI = 0;
		}
		else
		{
			SBUF = 'O';
			while(!TI);
			TI = 0;
		}
//	}
}
/************************************************/