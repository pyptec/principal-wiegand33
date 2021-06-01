#include <reg51.h>
#include "uart.h"

sbit lock2  = P1^5;					//Relevo de Salida (Inhabilitado Proc. Aux usa ERR IMP)	*
sbit lock1  = P1^6;					//Relevo de Entrada		

#define ESPERA_RX				0
//ESTADOR RECEPCION SOFTWARE

#define ESPERA_RX				0
#define VER_DIR					1
#define VER_COMANDO				2
#define POLL_COM_SOF			3
#define WRITE_COM_SOF			4
#define RECEPCION_STR_SOF_STX	5
#define SAVE_STR_SOF			6
#define ANALICE_STR_SOF			7
#define RECEPCION_ID			8


#define TAMANO_RX_COM_SOFT	60	
#define TAMANO_TX_COM_SOFT	50
#define TIMEW		0x1e   				//Tiempo para indicar TimeOut


#define ENQ	5
#define EOT	4
#define ACK	6
#define STX	2
#define ETX	3
#define CR	0x0d
#define LF	0x0a

 //ESTADOR TRANSMICION SOFTWARE

#define SIN_LECTURA_TX	0x00
#define LECTURA_COD_TX	0x01
#define LECTURA_WIEG_TX	0x02
#define COD_PRINT_TX	0x04
#define LPR_TX			0x08   //reconocimiento placa

#define    cte_seg  0x1c

extern unsigned char g_cEstadoComSoft;
extern unsigned char g_cContByteRx;
extern unsigned char g_cDirBoard;
extern unsigned char TimeOutLinea;
extern unsigned char g_cEstadoTxSoft;
extern unsigned char Rechazo;
extern unsigned int TimeOut_Codigo;
extern unsigned char seg,k;
extern  unsigned int TimeOut_Send_Acceso;
extern unsigned int OpenMensual_Apx;

extern unsigned char g_scArrRxComSoft[];
extern unsigned char g_scArrDisplay[];
extern unsigned char g_scArrTxComSoft[];
extern unsigned char xdata buffer_bus[];
extern unsigned char completo;

extern  bit sendactive;
extern  bit FueraLinea;
extern  bit txACK;
extern  bit rx_serie;
extern  bit Tiquete_Placa;
extern  bit Tiquete_Salida;
extern bit SerieOK;
extern bit Tx_Acceso;
extern  bit bandera_rx_soft;

extern bit Send_Wiegand;
extern bit notifyEVP;
extern  bit InhabilitaPulsoEvPOut;
extern bit SalidaW;
/***********************************************************************************************************


//*******************************************************************************************
//																							*
// Function to initialize the serial port and the UART baudrate.							*
//																							*
//*******************************************************************************************
*/
void com_initialize (void) 
{
  	sendactive = 0; 			        // transmitter is not active  						*
  	SCON = 0x50;			         	// serial port MODE 1, enable serial receiver		*
	PCON |= 0x80; 			         	// 0x80=SMOD: set serial baudrate doubler			*
  	TMOD |= 0x20; 			         	// put timer 1 into MODE 2							*
 	TCON = 0x40;    					// TCON 											*
//------------------------------------------------------------------------------------------*
// 	TH1 =  0xFF;    					// TH1 	115000 Bps @ 22.148MHZ						*
// 	TL1 =  0xFF;    					// TH1 												*
//------------------------------------------------------------------------------------------*
 	TH1 =  0xF4;    					// TH1 9600 Bps @ 22.148MHZ							*					 
 	TL1 =  0xF4;    					// TH1 												*
//------------------------------------------------------------------------------------------*
//  TH1 = (unsigned char) (256 - (XTAL / (16L * 12L * baudrate)));							*
//------------------------------------------------------------------------------------------*
   	TR1 = 1; 							// start timer 1
  	ES = 1; 							// enable serial interrupts
}

//*******************************************************************************************
// 	Transmision de un caracter																*
//*******************************************************************************************
 void tx_chr (unsigned char data_com)
 {
	while (sendactive==1) 
	{
	}
   	SBUF=data_com;
	sendactive=1;
 }

