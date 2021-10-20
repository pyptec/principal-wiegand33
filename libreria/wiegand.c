#include <reg51.h>
#include "wiegand.h"

sbit automovil  = P1^7;				//Entrada sensor automovil / Cajon Monedero	
sbit D0L1  = P3^2;
sbit D1L1  = P3^3;


#define  	WGND_SIZE  34 //26//49//33
#define STX	2
#define ETX	3



//const unsigned char  	WGND_SIZE=34 ;  //26//49//33
unsigned char nex_bit=0;
unsigned char completo=0;
unsigned char facility_code=0;
unsigned char card_number=0;
unsigned char card_number1=0;
unsigned char card_number2=0;
unsigned char timer_wiegand=0;
unsigned char codebits[WGND_SIZE +1];//+1
unsigned char buffer_wie[4];
extern unsigned char g_scArrTxComSoft[];

/*fuciones prototipo*/
extern void cont(unsigned char caracter);
extern void lcd_puts(unsigned char * str);
extern void vdato (unsigned char caracter)  ;
extern void Borra_all(void);
extern void EscribirCadenaSoft(unsigned char tamano_cadena);
extern int sprintf  (char *, const char *, ...);
extern void wait_long1 (unsigned int t) ;
extern bit tx_bus (unsigned char num_chr);

/*configuracion de bit*/
extern bit Dif_Mot_Car;
/*------------------------------------------------------------------------------
Interrupciones int0 

codebits=array de almacenamiento de la trama en bits del wiegand
nex_bit= es el contador de bits
WGND_SIZE= es el limite de bits a recibir
completo= es cuando termina de recibir la trama de wiegand
bit0=p bit de paridad
bit1 - bit8 = dato de 8bit llamado A
bit9 - bit16 = dato de 8bits llamado B
bit17 - bit 24 = datos de 8 bits  llamado c
bit25 - bit32	= datos de 8 bits llamado D
bit33=p bit de paridad

------------------------------------------------------------------------------*/

void  ex0_isr (void) interrupt 0 
{
	
	
			/*DOL1 - DATA0
	      lee solo los datos del los bits de (0)*/
			
			codebits[nex_bit] = '0';
			nex_bit++;
			while(D0L1==1);
		on_Timer0_Interrup();
		
	  
}
/*------------------------------------------------------------------------------
 interrupciones  int1
------------------------------------------------------------------------------*/	
	
void  ex1_isr (void) interrupt 2 
{
				/*D1L1 - DATA1 
				lee solo los datos del los bits de (1)*/
		
		codebits[nex_bit] = '1';	
		nex_bit++;
		while(D1L1==1);
		on_Timer0_Interrup();
	

}

/*------------------------------------------------------------------------------
interrupcion por timer 
ValTimeOutCom se decrementa cada overflow de la interrupcion
Timer_wait		Incrementa cada overflow de la interrrupcion
clock=22.1184mhz
ciclo de mqn= clock/12 =0.5nseg
timer= ciclo mqn* reloj = 0.5 x65535 =32
temporizado=timer* ValTimeOutCom = 32*100=320ms
------------------------------------------------------------------------------*/
void timer0_int() interrupt 1 using 2
{
			if(timer_wiegand==0)
			{
				timer_wiegand=6;
			completo= 1;
			}
			else
				{
			
			timer_wiegand--;
				}
			TF0=0;
}
void on_Timer0_Interrup(void)
{
	
	TF0=0;									//registro TCON bit 5 bandera de overflow 
	ET0=1;									// enable interrupcion bit 1 del registro IENO
	
}
void off_Timer0_Interrup(void)
{
	ET0=0;									// enable interrupcion bit 1 del registro IENO
	
}
/*------------------------------------------------------------------------------
Habilito interrupciones int0
------------------------------------------------------------------------------*/
void ini_ex0(void)
{

	IT0 = 1	;								/*detecta flanco de HIGH a low*/
	EX0 = 1	;							 /*enable interrrup*/
	
}
/*------------------------------------------------------------------------------
Habilito interrupciones  int1
------------------------------------------------------------------------------*/
void ini_ex1(void)
{

	IT1 = 1	;								/*detecta flanco de HIGH a low*/
	EX1 = 1	;								/*enable interrrup*/
	
}
/*------------------------------------------------------------------------------
Habilito interrupciones int0, int1
------------------------------------------------------------------------------*/
void on_ini_ex0_ex1(void)
{
	EA = 1		;							/*enable las dos interrupciones*/
	
}
/*------------------------------------------------------------------------------
   inicia las interrupciones externas para leer wiegand
------------------------------------------------------------------------------*/
void inicia_wiegand()
{

	ini_ex0();
	ini_ex1();
	on_ini_ex0_ex1();																						/*habilita interrupcion global*/
	limpia_data();
}
/*------------------------------------------------------------------------------
rutina que limpia el buffer de lectura de wiegand
------------------------------------------------------------------------------*/
void limpia_data(void)
{
unsigned char i;
  for(i=0;i<WGND_SIZE+1;i++)
	{	
		codebits[i]=0x00;	 					/*se limpia buffer de bits de wiegand*/
  	nex_bit=0;									/*contador de bits*/
		completo=0;									/*indica que hay un dato de wiegand*/
	}
		facility_code=0;
		card_number=0;
		card_number1=0;
		card_number2=0;
		timer_wiegand=6;
}
/*------------------------------------------------------------------------------
rutina que ajusta la lectura de wiegand
------------------------------------------------------------------------------*/

