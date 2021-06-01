/*********************************************
Diseñado Ing. Jaime Pedraza
Marzo 05 de 2019
*********************************************/
/*
@file		wiegand.h
@brief	header file for delay.c
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_
/*funciones prototipo*/
  void wait (void); 
void relayIO(void);
void wait_long1 (unsigned int t) ;
void wait_long (void);
void wait_ancho (void);
void pulso ()  ;
void cont(unsigned char caracter) ;
void borra();
void prg_disp (void);
void vdato (unsigned char caracter)  ;
void lcd_gotoxy( unsigned char x,   unsigned char y );
void lcd_puts(unsigned char * str);
void lcd_text(unsigned char row, unsigned char offset, unsigned char * lcd);
void ve_dir(void);
void BorraLCD_L1(void);
void BorraLCD_L2(void);
void Borra_all(void);

#endif	/* _DISPLAY_H_ */