/********************************************************************************************
//
// Serial Interrupt Service Routine
//
//*******************************************************************************************/
static void com_isr (void) interrupt 4 using 1 
{
	char cDatoRx;
//------------------------------------------------------------------------------------------*
// Received data interrupt. 																*
//------------------------------------------------------------------------------------------*
	if (RI) 
	{
	  	cDatoRx = SBUF; 			          		// read character
	 	 RI = 0; 									// clear interrupt request flag
	  
		switch (g_cEstadoComSoft)
		{
//---------------------------------------------------------------------------------------
			case ESPERA_RX:
			
				g_cContByteRx=0;
				if(cDatoRx==ENQ)
				{
					g_cEstadoComSoft=VER_DIR;
					
				}
			break;
//--------------------------------------------------------------------------------------
			case VER_DIR:
				if(cDatoRx==g_cDirBoard)
				{
					g_cEstadoComSoft=VER_COMANDO;
					TimeOutLinea=TIMEW;
					FueraLinea=0;
				}
				else
				{
					g_cEstadoComSoft=ESPERA_RX;
				}
				break;
//-----------------------------------------------------------------------------------------
			case VER_COMANDO:
				switch (cDatoRx)
				{
					case 'p':
						g_cEstadoComSoft=POLL_COM_SOF;					//Se atiende en el ciclo principal
						g_cContByteRx=0;
					break;

					case 'e':							  				//Se atiende en el ciclo principal
						g_cEstadoComSoft=RECEPCION_ID;
						g_cContByteRx=0;

					break;
									
					case 'w':
						g_cEstadoComSoft=RECEPCION_STR_SOF_STX;	  		//Se atiende en el ciclo principal
//						txACK=1;
						tx_chr(ACK);
						g_cContByteRx=0;

					break;
				
					case 'S':							  				//Se atiende en el ciclo principal
						g_cEstadoComSoft=RECEPCION_STR_SOF_STX;
						txACK=1;
						rx_serie=1;		
					break;
					case 'c':  											//numero de parqueos disponibles
					   g_cEstadoComSoft=RECEPCION_STR_SOF_STX;	  		//modificado jp
					   tx_chr(ACK);
					   g_cContByteRx=0;
						break;
					default:
						g_cEstadoComSoft=ESPERA_RX;
					break;
				} //switch (cDatoRx)
				break;
//--------------------------------------------------------------------------------------
			case RECEPCION_STR_SOF_STX:
				if(cDatoRx==STX)
				{
					g_cContByteRx=0;
					g_scArrRxComSoft[g_cContByteRx++]=cDatoRx;
					g_cEstadoComSoft=SAVE_STR_SOF;
				}
				else if (cDatoRx==ENQ)
				{
				 	g_cEstadoComSoft=VER_DIR;
					g_cContByteRx=0;
				}
				else
				{
					g_cEstadoComSoft=ESPERA_RX;
				}
			break;
//-----------------------------------------------------------------------------------------
			case RECEPCION_ID:
				g_scArrRxComSoft[g_cContByteRx++]=cDatoRx;
				if(g_cContByteRx>TAMANO_RX_COM_SOFT)
				{
					g_cEstadoComSoft=ESPERA_RX;
				}
				else if(cDatoRx==ETX)
				{
					g_cEstadoComSoft=ESPERA_RX;
					g_cEstadoTxSoft &=~LECTURA_COD_TX;
					Rechazo=0;

					Tiquete_Placa=0;
					Tiquete_Salida=0;

					TimeOut_Codigo=0;
				}
				else if (cDatoRx==ENQ)
				{
				 	g_cEstadoComSoft=VER_DIR;
					g_cContByteRx=0;
				}
				break;						
//-----------------------------------------------------------------------------------------
			case SAVE_STR_SOF:

				g_scArrRxComSoft[g_cContByteRx++]=cDatoRx;
	
				if(g_cContByteRx>TAMANO_RX_COM_SOFT)
				{
					g_cEstadoComSoft=ESPERA_RX;
				}
				else if(cDatoRx==ETX)
				{
					if (rx_serie==1)
					{
						rx_serie=0;
						SerieOK=1;
						for (k=0; k<g_cContByteRx; k++)
						{
						 	g_scArrDisplay[k]=g_scArrRxComSoft[k];
						}
 						g_cEstadoComSoft=ANALICE_STR_SOF;
					}
					
								

					if (g_cContByteRx==6)
					{
//--------------------------------------------------------------------------------------------------------------------------*
						if(g_scArrRxComSoft[1]=='D')
						{
//							lock1=0;

						}
						else if(g_scArrRxComSoft[1]=='A')
						{
						
							if (TimeOut_Send_Acceso!=0)					// Time Out para notificar al secunadrio
							{
								TimeOut_Send_Acceso=0;
								Tx_Acceso=1;						 	// Notifica al Secundario el Acceso
								g_cEstadoTxSoft &=~LECTURA_WIEG_TX;
								OpenMensual_Apx=cte_seg*3;			   	// Un segundo no acepta Acceso Axxx => Software envia 3 intentos;
							}
							else
							{
								if (OpenMensual_Apx==0)					// 
								{
									lock1=1;
									seg=cte_seg;
									TH0=0X00;																						
									TL0=0X00;
									TF0=0;								
								}
							}					
							
										
						}

						if(g_scArrRxComSoft[2]=='D')
						{
//				 			lock2=0;
						}
						else if(g_scArrRxComSoft[2]=='A')
						{
							if (notifyEVP==1)
							{
								Tx_Acceso=1;
								if (InhabilitaPulsoEvPOut==1)
								{
									if ((Send_Wiegand==1))
									{
										Send_Wiegand=0;
										g_cEstadoTxSoft &=~LECTURA_WIEG_TX;
										
										
										if (TimeOut_Send_Acceso!=0)					// Time Out para notificar al secunadrio
										{
											SalidaW=0;

											TimeOut_Send_Acceso=0;
											Tx_Acceso=1;						 	// Notifica al Secundario el Acceso
											g_cEstadoTxSoft &=~LECTURA_WIEG_TX;
											OpenMensual_Apx=cte_seg*3;			   	// Un segundo no acepta Acceso Axxx => Software envia 3 intentos;
										}
										else
										{
											if (OpenMensual_Apx==0)					// 
											{
												lock2=1;
												seg=cte_seg;
												TH0=0X00;																						
												TL0=0X00;
												TF0=0;								
											}
										}															
										
								
//										lock2=1;					// Alvaro Manda Abrir mensual y tiquete pero notifico
//										seg=cte_seg+14;
//										TH0=0X00;					//Inicializa timer										*						
//										TL0=0X00;
//										TF0=0;
									}
	 							}
								else
								{
									if ((Send_Wiegand==1))
									{
										Send_Wiegand=0;
										g_cEstadoTxSoft &=~LECTURA_WIEG_TX;
									}

									lock2=1;						// Alvaro Manda Abrir mensual y tiquete pero notifico
									seg=cte_seg+14;
									TH0=0X00;						//Inicializa timer										*						
									TL0=0X00;
									TF0=0;
								}
							}
							else
							{
									if ((Send_Wiegand==1))
									{
										Send_Wiegand=0;
										g_cEstadoTxSoft &=~LECTURA_WIEG_TX;
									}

									lock2=1;						// Alvaro Manda Abrir mensual y tiquete pero notifico
									seg=cte_seg+14;
									TH0=0X00;						//Inicializa timer										*						
									TL0=0X00;
									TF0=0;
							}
						}
						g_cEstadoComSoft=ESPERA_RX;
						g_cContByteRx=0;
						
//--------------------------------------------------------------------------------------------------------------------------*
	

					}
					else if(g_cContByteRx==7)	  							//modificado jp		tomo la cadena q llega por el serial tcp/ip
					{											   									// y la almaceno en el buffer_bus 
						if(g_scArrRxComSoft[1]=='c')
						{	
						   for (k=0; k<g_cContByteRx; k++)	  			//este for carga la inf pto paralelo p2
						{

							buffer_bus[k]=g_scArrRxComSoft[k];
						 
						}
				/*
						for (k=2; k<g_cContByteRx; k++)	  			//k=2 con lcd
						{

								buffer_Cupo[k-2]=g_scArrRxComSoft[k];	  		//solo usado para almacenar datos para mostrar en lcd
						}
						*/
 						g_cEstadoComSoft=ANALICE_STR_SOF;
				
						}
					}					
				
//------------------------------------------------------------------------------------
					else if	(g_cContByteRx==25)
					{ 
						for (k=0; k<g_cContByteRx; k++)
						{
						 	g_scArrDisplay[k]=g_scArrRxComSoft[k];
						}
 						g_cEstadoComSoft=ANALICE_STR_SOF;

					}
					else if	(g_cContByteRx>=38)
					{

						if(g_scArrRxComSoft[1]=='A')
						{

							
							if (TimeOut_Send_Acceso!=0)
							{
								g_cEstadoTxSoft &=~LECTURA_WIEG_TX;
								TimeOut_Send_Acceso=0;
								Tx_Acceso=1;
								
								OpenMensual_Apx=cte_seg*3;			   	// Un segundo no acepta Acceso Axxx => Software envia 3 intentos;
							}
							else
							{
								if (OpenMensual_Apx==0)					// 
								{
									lock1=1;
									seg=cte_seg;
									TH0=0X00;																						
									TL0=0X00;
									TF0=0;							
								}
							}				
						}
						if(g_scArrRxComSoft[2]=='A')
						{

	
						   	Tx_Acceso=1;
							if (notifyEVP==1)
							{
	
								if (InhabilitaPulsoEvPOut==1)
								{

									if ((Send_Wiegand==1))
									{
										Send_Wiegand=0;
										g_cEstadoTxSoft &=~LECTURA_WIEG_TX;


						  				if (TimeOut_Send_Acceso!=0)					// Time Out para notificar al secunadrio
										{
											SalidaW=0;	
											TimeOut_Send_Acceso=0;
											Tx_Acceso=1;						 	// Notifica al Secundario el Acceso
											g_cEstadoTxSoft &=~LECTURA_WIEG_TX;
											OpenMensual_Apx=cte_seg*3;			   	// Un segundo no acepta Acceso Axxx => Software envia 3 intentos;
										}
										else
										{
											if (OpenMensual_Apx==0)					// 
											{
												lock2=1;
												seg=cte_seg;
												TH0=0X00;																						
												TL0=0X00;
												TF0=0;								
											}
										}			

//										lock2=1;					// Alvaro Manda Abrir mensual y tiquete pero notifico
//										seg=cte_seg+14;
//										TH0=0X00;					//Inicializa timer										*						
//										TL0=0X00;
//										TF0=0;
									}
	 							}
								else
								{
									if ((Send_Wiegand==1))
									{
										Send_Wiegand=0;
										g_cEstadoTxSoft &=~LECTURA_WIEG_TX;
									}

									lock2=1;						// Alvaro Manda Abrir mensual y tiquete pero notifico
									seg=cte_seg+14;
									TH0=0X00;						//Inicializa timer										*						
									TL0=0X00;
									TF0=0;
								}
							}
							else
							{
									if ((Send_Wiegand==1))
									{
										Send_Wiegand=0;
										g_cEstadoTxSoft &=~LECTURA_WIEG_TX;
										lock2=1;						// Alvaro Manda Abrir mensual y tiquete pero notifico
										seg=cte_seg+14;
										TH0=0X00;						//Inicializa timer										*						
										TL0=0X00;
										TF0=0;
									}



							}
						}

						for (k=0; k<38; k++)
						{
						 	g_scArrDisplay[k]=g_scArrRxComSoft[k];
						}
 						g_cEstadoComSoft=ANALICE_STR_SOF;
						bandera_rx_soft=1;
					}
				}
				else if (cDatoRx==ENQ)
				{
				 	g_cEstadoComSoft=VER_DIR;
					g_cContByteRx=0;
				}

				break;
//------------------------------------------------------------------------------------------
			default:
				g_cEstadoComSoft=ESPERA_RX;
			break;
		} //switch (g_cEstadoComSoft) 
	}
//------------------------------------------------------------------------------------------*
// Transmitted data interrupt. 																*
//------------------------------------------------------------------------------------------*
  	if (TI != 0) 
	{
    	TI = 0; 								// clear interrupt request flag
		sendactive=0;
  	}
}