void ajusta_code(void)
{
	if(nex_bit==34)
	{
		facility_code=bits_wiegand_hex(1);
		card_number=bits_wiegand_hex(9);
		card_number1=bits_wiegand_hex(17);
		card_number2=bits_wiegand_hex(25);
	}
	else
	{
	facility_code=bits_wiegand_hex(1) ;
	card_number=bits_wiegand_hex(9) ;
	card_number1=bits_wiegand_hex(17);
	}
	
}


/*------------------------------------------------------------------------------
rutina que ajusta la lectura de wiegand para 33 bits
------------------------------------------------------------------------------*/

void id_Access()
{

		
		ajusta_code();											// lectura MF50 de 33bits
	if(nex_bit==34)
		{
			buffer_wie[0]=card_number;
			buffer_wie[1]=card_number1;
			buffer_wie[2]=card_number2;
		
		}
		else
		{
			buffer_wie[0]=facility_code;
			buffer_wie[1]=card_number;
			buffer_wie[2]=card_number1;
		}
//		
		lcd_wiegand();

}

/*------------------------------------------------------------------------------
rutina que convierte los bits de lectura de wiegand a hex
bits= es un arreglo donde se realiza una or con cada bit para crear el dato hex 
starting_position= posicion de inicio de analisis del arreglo de bits, para crear el caracter hex

codebits=Lectura de bits del codigo wiegand
------------------------------------------------------------------------------*/

unsigned char  bits_wiegand_hex(unsigned char starting_position)
{
	unsigned char apx_err  []= "ERROR DE LECTURA" ;
	unsigned char i,j,code_wiegand=0;
	unsigned char bits[8]={0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
	i=starting_position+8;
	
  for(j=starting_position ; j < i ; j++)
	{
    	if(codebits[j]=='1')
		{
      	code_wiegand=code_wiegand | bits[j-starting_position];
		}
			
		else if((codebits[j]>'1') || (codebits[j]< '0'))
		 {
			   
			cont(0x80);
 			lcd_puts(apx_err);
			code_wiegand=  0;
			break;	
		 }

	}
	return code_wiegand;
}

/*------------------------------------------------------------------------------
   muestra el numero de la tarjeta leida en el lcd
------------------------------------------------------------------------------*/
 void lcd_wiegand()
 {
 unsigned int temp;
 unsigned char buf[6]={0,0,0,0,0,0};
 unsigned char lectura  []= "LECT.1           " ;
 	Borra_all();									/*borra el lcd*/
 	cont(0x80);						  			/*inicio de la primer ilera del lcd*/
 	lcd_puts(lectura);							/*msj de (LECT.1)*/
	cont(0x87);									/*posicion donde se coloca la parte alta del codigo de la tarjeta*/
					

	temp=buffer_wie[0];						   		
 	sprintf(buf,"%u",temp);		  				/*convierto el hex a un string bcd*/
	lcd_puts(buf);
	vdato('-'); 

	temp=(buffer_wie[1] <<8)| buffer_wie[2] ;  /*uno los dos registros en uno de 16 bits*/
	sprintf(buf,"%u",temp);		  				/*convierto el dato en ascii*/
	lcd_puts(buf);

 }



/*------------------------------------------------------------------------------
Rutina que muestra el valor en hex en el lcd
------------------------------------------------------------------------------*/
/*
void Debug_chr_lcd(unsigned char Dat)
{
	unsigned char temp;
	
		temp=(Dat&0xf0)>>4;
		(temp>0x09)?(temp=temp+0x37):(temp=temp+0x30);
			
		vdato(temp);
							 
		temp=(Dat&0x0f);
		(temp>0x09)?(temp=temp+0x37):(temp=temp+0x30);
		vdato(temp);
		vdato(' ');
	
	
}
*/
