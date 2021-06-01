#include <reg51.h>
#include "display.h"
										  //********************************************************************************
//	RUTINAS DE DISPLAY DMC 2 LINEAS 20 CHR
//	MEDIO BYTE DE DATOS
//********************************************************************************
sbit RS = P1^0 ;   								/* define I/O functions 		*/
sbit E =  P1^1 ;							    /* P3.5 						*/
sbit sw_1 = P1^2;					//Direccion												*
sbit sw_2 = P1^3;					//Direccion		

extern bit audio1;
extern bit audio2;
extern bit audio3;
extern bit audio4;

extern unsigned char g_cRelevos;
extern  unsigned char g_cDirBoard;

#define RELEVO1		0x01
#define RELEVO2		0x02
#define RELEVO3		0x04
#define RELEVO4		0x08
//*********************************************************************************
void wait (void)  {                   			/* wait function 				*/
  ;                                   			
}
//*********************************************************************************
//*********************************************************************************
void relayIO(void)
{
	g_cRelevos=0x00;

	if (audio1==1)
	{
		g_cRelevos |= RELEVO1;
	}
	else if (audio2==1)
	{
		g_cRelevos |= RELEVO2;
	}

	else if (audio3==1)
	{
		g_cRelevos |= RELEVO3;
	}
	else if (audio4==1)
	{
		g_cRelevos |= RELEVO4;
	}
}
 void wait_long1 (unsigned int t)  {                   		/* wait function 				*/
  	unsigned int j;
 	for (j=0; j<t; j++)    						// 2
	{
		wait();
	}   
}	
//*********************************************************************************
void wait_long (void)  {                   		/* wait function 				*/
  	unsigned int j;
 	for (j=0; j<2; j++)    						// 2
	{
		wait();
	}   
}	
//*********************************************************************************
void wait_ancho (void)  {                   		/* wait function 			*/
  	unsigned int j;
 	for (j=0; j<2; j++)    						//
	{
		wait();
	}   
}	
//********************************************************************************
void pulso ()  
{                   
//  	E=0;
// 		wait_long();
		E=1;
		wait_long();
		E=0;

}
//********************************************************************************
void cont(unsigned char caracter)  		   		/* DIRECCION					*/ 
{ 			                 
  	RS=0;
	relayIO();
  	P0= (caracter&0xf0)|g_cRelevos; 								//| 0x0f;
  	pulso();
  	caracter <<=4;
  	P0= (caracter&0xf0)|g_cRelevos; 								//| 0x0f;
  	pulso();
 }
 //********************************************************************************
void borra()
{
  	unsigned int i;
   	cont(0x01);
  	for (i=0; i<250; i++)					   // 250
  	{
		wait();
  	}
}
//*********************************************************************************
/* 							PROGRAMA EL DISPLAY									 */
//*********************************************************************************
void prg_disp (void)  	 						
 {
 	RS=0;
	wait_long();				////////////
	E=0;
	wait_long();
//	wait_long();				/////////////
	P0=0x39; 					// 0x38 FUNCTION SET 8 BIT INTERFACE  +1 Bit MSB es TXD
	pulso();
	wait_long();				// WAIT 4.1 MS
 	pulso();					// FUNCTION SET 8 BIT INTERFACE
	wait_long();				// WAIT 100 uS
 	pulso();					// Function Set Command: (8-Bit interface)
	wait_long();				// After this command is written, BF can be checked.
 
  	P0=0x29;					// 0x28 Function Set: Sets interface to 4 -bit  +1 Bit MSB es TXD
	pulso();
	wait_long();
 	cont(0x0c);					// Display ON/OFF Control 0 0 0 0 1 D C B	 
								// The display is ON when D=1 and OFF when D=0 -- C=0: Cursor OFF -- B=0: Blink OFF
	borra();					// Clear Display  0x01
 	wait();
	cont(0x06);					// Display ON  0 1 I/D S  I/D=1: +1(Increment Mode)	 S=0: No Display Shift operation
	wait();
	cont(0x80);					// 
	wait_long();
//	wait_long();				/////////////
		
	relayIO();
	P0=g_cRelevos;
 }
//********************************************************************************
void vdato (unsigned char caracter)  
{                   					
  	RS=1;
	wait();
	relayIO();

   	P0= (caracter&0xf0)|g_cRelevos; 		//| 0x0f;
  	pulso();
  	caracter <<=4;
  	P0= (caracter&0xf0)|g_cRelevos;;		// | 0x0f;
	pulso();

 }

void lcd_gotoxy( unsigned char x,   unsigned char y )
{
  switch(y)
    {
    case 0 : cont(0x80 + x); break;
    case 1 : cont(0xC0 + x); break;
    case 2 : cont(0x94 + x); break;
    case 3 : cont(0xD4 + x); break;
    }
}



void lcd_puts(unsigned char * str)
{
	unsigned char xdata i;
	for (i=0; str[i] != '\0'; i++)
	{

	  vdato(str[i]);

	}

}
 void lcd_text(unsigned char row, unsigned char offset, unsigned char * lcd)
{
  lcd_gotoxy(0+offset,row);
  lcd_puts((char*)lcd);
}

void ve_dir(void)
{
	sw_1=0;
	sw_2=0;

	wait_long();
    g_cDirBoard=0x31;

	if (sw_1==1)
	{
		g_cDirBoard++;
	}
	if (sw_2==1)
	{
		g_cDirBoard=g_cDirBoard+2;
	}
	cont(0xc9);
	vdato(g_cDirBoard);
}
//*******************************************************************************************
//**************************************************************************************************************
void BorraLCD_L1(void)
{
	unsigned char k;

	cont(0x80);
	for (k=0; k<16; k++)								//	Limpia Primera Linea de Display
	{
	  	vdato(' ');
	}
}
//**************************************************************************************************************
void BorraLCD_L2(void)
{
	unsigned char k;

	cont(0xc0);
	for (k=0; k<16; k++)								//	Limpia Primera Linea de Display
	{
	  	vdato(' ');
	}
}
void Borra_all(void)
{
	BorraLCD_L1	 ();
	BorraLCD_L2	  ();
}