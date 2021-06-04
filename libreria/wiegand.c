#include <reg51.h>
#include "wiegand.h"

sbit automovil  = P1^7;				//Entrada sensor automovil / Cajon Monedero	




#define  	WGND_SIZE  33   //26//49//33
#define STX	2
#define ETX	3



//const unsigned char  	WGND_SIZE=34 ;  //26//49//33
unsigned char nex_bit=0;
unsigned char completo=0;
unsigned char facility_code=0;
unsigned char card_number=0;
unsigned char card_number1=0;
unsigned char card_number2=0;

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
		
		
		
			if(	++nex_bit == WGND_SIZE )
			{
				completo= 1 ;
				codebits[nex_bit+1] = 0 ;
			}
			
	  
}
/*------------------------------------------------------------------------------
 interrupciones  int1
------------------------------------------------------------------------------*/	
	
void  ex1_isr (void) interrupt 2 
{
				/*D1L1 - DATA1 
				lee solo los datos del los bits de (1)*/

		codebits[nex_bit] = '1';	
		
		
		if(	++nex_bit == WGND_SIZE )
		{
			completo= 1;
			codebits[nex_bit+1] = 0;
		}

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
	
}
/*------------------------------------------------------------------------------
rutina que ajusta la lectura de wiegand
------------------------------------------------------------------------------*/

void ajusta_code(void)
{
	facility_code=bits_wiegand_hex(1);
	card_number=bits_wiegand_hex(9);
	card_number1=bits_wiegand_hex(17);
	card_number2=bits_wiegand_hex(25);
}


/*------------------------------------------------------------------------------
rutina que ajusta la lectura de wiegand para 33 bits
------------------------------------------------------------------------------*/

void id_Access()
{

		
		ajusta_code();											// lectura MF50 de 33bits
//	buffer_wie[0]=facility_code;
		buffer_wie[0]=card_number;
		buffer_wie[1]=card_number1;
		buffer_wie[2]=card_number2;
//	buffer_wie[2]=card_number2;
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
 /*
 void tr_wiegand_soft()
 {
 	g_scArrTxComSoft[0]=STX;
	g_scArrTxComSoft[1]='B';
	g_scArrTxComSoft[2]=((buffer_wie[0]>>4)&0X0f)+0X30;
	g_scArrTxComSoft[3]=(buffer_wie[0]&0X0F)+0X30;
	g_scArrTxComSoft[4]=((buffer_wie[1]>>4)&0X0f)+0X30;
	g_scArrTxComSoft[5]=(buffer_wie[1]&0X0f)+0X30;
	g_scArrTxComSoft[6]=((buffer_wie[2]>>4)&0X0f)+0X30;
	g_scArrTxComSoft[7]=(buffer_wie[2]&0X0f)+0X30;
		
	g_scArrTxComSoft[8]=',';
	g_scArrTxComSoft[9]='0';
	
	if (Dif_Mot_Car==1)
	{
		if (automovil==0)
		{
		g_scArrTxComSoft[10]='0';		//Carro =0;
		}
		else
		{
		g_scArrTxComSoft[10]='1';	  	//MOTO = 1
		}
	}
	else
	{
		g_scArrTxComSoft[10]='0';			//Carro =0;
	}
		
	g_scArrTxComSoft[11]=ETX;
	EscribirCadenaSoft(12);

 }
 */
 /*------------------------------------------------------------------------------
inhabilito la interrupcion de entrada int0 y int1
------------------------------------------------------------------------------*/
/*
void off_ini_ex0_ex1(void)
{
	EA = 0 ;										// enable interrupcion bit 1 del registro IENO
	
}
*/
/*------------------------------------------------------------------------------
Ajuste codigo de lectura de 26 bits con complemento a uno
------------------------------------------------------------------------------*/
/*
void ajusta_code_26_complemento(void)
{
	facility_code=bits_wiegand_hex(1) ^ 0xff;
	card_number=bits_wiegand_hex(9) ^ 0xff;
	card_number1=bits_wiegand_hex(17) ^ 0xff;

	
}
*/
/*------------------------------------------------------------------------------
Ajuste codigo de lectura de 26 bits 
------------------------------------------------------------------------------*/
/*
void ajusta_code_26_normal(void)
{
	facility_code=bits_wiegand_hex(1) ;
	card_number=bits_wiegand_hex(9) ;
	card_number1=bits_wiegand_hex(17);

	
}
*/

/*------------------------------------------------------------------------------
rutina que ajusta la lectura de wiegand para 33 bits
------------------------------------------------------------------------------*/
/*
void id_Access_33()
{
	
	ajusta_code_26_normal();											// lectura MF50 de 33bits
	//buffer_wie[0]=facility_code;
	
	buffer_wie[0]=card_number;
	buffer_wie[1]=card_number1;
	buffer_wie[2]=card_number2;
	lcd_wiegand();

	
}
*/
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
/*
void lcd_debuger()
{
	ajusta_code_26_normal();
	buffer_wie[0]=facility_code;
	buffer_wie[1]=card_number;
	buffer_wie[2]=card_number1;
	
	Borra_all();									/*borra el lcd*/
/* 
cont(0x80);	
	Debug_chr_lcd(facility_code);
	Debug_chr_lcd(card_number);
	Debug_chr_lcd(card_number1);
	Debug_chr_lcd(card_number2);
	
}
*/