/*------------------------------------------------------------------------------
Transmito un caracter pasandolo a ascii 
------------------------------------------------------------------------------*/
// TEMPORALMENTE FUERA DE SERVICIO
/*
void Debug_chr_uart(unsigned char Dat)
{
	unsigned char temp;

		temp=(Dat&0xf0)>>4;
		(temp>0x09)?(temp=temp+0x37):(temp=temp+0x30);
	
		tx_chr(temp);	
	
							 
		temp=(Dat&0x0f);
		(temp>0x09)?(temp=temp+0x37):(temp=temp+0x30);
		tx_chr(temp);	
		tx_chr(' ');	
	
	
}
*/
/*------------------------------------------------------------------------------
imprime la trama hasta el caracter null
------------------------------------------------------------------------------*/
/*
void Debug_txt_uart(unsigned char * str)
{
	unsigned char i;

	i=0;
	
	
		for (i=0; str[i] != '\0'; i++)
		{
 	  		tx_chr(str[i]);
		}
		
	
}
*/

//**************************************************************************************************************
//**************************************************************************************************************
void EscribirCadenaSoft(unsigned char tamano_cadena)
{
unsigned char i;
 
	for(i=0;i<tamano_cadena;i++)
   	{
        tx_chr(g_scArrTxComSoft[i]);
    }
	
}