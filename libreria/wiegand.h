/*********************************************
Diseñado Ing. Jaime Pedraza
Marzo 05 de 2019
*********************************************/
/*
@file		wiegand.h
@brief	header file for delay.c
*/

#ifndef _WIEGAND_H_
#define _WIEGAND_H_
/*funciones prototipo*/
//#define  	WGND_SIZE  33   //26//49//33
void ini_ex0(void);
void ini_ex1(void);
void on_ini_ex0_ex1(void);
void off_ini_ex0_ex1(void);
void limpia_data(void);
unsigned char  bits_wiegand_hex(unsigned char starting_position);
void ajusta_code(void);
void id_Access();
void lcd_wiegand();
void inicia_wiegand();
void tr_wiegand_soft();
void lcd_debuger();
void Debug_chr_lcd(unsigned char Dat);
void ajusta_code_26_complemento(void);
void ajusta_code_26_normal(void);
void id_Access_33();
#endif	/* _WIEGAND_H_ */