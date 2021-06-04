#include <at89c51xd2.h>
						
#include <stdio.h>					//														*
#include <INTRINS.H>				//														*
#include <math.h>                   //														*          
#include <string.h>
#include "display.h"				//														*
#include "wiegand.h"
#include "uart.h"
//*******************************************************************************************
//	DEFINICION DE IO DEL MICROCONTROLADOR													*
//*******************************************************************************************
//sbit sw_1 = P1^2;					//Direccion												*
//sbit sw_2 = P1^3;					//Direccion												*
sbit ledv = P1^4;					//Led del boton expedidor								*
sbit lock1  = P1^6;					//Relevo de Entrada										*
sbit lock2  = P1^5;					//Relevo de Salida (Inhabilitado Proc. Aux usa ERR IMP)	*

sbit msg1  = P0^0;					//Relevo de Salida (AUDIO)								*
sbit msg2  = P0^1;					//Relevo de Salida (AUDIO)								*
sbit msg3  = P0^2;					//Relevo de Salida (AUDIO)								*
sbit msg4  = P0^3;					//Relevo de Salida (AUDIO)								*

sbit automovil  = P1^7;				//Entrada sensor automovil / Cajon Monedero				*
sbit SignalAcceso = P3^7;	 		//Sigue la señal de Sensor								*
sbit busy=P3^5;		  				//														*
sbit ready=P3^4;					//														*
sbit bus_clk=P3^6;					//	
//sbit RS = P1^0 ;   								/* define I/O functions 		*/
//sbit E =  P1^1 ;							    /* P3.5 						*/


//												*/
//*******************************************************************************************
 unsigned char sin_sensor  	[]= "ERROR:  LOOP 1  " ;
 unsigned char linea        []= " FUERA DE LINEA " ;
 unsigned char err_mifare   []= " ERROR MIFARE   " ;
 unsigned char err_cod      []= "ERROR COD. PARQ." ;
 unsigned char err_in       []= "  SIN INGRESO   " ;
 unsigned char err_sinpago  []= "NO REGISTRA PAGO" ;
 unsigned char err_gracia   []= "EXCEDE T.GRACIA " ;

 unsigned char MSGnegado    []= " ACCESO NEGADO  " ;
 unsigned char err_out      []= "   SIN SALIDA   " ;
 unsigned char err_data     []= "ERROR GRABACION " ;
 unsigned char err_Caja     []= "ACERQUESE A CAJA" ;
 unsigned char serie        []= "SERIE:          " ;
 unsigned char matchPlate   []= "COMPARANDO PLACA" ;
 unsigned char tarjeta_venc []= "TARJETA VENCIDA " ;
//*******************************************************************************************	

#define    cte_seg  0x1c
#define    cte_retry  0x2a


 unsigned char seg,k;

 unsigned char g_cContByteRx=0;
 unsigned char g_cDirBoard=0x31;

 unsigned char num_chr;
 unsigned char g_cRelevos;
 unsigned int TimeOut_Codigo;
 unsigned int TimeOut_Wiegand;
 unsigned char TimeOutLinea;
 unsigned char len_buffer;
 unsigned char num_data;

 unsigned int iTimeEsperaRtaLPR=0;

 unsigned char Ini_Fecha;
 unsigned char Ini_Dcto;

 unsigned char YearIn;
 unsigned char MonthIn;
 unsigned char DayIn;
 unsigned char HourIn;
 unsigned char MinutIn;

 unsigned char YearOut;
 unsigned char MonthOut;
 unsigned char DayOut;
 unsigned char HourOut;
 unsigned char MinutOut;

 unsigned char Tipo_Vehiculo;
 unsigned char xTipo_Vehiculo;
 unsigned int TimeOut_Send_Acceso=0;

 unsigned char NumDatRetry=0;

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

//ESTADOR TRANSMICION SOFTWARE

#define SIN_LECTURA_TX	0x00
#define LECTURA_COD_TX	0x01
#define LECTURA_WIEG_TX	0x02
#define COD_PRINT_TX	0x04
#define LPR_TX			0x08   //reconocimiento placa

unsigned char g_cEstadoComSoft=ESPERA_RX;
unsigned char g_cEstadoTxSoft=SIN_LECTURA_TX;




unsigned char g_scArrRxComSoft[TAMANO_RX_COM_SOFT];
unsigned char g_scArrTxComSoft[TAMANO_TX_COM_SOFT];

unsigned char g_scArrDisplay[32];

unsigned char xdata buffer_bus[30];

 unsigned char xdata BufferRetry[20];


 unsigned char xdata buffer_ticket[20];

 unsigned char xdata buffer_placa[10];

 extern unsigned char xdata buffer_wie[];
 unsigned char xdata buffer_wieLPR[4];
 unsigned char xdata buffer_Cupo[5];		//buffer donde almaceno el numero de cupos jp



 unsigned char 	NumDatosPlate=0;
 unsigned char 	Max_Len_Placa=0;
 unsigned char 	NumChrTicket=0;
 unsigned char  Num_Char_LPR=0;

 unsigned char temp;
 unsigned char Rechazo;
 unsigned char Seg_OFF=5;
 unsigned char TimeRetryCmd=0;
 char nbitsW;
 unsigned char Cod_Dcto;
 
 
 unsigned int OpenMensual_Apx=0;

#define ENQ	5
#define EOT	4
#define ACK	6
#define STX	2
#define ETX	3
#define CR	0x0d
#define LF	0x0a

#define GRACIAS 		0X01
#define BIENVENIDO 		0X02
#define NO_REGIST		0X04
#define LOT_FULL		0X05
#define	ERROR_LOOP	0x06
#define	EXCEDE_HORARIO	0X07
#define	MENSUAL_NO_PAGO	0X08
#define UN_MOMENTO			0X09

#define SOLICITA_EVN 	0XAA

#define NO_MENSUAL 	0XB1
#define NO_IN_PARK 	0XB2
#define IN_PARK 	0Xb3
#define EXPIRO	 	0Xb4
#define HORARIO	 	0Xb5
#define OFF_LINE	0XB6
#define ON_LINE		0XB7
#define IN_HORARIO	0Xb8
#define DiaX		0XB9

#define ACCESO		0Xab


#define PICO_PLACA		0XF3
#define RECHAZADA		0XF9
#define SIN_TARJETAS    0XFA


#define TIMEW		0x1e   				//Tiempo para indicar TimeOut
#define T_MS		20   				// Base para 1ms en Espera a tx Bus
#define MAX_CHR		30   				// Maximo Numero CHR a recibir del secundario


#define LOOP	0
#define CARD	1


 bit sendactive=0; 			         	// flag: marks transmitter active
 bit rx_dat=0;
 bit toggle=0;

 bit rx_serie=0;
 bit bandera_rx_soft=0;
 bit txACK=0;

 bit inicio_wiegand;
 bit lectura_wiegand;
// bit flanco_wiegand;

 bit audio1=0;
 bit audio2=0;							  
 bit audio3=0;
 bit audio4=0;

 bit msg_error=0;			 
 bit notifyEVP;
 bit retry;
 bit FueraLinea=0;
 bit ACCESO_USE_LPR;
 bit Flag_Dcto=0;
 bit datos_validos=0;
 bit Dif_Mot_Car=0;
 bit send_markCashierAut;
 bit Send_Wiegand=0;
 bit InhabilitaPulsoEvPOut=0;
 bit Send_Tipo_Veh=CARD;		//modificado 23/01/2019 jp					LOOP;
 bit SerieOK=0;
 bit Dato_OffLine=0;
 bit Off_Line_Salida=0;
 bit Atascado=0;
 bit Habilita_Lectura=1;

 bit Dato_Placa=0;
 bit Tiquete_Placa=0;
 bit Tiquete_Salida=0;

 bit Tx_Acceso=0;
 bit SalidaW=0;
 bit Central_ID_OFFLINE=0;
 bit OrigenTipoVeh=CARD;
 bit ErrorTx=0;
 bit RetryCmd=0;


extern unsigned char completo;
extern 	unsigned char codebits[];
extern 	unsigned char nex_bit;
extern	unsigned char facility_code;
extern	unsigned char	card_number;
extern	unsigned char card_number1;
extern	unsigned char card_number2;

//*******************************************************************************************
void Pulso_Bus(void)
{
	bus_clk=0;
	wait_ancho();
	wait_ancho();
	wait_ancho();
	bus_clk=1;
	wait_ancho();
	wait_ancho();
}

//**************************************************************************************************************
bit tx_bus (unsigned char num_chr)
{
  	unsigned char j;
	long int cont;
	bit timeOut;

	bus_clk=1;						// Asegura pulso de Clk en 1
	timeOut=0;						// Borra Bandera de Time Out
	busy=0;							// Genera Peticion de Atencion al Secundario
	cont=T_MS*200;					// 500 ms  cambio echo jp 100 por 200
//----------------------------------------
 	while ((ready==1)&&(timeOut==0))
	{
		cont--;
		if (cont==0)
		{
			timeOut=1;
			bus_clk=1;
			ready=1;
 			busy=1;
 		}
	}
//---------------------------------------
   	if (timeOut==0)
	{
		for (j=0; j<num_chr; j++)
		{
   			P2=buffer_bus[j];
			Pulso_Bus();
		}

	}
//---------------------------------------
	bus_clk=1;
//	ready=1;
 	busy=1;
	
	if (timeOut==0)
	{
		wait_long();
		wait_long();
		wait_long();
		wait_long();
//		wait_long();
	}
	return timeOut;
}

//**************************************************************************************************************

//**************************************************************************************************************
//*******************************************************************************************
void send_dataCLK(unsigned char fc, dir)
{
	unsigned int valor;
	unsigned char centena, decena, unidad;
	valor=0;

	centena=0;
	decena=0;
	unidad=0;
	 
 	while (fc>=0x064) 				// resto 100
	{
		fc=fc-0x64;
		centena=centena+1;
	}
	while (fc>=0x0a)				// resto 10
	{
		fc=fc-0x0a;
		decena=decena+1;
	}
	unidad=fc;

//	vdato(centena|0x30);
	g_scArrTxComSoft[dir]=(decena|0x30);
   	g_scArrTxComSoft[dir+1]=(unidad|0x30);
}

//**************************************************************************************************************
void relevos_aux(void)
{
	(audio1==1)?(msg1=1):(msg1=0);
	(audio2==1)?(msg2=1):(msg2=0);
	(audio3==1)?(msg3=1):(msg3=0);
	(audio4==1)?(msg4=1):(msg4=0);
}
//**************************************************************************************************************
//********************************************************************************************
void rx_bus (void)
{
	long int count;
	unsigned char timeOut,j=0;


	
	busy=1;
	num_data=0;
	if (ready==0)
	{
		timeOut=0;
		busy=0;
		for (j=0; j<MAX_CHR; j++)
		{									 //100000
			count=60;
			while ((bus_clk==1)&&(ready==0)&&(timeOut==0))
			{
				count--;
				if (count==0)
				{
					timeOut=1;
					j=MAX_CHR+1;
				}		
			}	 
			if ((bus_clk==0)&&(timeOut==0)&&(ready==0))
			{
				buffer_bus[j]=P2;
				num_data++; 
				count=35;				//50000
				while ((bus_clk==0)&&(timeOut==0))
				{
					buffer_bus[j]=P2;
					count--;
					if (count==0)
					{
						timeOut=1;
						j=MAX_CHR+1;
					}				
				}
			}
			if (ready==1)
			{
				j=MAX_CHR+1;
			}
		}
  	}
	busy=1;	   

}

//**************************************************************************************************************//**************************************************************************************************************
//*******************************************************************************************
//		FUNCIONES PARA VISUALIZAR FECHA HORA												*
//*******************************************************************************************
 void vdata_clk (unsigned char data_clk) 
 {
 
 unsigned int temp;

 temp=data_clk;
 temp=temp & 0xf0;
 temp>>=4;
 temp=temp|0x30;
 vdato(temp);

 temp=data_clk;
 temp=temp & 0x0f;
 temp=temp|0x30;
 vdato(temp);

}

//*******************************************************************************************
void SendMsg(unsigned char tipo)
{
	unsigned char i;


		buffer_bus[0]=STX;
		switch (tipo)
		{
			case GRACIAS:
				buffer_bus[1]='V';			
	
			break;
	
	
			case BIENVENIDO:
	 			buffer_bus[1]='O';
	 
	 		break;
		}
	 
		for(i=21; i<=37; i++)
		{
			buffer_bus[i-19]=g_scArrRxComSoft[i];
	 	}
		buffer_bus[18]=ETX;
		
		

		if (ready==1)
		{
			ErrorTx=tx_bus(19);
			if (ErrorTx==1)
			{
				for (i=0; i<19; i++)
				{
					BufferRetry[i]=buffer_bus[i];
				}
				NumDatRetry=19;
				TimeRetryCmd=cte_seg;
			}
		}
		else
		{
 			for (i=0; i<19; i++)
			{
				BufferRetry[i]=buffer_bus[i];
			}
			NumDatRetry=19;
			TimeRetryCmd=cte_seg;
			
		}

							

}
//**************************************************************************************************************
void SendRtaBus (unsigned char Respuesta)
{
	if (ready==1)
	{
		buffer_bus[0]=Respuesta;		
		ErrorTx=tx_bus(1);
		if (ErrorTx==1)
		{
			NumDatRetry=1;
			BufferRetry[0]=Respuesta;
			TimeRetryCmd=cte_retry;
		}			
	}
	else
	{
		NumDatRetry=1;
		BufferRetry[0]=Respuesta;
		TimeRetryCmd=cte_retry;
	}	
}
//**************************************************************************************************************
//
//**************************************************************************************************************
void AtencComSoft(void)
{

	char i, longTx;
	switch (g_cEstadoComSoft)
	{
//**************************************************************************************************************
		case POLL_COM_SOF:
//--------------------------------------------------------------------------------------------------------------
//	COLOCAR BUFER DE TRANSMICION INDEPENDIENTE PARA CADA SITUACION
//--------------------------------------------------------------------------------------------------------------
			if (g_cEstadoTxSoft==SIN_LECTURA_TX)
			{				
				tx_chr(EOT);	 											//No hay Novedad 
				g_cEstadoComSoft=ESPERA_RX;
				
				if (SignalAcceso==1)
				{
					buffer_bus[0]=SOLICITA_EVN;
					tx_bus(1);
				}		
				
			}
//--------------------------------------------------------------------------------------------------------------
			else if ((g_cEstadoTxSoft&LECTURA_COD_TX)==LECTURA_COD_TX)					//Lectura Cod Barras
			{
 		   		
				if (Tiquete_Placa==1)
				{
					longTx=0;
					g_scArrTxComSoft[longTx++]=STX;
					(send_markCashierAut==0)?(g_scArrTxComSoft[longTx++]='a'):(g_scArrTxComSoft[longTx++]='C');
					
					for (i=0; i<NumChrTicket; i++)
				  	{
						g_scArrTxComSoft[longTx++]=buffer_ticket[i];
	 			  	}
					g_scArrTxComSoft[longTx++]=',';
					if (Tiquete_Salida==1)
					{
						g_scArrTxComSoft[longTx++]=Cod_Dcto;
					}
					else
					{
						g_scArrTxComSoft[longTx++]='0';
					}
					
					
					if (OrigenTipoVeh==CARD)
					{
						if (Tipo_Vehiculo=='H')					// Heavy
						{
						 	g_scArrTxComSoft[longTx++]='0';		// Carro=0
						}
						else if	(Tipo_Vehiculo=='T') 			// Truck
						{
							g_scArrTxComSoft[longTx++]='0';		// Carro=0
						}
						else if(Tipo_Vehiculo=='M')				// Motocycle
						{
							g_scArrTxComSoft[longTx++]='1';		// Moto=1
						}
						else if(Tipo_Vehiculo=='C')				// Carro
						{
							g_scArrTxComSoft[longTx++]='0';		// Carro=0			modifica 24/01/2019    no  funciono
						}
						else
						{
						 	
							if (Dif_Mot_Car==1)
							{
								if (automovil==0)
								{
									if(SignalAcceso==0)							 //	 Se modifica 24/01/2019
									{
									g_scArrTxComSoft[longTx++]='0';				// Carro=0	
									}else	{ g_scArrTxComSoft[longTx++]='1';}		// Moto=1
								}	   
								else
								{
									g_scArrTxComSoft[longTx++]='1';		// Moto=1
								}
							}
							else
							{
								g_scArrTxComSoft[longTx++]='0';		// Carro=0		 modificado 24/01/2019
							}
 							
						}
					}
					else
					{

							if (Dif_Mot_Car==1)
							{
								if (automovil==0)
								{
									if(SignalAcceso==0)							 //	 Se modifica 24/01/2019
									{
									g_scArrTxComSoft[longTx++]='0';		// Carro=0	modificado jp 24/01/2019   no funciona
									}else	{ g_scArrTxComSoft[longTx++]='1';}
								}
								else
								{
									g_scArrTxComSoft[longTx++]='1';		// Moto=1
								}
							}
							else
							{
								g_scArrTxComSoft[longTx++]='0';		// Carro=0		 modificado jp 24/01/2019	nofunciona
							}
					}
					

					g_scArrTxComSoft[longTx++]=',';

					send_dataCLK((YearIn-0x30),longTx);
					longTx=longTx+2;
					send_dataCLK((MonthIn-0x30),longTx);
					longTx=longTx+2;
					send_dataCLK((DayIn-0x30),longTx);
					longTx=longTx+2;
					send_dataCLK((HourIn-0x30),longTx);
					longTx=longTx+2;
					send_dataCLK((MinutIn-0x30),longTx);
					longTx=longTx+2;

					
					if (send_markCashierAut==0)
					{
						g_scArrTxComSoft[longTx++]=',';
						if (Tiquete_Salida==1)
						{
	
							send_dataCLK((YearOut-0x30),longTx);
							longTx=longTx+2;
							send_dataCLK((MonthOut-0x30),longTx);
							longTx=longTx+2;
							send_dataCLK((DayOut-0x30),longTx);
							longTx=longTx+2;
							send_dataCLK((HourOut-0x30),longTx);
							longTx=longTx+2;
							send_dataCLK((MinutOut-0x30),longTx);
							longTx=longTx+2;
							g_scArrTxComSoft[longTx++]=',';
	
							g_scArrTxComSoft[longTx++]=' ';
	
							g_scArrTxComSoft[longTx++]=',';
							g_scArrTxComSoft[longTx++]=ETX;					
							EscribirCadenaSoft(longTx);			
	
						}
						else
						{
						 	g_scArrTxComSoft[longTx++]='0';
							g_scArrTxComSoft[longTx++]='0';
							g_scArrTxComSoft[longTx++]='0';
							g_scArrTxComSoft[longTx++]='0';
							g_scArrTxComSoft[longTx++]='0';
							g_scArrTxComSoft[longTx++]='0';
							g_scArrTxComSoft[longTx++]='0';
							g_scArrTxComSoft[longTx++]='0';
							g_scArrTxComSoft[longTx++]='0';
							g_scArrTxComSoft[longTx++]='0';
	
							g_scArrTxComSoft[longTx++]=',';
							for (i=0; i<Num_Char_LPR; i++)
							{
								g_scArrTxComSoft[longTx++]=buffer_placa[i];
							}
							g_scArrTxComSoft[longTx++]=',';
							g_scArrTxComSoft[longTx++]=ETX;					
							EscribirCadenaSoft(longTx);			
						}

					}
					else
					{
						g_scArrTxComSoft[longTx++]=ETX;
						EscribirCadenaSoft(longTx);
					}
		

				}
				else
				{
					g_scArrTxComSoft[0]=STX;
					
					if (Rechazo==0)
					{
						(send_markCashierAut==0)?(g_scArrTxComSoft[1]='A'):(g_scArrTxComSoft[1]='C');
					}
					else
					{
						g_scArrTxComSoft[1]=Rechazo;
					}
					
					
					longTx=len_buffer;
					for (i=0; i<len_buffer; i++)
				  	{
						g_scArrTxComSoft[i+2]=buffer_ticket[i];
	 			  	}
					g_scArrTxComSoft[longTx+2]=',';
	//--------------------------------------------------------------------
					if (Flag_Dcto==1)
					{
						g_scArrTxComSoft[longTx+3]=Cod_Dcto;
					}
					else
					{					
						g_scArrTxComSoft[longTx+3]='0';
					}
	//-------------------------------------------------------------------------------
					if (Send_Tipo_Veh==CARD)
					{
						if (Tipo_Vehiculo=='H')					// Heavy
						{
						 	xTipo_Vehiculo=4;
						}
						else if	(Tipo_Vehiculo=='T') 			// Truck
						{
							xTipo_Vehiculo=3;
						}
						else if(Tipo_Vehiculo=='B')	  			// Bycicle
						{
							xTipo_Vehiculo=2;
						}
						else if(Tipo_Vehiculo=='M')				// Motocycle
						{
							xTipo_Vehiculo=1;
						}
						else if(Tipo_Vehiculo=='C')				// Carro
						{
							xTipo_Vehiculo=0;
						}
						else									// Sin categoria
						{
	
							if (Dif_Mot_Car==1)
							{
								if (automovil==0)
								{
									xTipo_Vehiculo=0;			//Carro =0;
								}
								else
								{
									xTipo_Vehiculo=1;	  		//MOTO = 1
								}
							}
							else
							{
								xTipo_Vehiculo=0;				//Carro =0;
							}
	   					
	 					}
						g_scArrTxComSoft[longTx+4]=xTipo_Vehiculo+0X30;
	 				}
					else
					{
						if (Dif_Mot_Car==1)
						{
							if (automovil==0)
							{
								g_scArrTxComSoft[longTx+4]='0';		//Carro =0;		modificado 23/01/2019 jp
							}
							else
							{
								g_scArrTxComSoft[longTx+4]='1';	  	//MOTO = 1
							}
						}
						else
						{
							g_scArrTxComSoft[longTx+4]='0';		//Carro =0;
						}
	  				}
	//--------------------------------------------------------------------------------					
					if (send_markCashierAut==1)
					{
						g_scArrTxComSoft[longTx+5]=',';
	
						longTx=longTx+6;
						send_dataCLK((YearIn-0x30),longTx);
						longTx=longTx+2;
						send_dataCLK((MonthIn-0x30),longTx);
						longTx=longTx+2;
						send_dataCLK((DayIn-0x30),longTx);
						longTx=longTx+2;
						send_dataCLK((HourIn-0x30),longTx);
						longTx=longTx+2;
						send_dataCLK((MinutIn-0x30),longTx);
						longTx=longTx+2;
						g_scArrTxComSoft[longTx]=ETX;
						EscribirCadenaSoft(longTx+1);
	
	//					g_scArrTxComSoft[longTx+6]=YearIn;		
	//					g_scArrTxComSoft[longTx+7]=MonthIn;		
	//					g_scArrTxComSoft[longTx+8]=DayIn;		
	//					g_scArrTxComSoft[longTx+9]=HourIn;		
	//					g_scArrTxComSoft[longTx+10]=MinutIn;		
	//					g_scArrTxComSoft[longTx+11]=ETX;	  
	//				   	EscribirCadenaSoft(longTx+12);
		 
					}
					else
					{
						g_scArrTxComSoft[longTx+5]=ETX;	  
					   	EscribirCadenaSoft(longTx+6);
	
					}
				}
				
				

//				send_markCashierAut=0;
  				g_cEstadoComSoft=ESPERA_RX;
 			}
//-----------------------------------------------------------------------------------------------------------------
			else if	((g_cEstadoTxSoft&LECTURA_WIEG_TX)==LECTURA_WIEG_TX) 				// Lectura Wiegand Lector 1
			{
  					Send_Wiegand=1;
					seg=cte_seg;
					TH0=0X00;																						
					TL0=0X00;
					TF0=0;

					if (Dato_OffLine==0)
					 {
						if (SalidaW==1)
						{
						 	//SalidaW=0;
//---------------------;

						g_scArrTxComSoft[0]=STX;
						g_scArrTxComSoft[1]='w';
						g_scArrTxComSoft[2]=((buffer_wie[0]>>4)&0X0f)+0X30;
						g_scArrTxComSoft[3]=(buffer_wie[0]&0X0F)+0X30;
						g_scArrTxComSoft[4]=((buffer_wie[1]>>4)&0X0f)+0X30;
						g_scArrTxComSoft[5]=(buffer_wie[1]&0X0f)+0X30;
						g_scArrTxComSoft[6]=((buffer_wie[2]>>4)&0X0f)+0X30;
						g_scArrTxComSoft[7]=(buffer_wie[2]&0X0f)+0X30;
	
					   	g_scArrTxComSoft[8]=',';
						g_scArrTxComSoft[9]='0';
	
//-------------------------------------------------------------------------------
						if (Send_Tipo_Veh==CARD)
						{
							if (Tipo_Vehiculo=='H')					// Heavy
							{
							 	xTipo_Vehiculo=4;
							}
							else if	(Tipo_Vehiculo=='T') 			// Truck
							{
								xTipo_Vehiculo=3;
							}
							else if(Tipo_Vehiculo=='B')	  			// Bycicle
							{
								xTipo_Vehiculo=2;
							}
							else if(Tipo_Vehiculo=='M')				// Motocycle
							{
								xTipo_Vehiculo=1;
							}
							else if(Tipo_Vehiculo=='C')				// Carro
							{
								xTipo_Vehiculo=0;
							}
							else									// Sin categoria
							{
		
								if (Dif_Mot_Car==1)
								{
									if (automovil==0)
									{
										xTipo_Vehiculo=0;			//Carro =0;	  modificado 23/01/2019	  no funciona
									}
									else
									{
										xTipo_Vehiculo=1;	  		//MOTO = 1
									}
								}
								else
								{
									xTipo_Vehiculo=0;				//Carro =0;
								}
		   					
		 					}
							g_scArrTxComSoft[10]=xTipo_Vehiculo+0X30;
		 				}
						else
						{
							if (Dif_Mot_Car==1)
							{
								if (automovil==0)
								{
									g_scArrTxComSoft[10]='0';		//Carro =0;	  no funcionamodificado 23/01/2019
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
		  				}
//--------------------------------------------------------------------------------	
  					   	g_scArrTxComSoft[11]=',';
						

						send_dataCLK((YearIn-0x30),12);
						send_dataCLK((MonthIn-0x30),14);
						send_dataCLK((DayIn-0x30),16);
						send_dataCLK((HourIn-0x30),18);
						send_dataCLK((MinutIn-0x30),20);

						 	
 					   	g_scArrTxComSoft[22]=',';
						send_dataCLK((YearOut-0x30),23);
						send_dataCLK((MonthOut-0x30),25);
						send_dataCLK((DayOut-0x30),27);
						send_dataCLK((HourOut-0x30),29);
						send_dataCLK((MinutOut-0x30),31);
						temp=33;
						g_scArrTxComSoft[temp++]=',';
							
							if (Dato_Placa==1)
							{
//								Dato_Placa=0;
//								for (i=0; i<NumDatosPlate-1; i++)
								for (i=0; i<NumDatosPlate; i++)
								{
									g_scArrTxComSoft[temp++]=buffer_placa[i];
								}
					   		}
							else
							{
						  		g_scArrTxComSoft[temp++]=' ';
							}
							g_scArrTxComSoft[temp++]=',';
							g_scArrTxComSoft[temp++]=ETX;
							EscribirCadenaSoft(temp);




//---------------------
						}
						else
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
						


					}
					else
					{

						g_scArrTxComSoft[0]=STX;
						g_scArrTxComSoft[1]='w';
						g_scArrTxComSoft[2]=((buffer_wie[0]>>4)&0X0f)+0X30;
						g_scArrTxComSoft[3]=(buffer_wie[0]&0X0F)+0X30;
						g_scArrTxComSoft[4]=((buffer_wie[1]>>4)&0X0f)+0X30;
						g_scArrTxComSoft[5]=(buffer_wie[1]&0X0f)+0X30;
						g_scArrTxComSoft[6]=((buffer_wie[2]>>4)&0X0f)+0X30;
						g_scArrTxComSoft[7]=(buffer_wie[2]&0X0f)+0X30;
	
					   	g_scArrTxComSoft[8]=',';
						g_scArrTxComSoft[9]='0';
	
//-------------------------------------------------------------------------------
						if (Send_Tipo_Veh==CARD)
						{
							if (Tipo_Vehiculo=='H')					// Heavy
							{
							 	xTipo_Vehiculo=4;
							}
							else if	(Tipo_Vehiculo=='T') 			// Truck
							{
								xTipo_Vehiculo=3;
							}
							else if(Tipo_Vehiculo=='B')	  			// Bycicle
							{
								xTipo_Vehiculo=2;
							}
							else if(Tipo_Vehiculo=='M')				// Motocycle
							{
								xTipo_Vehiculo=1;
							}
							else if(Tipo_Vehiculo=='C')				// Carro
							{
								xTipo_Vehiculo=0;
							}
							else									// Sin categoria
							{
		
								if (Dif_Mot_Car==1)
								{
									if (automovil==0)
									{
										xTipo_Vehiculo=0;			//Carro =0;		   no funciona modificado 23/01/2019
									}
									else
									{
										xTipo_Vehiculo=1;	  		//MOTO = 1
									}
								}
								else
								{
									xTipo_Vehiculo=0;				//Carro =0;
								}
		   					
		 					}
							g_scArrTxComSoft[10]=xTipo_Vehiculo+0X30;
		 				}
						else
						{
							if (Dif_Mot_Car==1)
							{
								if (automovil==0)
								{
									g_scArrTxComSoft[10]='0';		//Carro =0;	  no funcionamodificado 23/01/2019
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
		  				}
//--------------------------------------------------------------------------------	
  					   	g_scArrTxComSoft[11]=',';
						

						send_dataCLK((YearIn-0x30),12);
						send_dataCLK((MonthIn-0x30),14);
						send_dataCLK((DayIn-0x30),16);
						send_dataCLK((HourIn-0x30),18);
						send_dataCLK((MinutIn-0x30),20);

						if (Off_Line_Salida==1)
						{
						 	
 					   		g_scArrTxComSoft[22]=',';
							send_dataCLK((YearOut-0x30),23);
							send_dataCLK((MonthOut-0x30),25);
							send_dataCLK((DayOut-0x30),27);
							send_dataCLK((HourOut-0x30),29);
							send_dataCLK((MinutOut-0x30),31);
							temp=33;
							g_scArrTxComSoft[temp++]=',';
							
							if (Dato_Placa==1)
							{
//								Dato_Placa=0;
//								for (i=0; i<NumDatosPlate-1; i++)
								for (i=0; i<NumDatosPlate; i++)
								{
									g_scArrTxComSoft[temp++]=buffer_placa[i];
								}
								g_scArrTxComSoft[temp++]=',';
								g_scArrTxComSoft[temp++]=ETX;
								EscribirCadenaSoft(temp);

							}
							else
							{
						  		g_scArrTxComSoft[temp++]=' ';
								g_scArrTxComSoft[temp++]=',';
							  	g_scArrTxComSoft[temp++]=ETX;
								EscribirCadenaSoft(temp);
							}

//							g_scArrTxComSoft[33]=ETX;
//							EscribirCadenaSoft(34);
						}
						else
						{
					   		g_scArrTxComSoft[22]=',';
							send_dataCLK((0),23);
							send_dataCLK((0),25);
							send_dataCLK((0),27);
							send_dataCLK((0),29);
							send_dataCLK((0),31);

							temp=33;
							g_scArrTxComSoft[temp++]=',';
							
							if (Dato_Placa==1)
							{
								Dato_Placa=0;
								for (i=0; i<NumDatosPlate-1; i++)
								{
									g_scArrTxComSoft[temp++]=buffer_placa[i];
								}
								g_scArrTxComSoft[temp++]=',';
								g_scArrTxComSoft[temp++]=ETX;
								EscribirCadenaSoft(temp);
							}
							else
							{
						  		g_scArrTxComSoft[temp++]=' ';
								g_scArrTxComSoft[temp++]=',';
							  	g_scArrTxComSoft[temp++]=ETX;
								EscribirCadenaSoft(temp);
							}

//							g_scArrTxComSoft[33]=ETX;
//							EscribirCadenaSoft(34);

						}

					}

// 					g_cEstadoTxSoft &=~LECTURA_WIEG_TX;
//jp					byte_wie=0x00000000;
//					nbitsW=0;
					g_cEstadoComSoft=ESPERA_RX;
			}
//-----------------------------------------------------------------------------------------------------------------			
		   	else if	((g_cEstadoTxSoft&COD_PRINT_TX)==COD_PRINT_TX)						// Envia Ticket/Impresora
			{
				g_cEstadoTxSoft &=~COD_PRINT_TX;
				g_cEstadoComSoft=ESPERA_RX;
			}	
//-----------------------------------------------------------------------------------------------------------------
		break;
//*****************************************************************************************************************
//*****************************************************************************************************************
		case ANALICE_STR_SOF:															   //ANALIZA DATOS RECIBIDOS

					g_scArrDisplay[g_cContByteRx]=0;
  				if (SerieOK==1)
				{
					SerieOK=0;
 					for (i=0; i<g_cContByteRx; i++)
					{
			 			buffer_bus[i]=g_scArrDisplay[i];
					}
					tx_bus(g_cContByteRx);
					txACK=1;
					g_cEstadoComSoft=ESPERA_RX;
				}																				
//---------------------------------------------------------------------------------------------------------------
//				else if((g_cContByteRx==25)&&(g_scArrRxComSoft[1]==g_cDirBoard)&&(g_scArrRxComSoft[2]=='H'))	//H: HORA DEL SISTEMA
				else if((g_cContByteRx==25)&&(g_scArrDisplay[1]==g_cDirBoard)&&(g_scArrDisplay[2]=='H'))		//H: HORA DEL SISTEMA
				{ 
						prg_disp();
						for (i=0; i<g_cContByteRx; i++)
						{
				 			buffer_bus[i]=g_scArrDisplay[i];
						}
						buffer_bus[i++]=calculo_bcc();
						backup_clk();
				
					
						cont(0x80);
						vdato('S');
						vdato('Y');
						vdato('N');
						vdato('C');
						txACK=1;
						g_cEstadoComSoft=ESPERA_RX;

				}
				else if(g_cContByteRx>=0x06)						//Area de Actuadores
				{
					/*envia los cupos al segundario*/
					if(g_cContByteRx>=0x07)			  				// modificado jp cmd del num de cupos disponibles
				   {	
				   	if(g_scArrRxComSoft[1]=='c')			// valido el cmd de cupos q es 'c'
					  {
							
						tx_bus(g_cContByteRx);
					/*
						  cont(0x80);								 					//se envia inf al lcd para pruebas
						  buffer_Cupo[4]='\0';
						 lcd_text(0,0,(unsigned char *)  buffer_Cupo);
							*/
						  txACK=1;
						  g_cEstadoComSoft=ESPERA_RX;
						}
					}
					
					if(g_scArrRxComSoft[1]=='D')
					{

					}
					else if(g_scArrRxComSoft[1]=='A')
					{
//						lock1=1;
//						seg=cte_seg;
//						TH0=0X00;																						
//						TL0=0X00;
//						TF0=0;				
					}
					if(g_scArrRxComSoft[2]=='D')
					{

					}
					else if(g_scArrRxComSoft[2]=='A')
					{
						if (notifyEVP==1)
						{
							if (InhabilitaPulsoEvPOut==1)
							{
								if ((Send_Wiegand==1))
								{
									if (SalidaW==0)
									{
										lock2=1;						// Alvaro Manda Abrir mensual y tiquete pero notifico
										seg=cte_seg+14;
										TH0=0X00;						//Inicializa timer										*						
										TL0=0X00;
										TF0=0;
									}
								}
 							}
							else
							{
								if (SalidaW==0)
								{
									lock2=1;							// Alvaro Manda Abrir mensual y tiquete pero notifico
									seg=cte_seg+14;
									TH0=0X00;							//Inicializa timer										*						
									TL0=0X00;
									TF0=0;
							   	}
							}

						}
					}
//---------------------------------------------------------------------------------------------------------------------------*
					if (bandera_rx_soft==1)									//(g_cContByteRx>0x06) 													//Area de Mensajes L1				
					{
						bandera_rx_soft=0;

						if ((g_scArrDisplay[5]=='A')&&(g_scArrDisplay[6]=='C')&&(g_scArrDisplay[7]=='E')&&(g_scArrDisplay[8]=='R')&&(g_scArrDisplay[9]=='Q')&&(g_scArrDisplay[10]=='U')&&(g_scArrDisplay[11]=='E')&&(g_scArrDisplay[12]=='S')&&(g_scArrDisplay[13]=='E'))
						{
					 		SendRtaBus(0xa1);
						}
   						else if ((g_scArrDisplay[5]=='U')&&(g_scArrDisplay[6]=='N')&&(g_scArrDisplay[7]==' ')&&(g_scArrDisplay[8]=='M'))
						{
							SendRtaBus(UN_MOMENTO);
							
							g_cEstadoTxSoft &=~LECTURA_COD_TX;
							g_cEstadoTxSoft &=~LECTURA_WIEG_TX;
							Dato_Placa=0;

							Tiquete_Placa=0;
							Tiquete_Salida=0;
							send_markCashierAut=0;

							Rechazo=0;
							TimeOut_Codigo=0;
							TimeOut_Wiegand=0;
							SalidaW=0;
							Tipo_Vehiculo=' ';
							
 						}
   						else if ((g_scArrDisplay[1]=='X')&&(g_scArrDisplay[2]=='A')&&(g_scArrDisplay[3]=='X')&&(g_scArrDisplay[4]=='X')&&(g_scArrDisplay[5]=='G')&&(g_scArrDisplay[6]=='R')&&(g_scArrDisplay[7]=='A')&&(g_scArrDisplay[8]=='C')&&(g_scArrDisplay[9]=='I')&&(g_scArrDisplay[10]=='A')&&(g_scArrDisplay[11]=='S'))
						{
							SendMsg(GRACIAS);
  						}
   						else if ((g_scArrDisplay[1]=='X')&&(g_scArrDisplay[2]=='X')&&(g_scArrDisplay[3]=='X')&&(g_scArrDisplay[4]=='X')&&(g_scArrDisplay[5]=='B')&&(g_scArrDisplay[6]=='I')&&(g_scArrDisplay[7]=='E')&&(g_scArrDisplay[8]=='N')&&(g_scArrDisplay[9]=='V')&&(g_scArrDisplay[10]=='E')&&(g_scArrDisplay[11]=='N')&&(g_scArrDisplay[12]=='I')&&(g_scArrDisplay[13]=='D')&&(g_scArrDisplay[14]=='O'))
						{
					 		SendMsg(BIENVENIDO);
 						}
				   		else if ((g_scArrDisplay[1]=='X')&&(g_scArrDisplay[2]=='X')&&(g_scArrDisplay[3]=='X')&&(g_scArrDisplay[4]=='X')&&(g_scArrDisplay[5]=='M')&&(g_scArrDisplay[6]=='E')&&(g_scArrDisplay[7]=='N')&&(g_scArrDisplay[8]=='S')&&(g_scArrDisplay[9]=='U')&&(g_scArrDisplay[10]=='A')&&(g_scArrDisplay[11]=='L')&&(g_scArrDisplay[12]==' ')&&(g_scArrDisplay[13]=='N')&&(g_scArrDisplay[14]=='O')&&(g_scArrDisplay[15]==' ')&&(g_scArrDisplay[16]=='E')&&(g_scArrDisplay[17]=='S')&&(g_scArrDisplay[18]=='T')&&(g_scArrDisplay[19]=='A'))
						{
							SendRtaBus(NO_IN_PARK);
 						}

   						else if ((g_scArrDisplay[1]=='X')&&(g_scArrDisplay[2]=='X')&&(g_scArrDisplay[3]=='X')&&(g_scArrDisplay[4]=='X')&&(g_scArrDisplay[5]=='M')&&(g_scArrDisplay[6]=='E')&&(g_scArrDisplay[7]=='N')&&(g_scArrDisplay[8]=='S')&&(g_scArrDisplay[9]=='U')&&(g_scArrDisplay[10]=='A')&&(g_scArrDisplay[11]=='L')&&(g_scArrDisplay[12]==' ')&&(g_scArrDisplay[13]=='Y')&&(g_scArrDisplay[14]=='A')&&(g_scArrDisplay[15]==' ')&&(g_scArrDisplay[16]=='E')&&(g_scArrDisplay[17]=='S')&&(g_scArrDisplay[18]=='T')&&(g_scArrDisplay[19]=='A'))
						{
							SendRtaBus(IN_PARK);
		 				}
   						else if ((g_scArrDisplay[1]=='X')&&(g_scArrDisplay[2]=='X')&&(g_scArrDisplay[3]=='X')&&(g_scArrDisplay[4]=='X')&&(g_scArrDisplay[5]=='M')&&(g_scArrDisplay[6]=='E')&&(g_scArrDisplay[7]=='N')&&(g_scArrDisplay[8]=='S')&&(g_scArrDisplay[9]=='U')&&(g_scArrDisplay[10]=='A')&&(g_scArrDisplay[11]=='L')&&(g_scArrDisplay[12]==' ')&&(g_scArrDisplay[13]=='F')&&(g_scArrDisplay[14]=='U')&&(g_scArrDisplay[15]=='E')&&(g_scArrDisplay[16]=='R')&&(g_scArrDisplay[17]=='A')&&(g_scArrDisplay[18]==' ')&&(g_scArrDisplay[19]=='D')&&(g_scArrDisplay[20]=='E')&&(g_scArrDisplay[21]=='R'))
						{
							SendRtaBus(EXPIRO);
 						}
   						else if ((g_scArrDisplay[1]=='X')&&(g_scArrDisplay[2]=='X')&&(g_scArrDisplay[3]=='X')&&(g_scArrDisplay[4]=='X')&&(g_scArrDisplay[5]=='M')&&(g_scArrDisplay[6]=='E')&&(g_scArrDisplay[7]=='N')&&(g_scArrDisplay[8]=='S')&&(g_scArrDisplay[9]=='U')&&(g_scArrDisplay[10]=='A')&&(g_scArrDisplay[11]=='L')&&(g_scArrDisplay[12]==' ')&&(g_scArrDisplay[13]=='F')&&(g_scArrDisplay[14]=='U')&&(g_scArrDisplay[15]=='E')&&(g_scArrDisplay[16]=='R')&&(g_scArrDisplay[17]=='A')&&(g_scArrDisplay[18]==' ')&&(g_scArrDisplay[19]=='D')&&(g_scArrDisplay[20]=='E')&&(g_scArrDisplay[21]=='H'))
						{
							SendRtaBus(HORARIO);
 						}
						else if ((g_scArrDisplay[1]=='X')&&(g_scArrDisplay[2]=='X')&&(g_scArrDisplay[3]=='X')&&(g_scArrDisplay[4]=='X')&&(g_scArrDisplay[5]=='N')&&(g_scArrDisplay[6]=='O')&&(g_scArrDisplay[7]==' ')&&(g_scArrDisplay[8]=='E')&&(g_scArrDisplay[9]=='S')&&(g_scArrDisplay[10]==' ')&&(g_scArrDisplay[11]=='M')&&(g_scArrDisplay[12]=='E')&&(g_scArrDisplay[13]=='N')&&(g_scArrDisplay[14]=='S'))
						{
							SendRtaBus(NO_MENSUAL);
											 	
 						}
					   	else if ((g_scArrDisplay[1]=='X')&&(g_scArrDisplay[2]=='X')&&(g_scArrDisplay[3]=='X')&&(g_scArrDisplay[4]=='X')&&(g_scArrDisplay[5]=='N')&&(g_scArrDisplay[6]=='O')&&(g_scArrDisplay[7]==' ')&&(g_scArrDisplay[8]=='R')&&(g_scArrDisplay[9]=='E')&&(g_scArrDisplay[10]=='G')&&(g_scArrDisplay[11]=='I')&&(g_scArrDisplay[12]=='S')&&(g_scArrDisplay[13]=='T')&&(g_scArrDisplay[14]=='R'))
						{
							SendRtaBus(NO_REGIST);

						}
    					else if ((g_scArrDisplay[1]=='X')&&(g_scArrDisplay[2]=='X')&&(g_scArrDisplay[3]=='X')&&(g_scArrDisplay[4]=='X')&&(g_scArrDisplay[5]=='L')&&(g_scArrDisplay[6]=='O')&&(g_scArrDisplay[7]=='T')&&(g_scArrDisplay[8]=='E')&&(g_scArrDisplay[9]==' ')&&(g_scArrDisplay[10]=='A')&&(g_scArrDisplay[11]=='S')&&(g_scArrDisplay[12]=='I')&&(g_scArrDisplay[13]=='G')&&(g_scArrDisplay[14]=='N'))
						{
							SendRtaBus(LOT_FULL);
						}
						//
							else if ((g_scArrDisplay[1]=='X')&&(g_scArrDisplay[2]=='X')&&(g_scArrDisplay[3]=='X')&&(g_scArrDisplay[4]=='X')&&(g_scArrDisplay[5]=='M')&&(g_scArrDisplay[6]=='E')&&(g_scArrDisplay[7]=='N')&&(g_scArrDisplay[8]=='S')&&(g_scArrDisplay[9]=='U')&&(g_scArrDisplay[10]=='A')&&(g_scArrDisplay[11]=='L')&&(g_scArrDisplay[12]==' ')&&(g_scArrDisplay[13]=='E')&&(g_scArrDisplay[14]=='X')&&(g_scArrDisplay[15]=='C')&&(g_scArrDisplay[16]=='E')&&(g_scArrDisplay[17]=='D')&&(g_scArrDisplay[18]=='E'))
						{
							SendRtaBus(EXCEDE_HORARIO);
						}
							else if ((g_scArrDisplay[1]=='X')&&(g_scArrDisplay[2]=='X')&&(g_scArrDisplay[3]=='X')&&(g_scArrDisplay[4]=='X')&&(g_scArrDisplay[5]=='M')&&(g_scArrDisplay[6]=='E')&&(g_scArrDisplay[7]=='N')&&(g_scArrDisplay[8]=='S')&&(g_scArrDisplay[9]=='U')&&(g_scArrDisplay[10]=='A')&&(g_scArrDisplay[11]=='L')&&(g_scArrDisplay[12]==' ')&&(g_scArrDisplay[13]=='N')&&(g_scArrDisplay[14]=='O')&&(g_scArrDisplay[15]==' ')&&(g_scArrDisplay[16]=='P')&&(g_scArrDisplay[17]=='A')&&(g_scArrDisplay[18]=='G'))
						{
							SendRtaBus(MENSUAL_NO_PAGO);
						}
//-------------------------------------------------------------------------------------------------------------------------
						cont(0x80);
						for(i=5;(i<21)&&(g_scArrDisplay[i]!=ETX);i++)			/*msj en el display ejemplo (PARQUEADERO)*/
						{
							vdato(g_scArrDisplay[i]);
 						}
						cont(0xc0);
						for(i=21;(i<=38)&&(g_scArrDisplay[i]!=ETX);i++)
						{
  							if (msg_error==0)
							{
								vdato(g_scArrDisplay[i]);											/*hora del sistema*/
							}
						}

					} //if(g_cContByteRx>0x06)			 


				}
//-------------------------------------------------------------------------------------------------------------------------
				g_cEstadoComSoft=ESPERA_RX;
				txACK=1;
	   	
		break;
//-----------------------------------------------------------------------------------------------------------
		default:
			g_cEstadoComSoft=ESPERA_RX;
		break;
	
	}
} 

//*******************************************************************************************
//
//	Main C function that start the interrupt-driven serial I/O.
//
//*******************************************************************************************
void main (void) 
{

 unsigned char MSGnotify   	[]= "COMPARACION:    " ;
 unsigned char msgdir  	    []= "   Addr:        " ;
 unsigned char sin_resp     []= " SIN RESPUESTA  " ; 
 unsigned char resp_NACK    []= " PLACA NO IGUAL " ; 
 unsigned char SendCode 	[]= " ENVIANDO DATOS " ; 

 unsigned char k,i,NumDatValidar;
 unsigned char PosComa[2];
 unsigned char temp;

//*******************************************************************************************	
 	com_initialize ();                  // Initialize interrupt driven serial I/O 			*
	inicia_wiegand();									// Inicia wiegand lectrua a 26 complemnto a uno, lectura a 33, 27 lectura a 26 sin complemeto
 // 	EA = 1;                             // Enable global interrupts 						*
 	prg_disp();
//*******************************************************************************************	
	TMOD=(TMOD & 0xf0) | 0x01;			//  Coloca el temporizador 0 y 1 en modo 1.  16bITS	*
 	TF0=0; 								//  Bandera de Timer								*
	TH0=0X00;							//													*						
	TL0=0X00;							//													*
	TR0=1;								// Run TM2											*
//******************************************************************************************* 
	lock1=0;							// Relevo											*
	lock2=0;
	ledv=0;							   	// Led Pulsador										*
	bus_clk=1;
//******************************************************************************************* 
// 	INICIALIZACIONES
//*******************************************************************************************


	toggle=1;
	TimeOut_Codigo=0;
	TimeOut_Wiegand=0;
	TimeOutLinea=TIMEW;
	seg=cte_seg;
	retry=0;

 	msg1=0;
	audio1=0;

	msg2=0;
	audio2=0;

	msg3=0;
	audio3=0;

	msg4=0;
	audio4=0;

	send_markCashierAut=0;
	Rechazo=0;
	Dato_OffLine=0;
	Tiquete_Placa=0;
	Tiquete_Salida=0;
	TimeRetryCmd=0;
	TimeOut_Send_Acceso=0;
	Tx_Acceso=0;
	iTimeEsperaRtaLPR=0;
	RetryCmd=0;
//*******************************************************************************************
//	PROGRAMACIONES
//*******************************************************************************************
	ACCESO_USE_LPR=0;		  							// 0 = Envia a Evp   1 = Envia al secundario LPR para Decision de Acceso
	notifyEVP=1;
	Dif_Mot_Car=1;
	OrigenTipoVeh=CARD;									// CARD - LOOP


	InhabilitaPulsoEvPOut=1;   							// Mifare Inhabilta Pulso de Evp...
														// Ticket NO debe Inhabilitar
	Send_Tipo_Veh=CARD;									// CARD o LOOP
	OpenMensual_Apx=0;									// 
	Central_ID_OFFLINE=0;
//*******************************************************************************************

	lcd_text(0,0,(unsigned char *) " AccesScan v4.5 ");

	cont(0xc0);
	lcd_puts(msgdir);  						/*jp se muestra la direccion*/
 	ve_dir();
//------------------------------------------------------------------------------------------*
	busy=1;
	bus_clk=1;
	if (automovil==1)
	{
	 	
	}
//*******************************************************************************************
  while (1)
   {

  		P2=0xff;
		temp=P2;
  
		if((completo == 1) &&(SignalAcceso==0))
		{
		
			if (Habilita_Lectura==1)
			{
				TimeOutLinea=TIMEW;							/*tengo el dato wiegand en el orden de access*/
		  		id_Access();   								/*tengo el dato en buffer_wie*/
				
				limpia_data();								/*limpio los datos*/				
			  	
				if ((ACCESO_USE_LPR==1))
				{
		
					if (iTimeEsperaRtaLPR==0)					/*envio trama para ser transmitida por tcp/ip*/
					{
					buffer_bus[0]=STX;
					buffer_bus[1]='W';
					buffer_bus[2]=buffer_wie[0];
					buffer_bus[3]=buffer_wie[1];
					buffer_bus[4]=buffer_wie[2];
					buffer_bus[5]=ETX;
	
					buffer_wieLPR[0]=buffer_wie[0];
					buffer_wieLPR[1]=buffer_wie[1];
					buffer_wieLPR[2]=buffer_wie[2];
	
					g_cContByteRx=6;
					tx_bus(g_cContByteRx);									/*transmito al pto paralelo*/
					g_cEstadoTxSoft &=~LECTURA_WIEG_TX;			// Si usa LPR no envia al software. LPR agrega el registro y da Acceso
					iTimeEsperaRtaLPR=cte_seg*20;
					}
					else
					{
					g_cEstadoTxSoft &=~LECTURA_WIEG_TX;			// Si usa LPR no envia al software. LPR agrega el registro y da Acceso
					}
				  
				}
				else
				{
				buffer_bus[0]=STX;
				buffer_bus[1]='W';
				buffer_bus[2]=buffer_wie[0];
				buffer_bus[3]=buffer_wie[1];
				buffer_bus[4]=buffer_wie[2];
				buffer_bus[5]=ETX;
				g_cContByteRx=6;
				tx_bus(g_cContByteRx);
				g_cEstadoTxSoft=LECTURA_WIEG_TX;
				SalidaW=0;
				TimeOut_Wiegand=5;
					g_cEstadoComSoft=POLL_COM_SOF;
					AtencComSoft();
				}

			}	
			else
			{
				lcd_text(1,0,(unsigned char *) "INHABILITADA... ");
				g_cEstadoTxSoft &=~LECTURA_WIEG_TX;
			}


				if (Central_ID_OFFLINE==1)
				{
					if (((buffer_wie[0]==0x76)&&(buffer_wie[1]==0xd6)&&(buffer_wie[2]==0x0b))||((buffer_wie[0]==0x24)&&(buffer_wie[1]==0x54)&&(buffer_wie[2]==0xe4))||((buffer_wie[0]==0x95)&&(buffer_wie[1]==0x00)&&(buffer_wie[2]==0xd5))||((buffer_wie[0]==0x5F)&&(buffer_wie[1]==0xd7)&&(buffer_wie[2]==0x84)))
					{
						lock1=1;
						lock2=1;
						seg=cte_seg+14;
						TH0=0X00;						//Inicializa timer										*						
						TL0=0X00;
						TF0=0;
					}
					else if (((buffer_wie[0]==0x95)&&(buffer_wie[1]==0x00)&&(buffer_wie[2]==0xf2))||((buffer_wie[0]==0x60)&&(buffer_wie[1]==0x40)&&(buffer_wie[2]==0x94))||((buffer_wie[0]==0x95)&&(buffer_wie[1]==0x00)&&(buffer_wie[2]==0xf3))||((buffer_wie[0]==0x37)&&(buffer_wie[1]==0x36)&&(buffer_wie[2]==0x24)))
					{
						lock1=1;
						lock2=1;
						seg=cte_seg+14;
						TH0=0X00;						//Inicializa timer										*						
						TL0=0X00;
						TF0=0;
					}
					else if (((buffer_wie[0]==0x95)&&(buffer_wie[1]==0x01)&&(buffer_wie[2]==0x10))||((buffer_wie[0]==0x7f)&&(buffer_wie[1]==0x75)&&(buffer_wie[2]==0x44))||((buffer_wie[0]==0x95)&&(buffer_wie[1]==0x01)&&(buffer_wie[2]==0x11))||((buffer_wie[0]==0x17)&&(buffer_wie[1]==0xee)&&(buffer_wie[2]==0x24)))
					{
						lock1=1;
						lock2=1;
						seg=cte_seg+14;
						TH0=0X00;						//Inicializa timer										*						
						TL0=0X00;
						TF0=0;
					}
					else if (((buffer_wie[0]==0x95)&&(buffer_wie[1]==0x4f)&&(buffer_wie[2]==0xd7))||((buffer_wie[0]==0x67)&&(buffer_wie[1]==0x17)&&(buffer_wie[2]==0xb4))||((buffer_wie[0]==0x94)&&(buffer_wie[1]==0xea)&&(buffer_wie[2]==0x75))||((buffer_wie[0]==0x7c)&&(buffer_wie[1]==0xde)&&(buffer_wie[2]==0x34)))
					{
						lock1=1;
						lock2=1;
						seg=cte_seg+14;
						TH0=0X00;						//Inicializa timer										*						
						TL0=0X00;
						TF0=0;
					}
					else if (((buffer_wie[0]==0x94)&&(buffer_wie[1]==0xeb)&&(buffer_wie[2]==0x58))||((buffer_wie[0]==0x17)&&(buffer_wie[1]==0x16)&&(buffer_wie[2]==0xd4))||((buffer_wie[0]==0x8f)&&(buffer_wie[1]==0x42)&&(buffer_wie[2]==0xd6)))
					{
						lock1=1;
						lock2=1;
						seg=cte_seg+14;
						TH0=0X00;						//Inicializa timer										*						
						TL0=0X00;
						TF0=0;
					}
					else if (((buffer_wie[0]==0x95)&&(buffer_wie[1]==0x1d)&&(buffer_wie[2]==0xbc))||((buffer_wie[0]==0x26)&&(buffer_wie[1]==0x12)&&(buffer_wie[2]==0x84)))
					{
						lock1=1;
						lock2=1;
						seg=cte_seg+14;
						TH0=0X00;						//Inicializa timer										*						
						TL0=0X00;
						TF0=0;
					}
					else if (((buffer_wie[0]==0x82)&&(buffer_wie[1]==0x88)&&(buffer_wie[2]==0x14))||((buffer_wie[0]==0x51)&&(buffer_wie[1]==0xc1)&&(buffer_wie[2]==0x14)))
					{
						lock1=1;
						lock2=1;
						seg=cte_seg+14;
						TH0=0X00;						//Inicializa timer										*						
						TL0=0X00;
						TF0=0;
					}
				}

//-----------------------------------------------------------------------------------------------------------*
	
			}
			else
			{
					
				if (completo==1)
				{
					id_Access();																				/*se muestra el nuemro de la tarjeta pero no hay presencia*/
			
					limpia_data();	
					SendRtaBus(ERROR_LOOP);																/*envia un msj a segundario q hay un error en el loop*/
					lcd_text(1,0,(unsigned char *) "ERROR EN LOOP...");
					g_cEstadoTxSoft &=~LECTURA_WIEG_TX;
				}
				

 			}
	
//------------------------------------------------------------------------------------------*
//		INTERRUPCION DEL AUXILIAR
//------------------------------------------------------------------------------------------*
		if (ready==0)							//Recibe del procesador Aux	(Interrupcion generada del Aux)
		{
			buffer_bus[0]=0xff;
			buffer_bus[1]=0xff;
			rx_bus();
		/*
			lcd_text(0,0,(unsigned char *) " pto P0 "); 		//jp
			temp=0xff;
			while(temp){temp--;}//jp
//-----------------------------------  TEST
 			for (k=0; k<num_data; k++)
		   	{
				tx_chr(buffer_bus[k]);
			}
//-----------------------------------		
		*/
			if (num_data!=0)
			{
			   	if ((num_data==1)&&(buffer_bus[0]==ACK))   											// ERRORES
				{
					Habilita_Lectura=1;
				}

				if ((num_data==1)&&(buffer_bus[0]>=0xDF)&&(buffer_bus[0]<=0xef))   					// ERRORES
				{
						prg_disp();
 					if	((buffer_bus[0]==0xe2)||(buffer_bus[0]==0xe3)||(buffer_bus[0]==0xe4))
					{
					    cont(0xc0);
 						for (k=0;err_mifare[k]!='\0';k++)
						{
  							vdato(err_mifare[k]);
						}
						cont(0xcE);
						vdato((buffer_bus[0]&0x0f)+0x30);
					}

					else if(buffer_bus[0]==0xe0)
					{
  					    cont(0xc0);
 						for (k=0;sin_sensor[k]!='\0';k++)
						{
  							vdato(sin_sensor[k]);
						}

 					}
 					else if(buffer_bus[0]==0xe1)
					{
  					    
						lcd_text(1,0,(unsigned char *) "TARJETA INVALIDA");
						 
 					}
					else if(buffer_bus[0]==0xe5)
					{
  					    cont(0xc0);
 						for (k=0;err_cod[k]!='\0';k++)
						{
  							vdato(err_cod[k]);
						}
 					}
					else if(buffer_bus[0]==0xe6)
					{
 					    cont(0xc0);
 						for (k=0;err_in[k]!='\0';k++)
						{
  							vdato(err_in[k]);
						}
					}
					else if(buffer_bus[0]==0xe7)
					{
					
						cont(0xc0);
 						lcd_puts((unsigned char *)"NO REGISTRA PAGO");
					 	
						
						wait_long1(10000);
						//{
  						//	vdato(err_sinpago[k]);
						//}
					}
					else if(buffer_bus[0]==0xe8)
					{
						cont(0xc0);
 						for (k=0;err_gracia[k]!='\0';k++)
						{
  							vdato(err_gracia[k]);
						}					
					}

					else if(buffer_bus[0]==0xe9)
					{
						cont(0xc0);
 						for (k=0;err_out[k]!='\0';k++)
						{
  							vdato(err_out[k]);
						}					
					}
					else if(buffer_bus[0]==0xea)
					{
						cont(0xc0);
 						for (k=0;err_data[k]!='\0';k++)
						{
  							vdato(err_data[k]);
						}					
					}
					else if(buffer_bus[0]==0xeB)
					{
						cont(0xc0);
 						for (k=0;err_Caja[k]!='\0';k++)
						{
  							vdato(err_Caja[k]);
						}					
					}
					else if(buffer_bus[0]==0xec)
					{
						cont(0xc0);
 						for (k=0;tarjeta_venc[k]!='\0';k++)
						{
  							vdato(tarjeta_venc[k]);
						}					
					}
					else if (buffer_bus[0]==0xed)
					{
						cont(0xc0);
 						for (k=0;sin_resp[k]!='\0';k++)
						{
  							vdato(sin_resp[k]);
						}					
					}
					else if (buffer_bus[0]==0xee)
					{
						cont(0xc0);
 						for (k=0;resp_NACK[k]!='\0';k++)
						{
  							vdato(resp_NACK[k]);
						}					
					}

					else if (buffer_bus[0]==0xeF)
					{
						cont(0xc0);
						vdato('T');
						vdato('A');
						vdato('R');
						vdato('J');
						vdato('E');
						vdato('T');
						vdato('A');
						vdato(' ');
						vdato('A');
						vdato('T');
						vdato('A');
						vdato('S');
						vdato('C');
						vdato('A');
						vdato('D');
						vdato('A');

//						lock1=1;
						Atascado=1;
						seg=cte_seg;
					}
					else if (buffer_bus[0]==0xDF)
					{	
						prg_disp();
						cont(0xc0);
						lcd_puts((unsigned char *) "TAR. SIN FORMATO");
						
					}
 					msg_error=1;
					seg=cte_seg;
					TH0=0X00;								//Inicializa timer						*						
					TL0=0X00;
					TF0=0;
				}
				else if ((num_data==1)&&(buffer_bus[0]==0xa0))
				{
				 	audio1=1;
					msg1=1;
					seg=cte_seg;
				}
				else if ((num_data==1)&&(buffer_bus[0]==0xa1))
				{
				 	audio2=1;
					msg2=1;
					seg=cte_seg;
					lcd_text(0,0,(unsigned char *) "DIRIJASE A CAJA ");
				}
				else if ((num_data==1)&&(buffer_bus[0]==0xa2))
				{
					audio3=1;
					msg3=1;
					seg=cte_seg;
				}
				else if ((num_data==1)&&(buffer_bus[0]==0xA3))
				{
				 	audio4=1;
					msg4=1;
					seg=cte_seg+14;
				}
				else if ((num_data==1)&&(buffer_bus[0]==0XAF))
				{
					cont(0xc0);

					vdato('C');
					vdato('O');
					vdato('N');
					vdato('T');
					vdato('.');
					vdato(' ');
					vdato('E');
					vdato('V');
					vdato('N');
					vdato(' ');
					vdato('I');
					vdato('N');
					vdato('I');
					vdato('C');
					vdato('I');
					vdato('.');

				}
				else if ((num_data==1)&&(buffer_bus[0]=='N'))
				{
					notifyEVP=1;

					cont(0x80);
					for (k=0;MSGnotify[k]!='\0';k++)
					{
						vdato(MSGnotify[k]);
					}
					cont(0x8D);
					vdato('N');
					if (ACCESO_USE_LPR==1)
					{
						vdato(' ');
						vdato('M');
 					}
					
				}
				else if ((num_data==1)&&(buffer_bus[0]=='S'))
				{
				 	notifyEVP=0;
					cont(0x80);
					for (k=0;MSGnotify[k]!='\0';k++)
					{
						vdato(MSGnotify[k]);
					}
					cont(0x8D);
					vdato('S');
					if (ACCESO_USE_LPR==1)
					{
						vdato(' ');
						vdato('M');
 					}



 				}
				else if ((num_data==1)&&(buffer_bus[0]=='P'))
				{
				 	notifyEVP=0;
					cont(0xc0);
					for (k=0;matchPlate[k]!='\0';k++)
					{
						vdato(matchPlate[k]);
					}
 				}
				else if ((num_data==1)&&(buffer_bus[0]=='D'))
				{
					cont(0xc0);
					for (k=0;SendCode[k]!='\0';k++)
					{
						vdato(SendCode[k]);
					}
 				}
				else if ((num_data==1)&&(buffer_bus[0]==0XF1))
				{
					cont(0xc0);

					vdato('U');
					vdato('N');
					vdato(' ');
					vdato('M');
					vdato('O');
					vdato('M');
					vdato('E');
					vdato('N');
					vdato('T');
					vdato('O');
					vdato('.');
					vdato('.');
					vdato('.');
					vdato(' ');
					vdato(' ');
					vdato(' ');



 				}
				else if ((num_data==1)&&(buffer_bus[0]==PICO_PLACA))
				{
					cont(0xc0);

					vdato('P');
					vdato('I');
					vdato('C');
					vdato('O');
					vdato(' ');
					vdato('Y');
					vdato(' ');
					vdato('P');
					vdato('L');
					vdato('A');
					vdato('C');
					vdato('A');
					vdato('.');
					vdato('.');
					vdato('.');
					vdato(' ');

 				}
				else if ((num_data==1)&&(buffer_bus[0]==0XF8))
				{
					cont(0xc0);

					vdato('V');
					vdato('E');
					vdato('H');
					vdato('I');
					vdato('.');
					vdato(' ');
					vdato('I');
					vdato('N');
					vdato('V');
					vdato('A');
					vdato('L');
					vdato('I');
					vdato('D');
					vdato('O');
					vdato('.');
					vdato('.');
					vdato('.');


 				}
				else if ((num_data==1)&&(buffer_bus[0]==0XF9))
				{
					cont(0xc0);

					vdato('S');
					vdato('A');
					vdato('L');
					vdato('I');
					vdato('D');
					vdato('A');
					vdato(' ');
					vdato('R');
					vdato('E');
					vdato('C');
					vdato('H');
					vdato('A');
					vdato('Z');
					vdato('A');
					vdato('D');
					vdato('A');


 				}
				else if ((num_data==1)&&(buffer_bus[0]==SIN_TARJETAS))
				{
					cont(0xc0);

					vdato('S');
					vdato('I');
					vdato('N');
					vdato(' ');
					vdato('T');
					vdato('A');
					vdato('R');
					vdato('J');
					vdato('E');
					vdato('T');
					vdato('A');
					vdato('S');
					vdato(' ');
					vdato(' ');
					vdato(' ');
					vdato(' ');


 				}
				else if ((num_data==1)&&(buffer_bus[0]==HORARIO))
				{
					cont(0xc0);

					vdato('F');
					vdato('U');
					vdato('E');
					vdato('R');
					vdato('A');
					vdato(' ');
					vdato('D');
					vdato('E');
					vdato(' ');
					vdato('H');
					vdato('O');
					vdato('R');
					vdato('A');
					vdato('R');
					vdato('I');
					vdato('O');


 				}
				else if ((num_data==1)&&(buffer_bus[0]==IN_HORARIO))
				{
					cont(0x80);

					vdato(' ');
					vdato('E');
					vdato('N');
					vdato('T');
					vdato('R');
					vdato('A');
					vdato('D');
					vdato('A');
					vdato(' ');
					vdato('F');
					vdato('U');
					vdato('E');
					vdato('R');
					vdato('A');
					vdato(' ');
					vdato(' ');

					cont(0xc0);

					vdato('D');
					vdato('E');
					vdato(' ');
					vdato('H');
					vdato('O');
					vdato('R');
					vdato('A');
					vdato('R');
					vdato('I');
					vdato('O');
 					vdato(' ');
					vdato('-');
					vdato('C');
					vdato('A');
					vdato('J');
					vdato('A');

 				}
				else if ((num_data==1)&&(buffer_bus[0]==DiaX))
				{
					cont(0x80);

					vdato('D');
					vdato('I');
					vdato('A');
					vdato(' ');
					vdato('D');
					vdato('E');
					vdato(' ');
					vdato('A');
					vdato('C');
					vdato('C');
					vdato('E');
					vdato('S');
					vdato('O');
					vdato(' ');
					vdato(' ');
					vdato(' ');

					cont(0xc0);

					vdato('I');
					vdato('N');
					vdato('V');
					vdato('A');
					vdato('L');
					vdato('I');
					vdato('D');
					vdato('O');
					vdato(' ');
					vdato(' ');
 					vdato(' ');
					vdato(' ');
					vdato(' ');
					vdato(' ');
					vdato(' ');
					vdato(' ');

 				}
				else if ((num_data==1)&&(buffer_bus[0]==0XFb))
				{
					cont(0xc0);

					vdato('E');
					vdato('R');
					vdato('R');
					vdato('O');
					vdato('R');
					vdato(' ');
					vdato('T');
					vdato('R');
					vdato('A');
					vdato('N');
					vdato('S');
					vdato('P');
					vdato('O');
					vdato('R');
					vdato('T');
					vdato('E');
  
 				}
				else if ((num_data==1)&&(buffer_bus[0]==0XFC))
				{
					cont(0xc0);

					vdato('N');
					vdato('O');
					vdato(' ');
					vdato('E');
					vdato('S');
					vdato(' ');
					vdato('M');
					vdato('E');
					vdato('N');
					vdato('S');
					vdato('U');
					vdato('A');
					vdato('L');
					vdato('.');
					vdato('.');
					vdato('.');
  
 				}
				else if ((num_data==1)&&(buffer_bus[0]==0XFD))
				{
					cont(0xc0);

					vdato('N');
					vdato('O');
					vdato(' ');
					vdato('E');
					vdato('S');
					vdato(' ');
					vdato('D');
					vdato('E');
					vdato(' ');
					vdato('R');
					vdato('O');
					vdato('T');
					vdato('A');
					vdato('C');
					vdato('I');
					vdato('O');
  
 				}
				else if ((num_data==1)&&(buffer_bus[0]==0XFe))
				{
					cont(0xc0);

					vdato('B');
					vdato('I');
					vdato('E');
					vdato('N');
					vdato('V');
					vdato('E');
					vdato('N');
					vdato('I');
					vdato('D');
					vdato('O');
					vdato(' ');
					vdato(' ');
					vdato(' ');
					vdato(' ');
					vdato(' ');
					vdato(' ');
  
 				}
				else if ((num_data==1)&&(buffer_bus[0]==0XFF))
				{
					cont(0x80);

					vdato('G');
					vdato('R');
					vdato('A');
					vdato('C');
					vdato('I');
					vdato('A');
					vdato('S');
					vdato('.');
					vdato('.');
					vdato('.');
					vdato(' ');
					vdato(' ');
					vdato(' ');
					vdato(' ');
					vdato(' ');
					vdato(' ');
  
 				}
				else if ((buffer_bus[0]==0x02)&&(buffer_bus[1]=='i'))	 //	(num_data>=18)&&
				{

						prg_disp();
						
						lcd_text(0,0,(unsigned char *) "Fecha Hora:     ");		//	muestra en el lcd la fecha                     								*

//						cont(0x80);
//						for (k=0;informa[k]!='\0';k++)
//						{
//							vdato(informa[k]);
//						}
			
						cont(0xc0);	
						vdata_clk(buffer_bus[2]); 		//año	 [2]
						vdato('/');

						vdata_clk(buffer_bus[3]);  		//mes
						vdato('/');

						vdata_clk(buffer_bus[4]);		//dia
						vdato(' ');

						vdata_clk(buffer_bus[5]);		//hora
						vdato(':');

 						vdata_clk(buffer_bus[6]);		//minuto
							
  						vdata_clk(buffer_bus[7]);		//segundo

						cont(0x8c);
						if (buffer_bus[8]==1)
						{
							vdato('D');
							vdato('o');
							vdato('m');
						
						}
						else if	(buffer_bus[8]==2)
						{
							vdato('L');
							vdato('u');
							vdato('n');

						}
						else if	(buffer_bus[8]==3)
						{
							vdato('M');
							vdato('a');
							vdato('r');

						}
						else if	(buffer_bus[8]==4)
						{
							vdato('M');
							vdato('i');
							vdato('e');
						}
						else if	(buffer_bus[8]==5)
						{
							vdato('J');
							vdato('u');
							vdato('e');
						}
						else if	(buffer_bus[8]==6)
						{
							vdato('V');
							vdato('i');
							vdato('e');
						}
						else if	(buffer_bus[8]==7)
						{
							vdato('S');
							vdato('a');
							vdato('b');
						}													
				}
				else if ((buffer_bus[0]==0x02)&&(buffer_bus[1]=='I'))	 //	(num_data>=18)&&
				{
					cont(0x80);
 					for (k=0;serie[k]!='\0';k++)
					{
 						vdato(serie[k]);
					}
  					temp=num_data-2;
					cont(0xc0);
					num_chr=0;
					for (k=0; k<temp; k++)
					{
						if (buffer_bus[k+2]==ETX)
						{
							k=temp+1;
						}
						else
						{
							vdato(buffer_bus[k+2]);
							num_chr++;
						}
					}
					for (k=0; k<(16-num_chr); k++)
					{
						vdato(' ');
					}
				}
				else if ((buffer_bus[0]==0x02)&&(buffer_bus[1]=='D'))	 //	(num_data>=18)&&
				{
					prg_disp();
				   cont(0x80);
					lcd_puts((unsigned char *) "ID_CLIENTE:");
						for(k=2;;k++)
						{
							if(buffer_bus[k]!=';')
							 {
								vdato(buffer_bus[k]);
							 }	else break;
						}
					cont(0xc0);
					k++;
						lcd_puts((unsigned char *) "COD_PARK:");
							for(i=2;;k++)
							{
								if(buffer_bus[k]!=';')
							 	{
									vdato(buffer_bus[k]);
								}else break;
							}
			    }
				
					else if ((buffer_bus[0]==0x02)&&(buffer_bus[1]==05)&&(buffer_bus[2]==0x03))	 //	bcc hora
				{
					Debug_txt_uart((unsigned char *) "recibo trama Hora: ");
					Retransmitir_trama_hora();
			  }
					
					
				else if ((buffer_bus[0]==0x02)&&(buffer_bus[1]=='W')&&(buffer_bus[num_data-1]==ETX)&&(num_data==18))	//	
				{
					
					iTimeEsperaRtaLPR=0;
					
					Dato_OffLine=0;
 					SalidaW=1;

					buffer_wie[0]=buffer_bus[4];
					buffer_wie[1]=buffer_bus[3];
					buffer_wie[2]=buffer_bus[2];

					YearIn=buffer_bus[6];
					MonthIn=buffer_bus[7];
					DayIn=buffer_bus[8];
					HourIn=buffer_bus[9];
					MinutIn=buffer_bus[10];
	
					YearOut=buffer_bus[11];
					MonthOut=buffer_bus[12];
					DayOut=buffer_bus[13];
					HourOut=buffer_bus[14];
					MinutOut=buffer_bus[15];
					Tipo_Vehiculo=buffer_bus[16];
					 

					g_cEstadoTxSoft |= LECTURA_WIEG_TX;
					TimeOut_Wiegand=5;

 					
				}
				else if ((buffer_bus[0]==0x02)&&(buffer_bus[1]=='W'))	//	
				{
					iTimeEsperaRtaLPR=0;
					
					Dato_OffLine=0;
					SalidaW=0;

 					buffer_wie[0]=buffer_bus[4];
					buffer_wie[1]=buffer_bus[3];
					buffer_wie[2]=buffer_bus[2];
					g_cEstadoTxSoft |= LECTURA_WIEG_TX;
					TimeOut_Wiegand=5;

 					
				}
				else if ((buffer_bus[0]==0x02)&&((buffer_bus[1]=='M')))	//	
				{

					iTimeEsperaRtaLPR=0;
					 lcd_wiegand();
				//	lcd_text(0,0,(unsigned char *) "LECT.           ");
  				//	cont(0x87);						   	// Visualiza FC + ID
//ojo					ve_fc(buffer_wie[0]);
				//	cont(0x8b);						   	// Visualiza FC + ID
//					ve_id(buffer_wie[1],buffer_wie[2]);

					Dato_OffLine=0;
 					buffer_wie[0]=buffer_bus[4];
					buffer_wie[1]=buffer_bus[3];
					buffer_wie[2]=buffer_bus[2];
					g_cEstadoTxSoft |= LECTURA_WIEG_TX;
					TimeOut_Wiegand=5;
					TimeOut_Send_Acceso=cte_seg*5;

				}
				else if ((buffer_bus[0]==0x02)&&(buffer_bus[1]=='w')&&(buffer_bus[num_data-1]==ETX))	//	 inf del mc aux    jp
				{
					
					iTimeEsperaRtaLPR=0;

					Dato_OffLine=1;
					Off_Line_Salida=0;
					Dato_Placa=0;

					// buffer_bus[5];    Parte del codigo de la tarjeta 4 bytes
				
 					buffer_wie[0]=buffer_bus[4];
					buffer_wie[1]=buffer_bus[3];
					buffer_wie[2]=buffer_bus[2];

					YearIn=buffer_bus[6];
					MonthIn=buffer_bus[7];
					DayIn=buffer_bus[8];
					HourIn=buffer_bus[9];
					MinutIn=buffer_bus[10];
	
					if (num_data==18)
					{
						YearOut=buffer_bus[11];
						MonthOut=buffer_bus[12];
						DayOut=buffer_bus[13];
						HourOut=buffer_bus[14];
						MinutOut=buffer_bus[15];
						Tipo_Vehiculo=buffer_bus[16];
						Off_Line_Salida=1;
						Dato_Placa=0;
															 // ojo jp
					   //lcd_text(1,0,(unsigned char *) "q pasa");
					   //while(1){};//jp
					}
					else if (num_data>18)
					{

						YearOut=buffer_bus[11];
						MonthOut=buffer_bus[12];
						DayOut=buffer_bus[13];
						HourOut=buffer_bus[14];
						MinutOut=buffer_bus[15];

						Tipo_Vehiculo=buffer_bus[num_data-2];
							// lcd_text(1,0,(unsigned char *) buffer_bus[num_data-2]);  //jp
						//	 while(1){};//jp
						Off_Line_Salida=1;
						Dato_Placa=1;
						NumDatosPlate=0;
						Max_Len_Placa=10;
	
						for (k=16; k<num_data-2; k++)
						{
						 	if (buffer_bus[k]!=ETX)
							{
	 							buffer_placa[k-16]=buffer_bus[k];
								NumDatosPlate++;
							}
							else
							{
								k=num_data;
							}
							
						}
					}
					else
					{
						Tipo_Vehiculo=buffer_bus[11];
						//	 lcd_text(1,0,(unsigned char *) buffer_bus[11]);  //jp
						//	 while(1){};//jp
						Off_Line_Salida=0;
 					}
					

					g_cEstadoTxSoft |= LECTURA_WIEG_TX;
					TimeOut_Wiegand=10;
 
				}
				else if ((buffer_bus[0]==STX)&&((buffer_bus[1]=='P')||(buffer_bus[1]=='T')||(buffer_bus[1]=='V')||(buffer_bus[1]=='E')||(buffer_bus[1]=='G')))	//	
				{
					iTimeEsperaRtaLPR=0;
					
					Rechazo=buffer_bus[1];
					len_buffer=num_data;
//					len_buffer=buffer_bus[2];  							//	Numero de Datos recibidos del Secunadario
					BorraLCD_L1();
					send_markCashierAut=0;							   	// Incializa Marca de Liquidacion en Cajero
					Flag_Dcto=0;									   	// Inicializa bandera de Salida con descuento
					cont(0x80);										  	// Visualiza Lectura (hasta 1er separador)
					for (k=3; k<len_buffer; k++)						// Chequea datos
					{
						if (((buffer_bus[k]>='0')&&(buffer_bus[k]<='9'))||((buffer_bus[k]>='A')&&(buffer_bus[k]<='Z'))||(buffer_bus[k]=='-')||(buffer_bus[k]=='.'))
						{
						 	datos_validos=1;
						}
						else
						{
							datos_validos=0;
							k=len_buffer+1;
						}
						vdato(buffer_bus[k]);
 					}
					if 	(datos_validos==1)
					{
//-------------------------------------------------------------------------------------------------------------------------------------
						i=0;
						Tipo_Vehiculo=0;								 // Borra Tipo
						for (k=0; k<len_buffer; k++)					 // Busca Separador de Datos para Tipo Vehiculo
						{
					 		if (buffer_bus[k]=='.')
							{
								Tipo_Vehiculo=buffer_bus[k+1]; 			// Almacena Tipo Vehiculo de la tarjeta
								k=len_buffer;
							}
						}
						if (Tipo_Vehiculo!=0)
						{
							len_buffer=len_buffer-0x05;	  				// Quita: STX Letra #  punto TipoVehiculo = 5 Caracteres del total
						}
//-------------------------------------------------------------------------------------------------------------------------------------- 
 						g_cEstadoTxSoft |= LECTURA_COD_TX;
						
						for (k=0; k<len_buffer; k++)
						{
							buffer_ticket[k]=buffer_bus[k+3]; 			//
						}

						//Habilita_Lectura=0; 							// Inhabilita Aproximacion
						//lcd_text(1,0,(unsigned char *) "APX. NO ACTIVA  ");
					}
					TimeOutLinea=TIMEW;
					TimeOut_Codigo=30;
				}
//-------------------------------------------------------------------------------------------------------------------------------------*
				else if	((buffer_bus[0]==STX)&&((buffer_bus[1]=='a'))&&(buffer_bus[num_data-1]==ETX))	//		  STX "a" Tiquete - Fecha_In - LPR ETX
				{
 						
//						tx_chr(ACK);

						iTimeEsperaRtaLPR=0;


						Tiquete_Placa=1;
						Tiquete_Salida=0;
						NumChrTicket=0;

						for (k=2; k<num_data; k++)					 	// Busca Separador de Datos para Tipo Vehiculo
						{
					 		if (buffer_bus[k]=='-')
							{
								Ini_Fecha=k+1;
								k=num_data;
							}
						}
						
					

						for (k=2; k<Ini_Fecha-1; k++)
						{
												
							buffer_ticket[k-2]=buffer_bus[k]; 			//
							NumChrTicket++;
						}
					  

					   	YearIn=buffer_bus[Ini_Fecha++];
						MonthIn=buffer_bus[Ini_Fecha++];
						DayIn=buffer_bus[Ini_Fecha++];
						HourIn=buffer_bus[Ini_Fecha++];
						MinutIn=buffer_bus[Ini_Fecha++];


						Num_Char_LPR=0;
						for (k=Ini_Fecha+1; k<num_data-1; k++)
						{
							buffer_placa[Num_Char_LPR++]=buffer_bus[k];
						}
						
												
						g_cEstadoTxSoft |= LECTURA_COD_TX;
						TimeOutLinea=TIMEW;
						TimeOut_Codigo=30;						


				}
//-------------------------------------------------------------------------------------------------------------------------------------*
				else if	(((buffer_bus[0]==STX)&&(buffer_bus[num_data-1]==ETX))&&((buffer_bus[1]=='m')||(buffer_bus[1]=='n')||(buffer_bus[1]=='o')||(buffer_bus[1]=='A')||(buffer_bus[1]=='R')))	//  STX *** "m" "n" "o" *** ETX
				{
 					if (buffer_bus[1]=='n')
					{
					 	iTimeEsperaRtaLPR=0;
						
						ACCESO_USE_LPR=0;
						cont(0x80);
	 					for (k=0;MSGnotify[k]!='\0';k++)
						{
							vdato(MSGnotify[k]);
						}
						cont(0x8D);
						(notifyEVP==1)?(vdato('N')):(vdato('S'));
						
						vdato(' ');
						vdato('N');


					}
					else if (buffer_bus[1]=='m')
					{
					 	ACCESO_USE_LPR=1;

						cont(0x80);
	 					for (k=0;MSGnotify[k]!='\0';k++)
						{
							vdato(MSGnotify[k]);
						}
						cont(0x8D);
						(notifyEVP==1)?(vdato('N')):(vdato('S'));
						
						vdato(' ');
						vdato('M');
						



					}
					else if (buffer_bus[1]=='o')
					{
					 	if (buffer_bus[2]=='0')
						{
						 	ACCESO_USE_LPR=0;
							iTimeEsperaRtaLPR=0;

							cont(0x80);
		 					for (k=0;MSGnotify[k]!='\0';k++)
							{
								vdato(MSGnotify[k]);
							}
							cont(0x8D);
							(notifyEVP==1)?(vdato('N')):(vdato('S'));
							
							vdato(' ');
							vdato('o');

						}
						else if	(buffer_bus[2]=='1')
						{
							ACCESO_USE_LPR=1;
							cont(0x80);
		 					for (k=0;MSGnotify[k]!='\0';k++)
							{
								vdato(MSGnotify[k]);
							}
							cont(0x8D);
							(notifyEVP==1)?(vdato('N')):(vdato('S'));
							
							vdato(' ');
							vdato('M');
						}
					}
					else if (buffer_bus[1]=='A')   						// Acepta de LPR
					{
						if (iTimeEsperaRtaLPR!=0)
						{
							iTimeEsperaRtaLPR=0;

							buffer_wie[0]=buffer_wieLPR[0];
							buffer_wie[1]=buffer_wieLPR[1];
							buffer_wie[2]=buffer_wieLPR[2];
							g_cEstadoTxSoft |= LECTURA_WIEG_TX;
							TimeOut_Wiegand=5;
							SalidaW=0;
						}
 
					}
					else if (buffer_bus[1]=='R') 						// Rechaza de LPR
					{
				   			iTimeEsperaRtaLPR=0;
							cont(0x80);
		 					for (k=0;MSGnegado[k]!='\0';k++)
							{
								vdato(MSGnegado[k]);
							}
 
					}
					buffer_bus[1]=0xff;									
 
				}
//-------------------------------------------------------------------------------------------------------------------------------------*
				else if	((buffer_bus[0]==STX)&&((buffer_bus[1]=='s')||(buffer_bus[1]=='C'))&&(buffer_bus[num_data-1]==ETX))	//		  STX "s" Tiquete - Fecha_In - Fecha Out - LPR ETX
				{
 						
						iTimeEsperaRtaLPR=0;

						Tiquete_Placa=1;
						Tiquete_Salida=1;
						NumChrTicket=0;

						if (buffer_bus[1]=='C')
						{
							send_markCashierAut=1;
						}
						else
						{
							send_markCashierAut=0;
						}



						for (k=2; k<num_data; k++)					 	// Busca Separador de Datos para Tipo Vehiculo
						{
					 		if (buffer_bus[k]=='-')
							{
								Ini_Dcto=k+1;
								k=num_data;
							}
						}

						Cod_Dcto=buffer_bus[Ini_Dcto];

						for (k=2; k<Ini_Dcto-1; k++)
						{
							buffer_ticket[k-2]=buffer_bus[k]; 			//
							NumChrTicket++;
						}

						
						Ini_Fecha=Ini_Dcto+2;


					   	YearIn=buffer_bus[Ini_Fecha++];
						MonthIn=buffer_bus[Ini_Fecha++];
						DayIn=buffer_bus[Ini_Fecha++];
						HourIn=buffer_bus[Ini_Fecha++];
						MinutIn=buffer_bus[Ini_Fecha++];

						Ini_Fecha++;

						YearOut=buffer_bus[Ini_Fecha++];
						MonthOut=buffer_bus[Ini_Fecha++];
						DayOut=buffer_bus[Ini_Fecha++];
						HourOut=buffer_bus[Ini_Fecha++];
						MinutOut=buffer_bus[Ini_Fecha++];

						Ini_Fecha++;

						Tipo_Vehiculo=buffer_bus[Ini_Fecha++];

						Num_Char_LPR=0;
												
						g_cEstadoTxSoft |= LECTURA_COD_TX;
						TimeOutLinea=TIMEW;
						TimeOut_Codigo=30;
											


				}
				else if ((buffer_bus[0]==0x02)&&(buffer_bus[1]=='d')&&(buffer_bus[num_data-1]==ETX)&&(num_data==9))	//		DINERO de SALDO
				{
					BorraLCD_L1();	
				   	cont(0x80);	

					vdato('S');
					vdato('a');
					vdato('l');
					vdato('d');
					vdato('o');
 					vdato(' ');
					vdato('D');
					vdato('i');
					vdato('s');
					vdato('p');
 					vdato('o');
					vdato('n');
					vdato('i');
					vdato('b');
					vdato('l');
					vdato('e');	

					BorraLCD_L2();
					cont(0xc0);
					
					for (k=2; k<8; k++)
					{
						vdato(buffer_bus[k]);
					}				
				}
				else													//
				{
//-------------------------------------------------------------------------------------------------------------------------------------*
					len_buffer=num_data;  								//	Numero de Datos recibidos del Secunadario
					BorraLCD_L1();
//-------------------------------------------------------------------------------------------------------------------------------------*
					Tipo_Vehiculo=0;								 	// Borra Tipo
					for (k=0; k<len_buffer; k++)					 	// Busca Separador de Datos para Tipo Vehiculo
					{
					 	if (buffer_bus[k]=='.')
						{
							Tipo_Vehiculo=buffer_bus[k+1]; 				// Almacena Tipo Vehiculo de la tarjeta
							k=len_buffer;
						}
					}
					if (Tipo_Vehiculo!=0)								// H T B M C (LETRAS DEL TIPO DE VEHICULO)
					{
							len_buffer=len_buffer-0x02;	  				// Quita: punto TipoVehiculo = 2 Caracteres del total
					}
//--------------------------------------------------------------------------------------------------------------------------------------
					send_markCashierAut=0;							   // Incializa Marca de Liquidacion en Cajero
					Flag_Dcto=0;									   // Inicializa bandera de Salida con descuento
					i=0;
					for (k=0; k<len_buffer; k++)					   // Busca Separador de Datos
					{
					 	if (buffer_bus[k]=='-')
						{
							PosComa[i++]=k;
						}
					}
//--------------------------------------------------------------------------------------------------------------------------------------
					(i!=0)?(NumDatValidar=PosComa[0]+1):(NumDatValidar=len_buffer);	 	// EXTRAE SOLO EL CODIGO PARA VALIDAD DATOS
//--------------------------------------------------------------------------------------------------------------------------------------
					cont(0x80);										  	// Visualiza Lectura (hasta 1er separador)
					for (k=0; k<NumDatValidar; k++)						// Chequea datos
					{
						if (((buffer_bus[k]>='0')&&(buffer_bus[k]<='9'))||((buffer_bus[k]>='A')&&(buffer_bus[k]<='Z'))||(buffer_bus[k]=='-')||(buffer_bus[k]=='.'))
						{
						 	datos_validos=1;
						}
						else
						{
							datos_validos=0;
							k=NumDatValidar+1;
						}
						vdato(buffer_bus[k]);
 					}
//----------------------------------------------------------------------
					if 	(datos_validos==1)
					{
 						iTimeEsperaRtaLPR=0;
						
						g_cEstadoTxSoft |= LECTURA_COD_TX;
					
						//Habilita_Lectura=0; 							// Inhabilita Aproximacion
						//lcd_text(1,0,(unsigned char *) "APX. NO ACTIVA  ");
				
						if (i!=0)
						{
							if (i==1)
							{
							 	if ((len_buffer-1-PosComa[0])==1)				// Solo Cod Descuento, Evalua cantidad de datos si es 1 xq es solo codigo Dcto
								{
									for (k=0; k<len_buffer; k++)
									{
										buffer_ticket[k]=buffer_bus[k];
							
	 								}
									Flag_Dcto=1;
									Cod_Dcto= buffer_ticket[len_buffer-1];
									vdato('D');
									vdato(Cod_Dcto);
									len_buffer=len_buffer-2;								
								}
								else											// Solo Liquidacion en Cajero Posee mas de un dato, o sea la fecha
								{
									Flag_Dcto=0;
									for (k=0; k<PosComa[0]; k++)
									{
										buffer_ticket[k]=buffer_bus[k];
										
									}
																		
									if ((len_buffer-1-PosComa[0])==5)			// 5 datos de: YMDHM  (Año Mes Dia Hor Min)
									{
										send_markCashierAut=1;

										vdato('C');
										vdato('a');
										vdato('j');
										vdato('e');
										vdato('r');
										vdato('o');

										Ini_Fecha=PosComa[0]+1;	
																			
										YearIn=buffer_bus[Ini_Fecha++];
										MonthIn=buffer_bus[Ini_Fecha++];
										DayIn=buffer_bus[Ini_Fecha++];
										HourIn=buffer_bus[Ini_Fecha++];
										MinutIn=buffer_bus[Ini_Fecha];
										
									}
									len_buffer=PosComa[0];
								}	
							}
							else 		   											// Posee Codigo de descuento y Liquidado en Cajero
							{
							  	for (k=0; k<PosComa[0]; k++)
								{
									buffer_ticket[k]=buffer_bus[k];
							 	}
								
								Flag_Dcto=1;
								Cod_Dcto= buffer_ticket[PosComa[0]+1];
								
								
								send_markCashierAut=1;
								Ini_Fecha=PosComa[1]+1;
																		
								YearIn=buffer_bus[Ini_Fecha++];
								MonthIn=buffer_bus[Ini_Fecha++];
								DayIn=buffer_bus[Ini_Fecha++];
								HourIn=buffer_bus[Ini_Fecha++];
								MinutIn=buffer_bus[Ini_Fecha];

								len_buffer=PosComa[0];																
							}
						}
						else														// Dato sin nada fuera de lo normal.
						{
							for (k=0; k<len_buffer; k++)
							{
								buffer_ticket[k]=buffer_bus[k];
							}
						   	Flag_Dcto=0;
							send_markCashierAut=0;
						}

						TimeOutLinea=TIMEW;
						TimeOut_Codigo=30;
					}
//---------------------------------------------------------------------					
  				}
			}
			else
			{
//				cont(0x8E);
//				temp=(buffer_bus[0]&0xf0);
//				temp>>=4;
//				vdato(temp);
//				vdato((buffer_bus[0]&0x0f)+0x30);

			}
 		}

		if(TF0==1)
		{
			TF0=0;
//----------------------------------------------------------------------------------------------*
			if (TimeRetryCmd!=0)
			{
			 	TimeRetryCmd--;
				if (TimeRetryCmd==0)
				{
					RetryCmd=1;
				}
			}
//----------------------------------------------------------------------------------------------*
			if (TimeOut_Send_Acceso!=0)
			{
				TimeOut_Send_Acceso--;
			}
			if (OpenMensual_Apx!=0)
			{
			 	OpenMensual_Apx--;
			}
//----------------------------------------------------------------------------------------------*
			if (iTimeEsperaRtaLPR!=0)
			{
				iTimeEsperaRtaLPR--;
				if (iTimeEsperaRtaLPR==0)
				{
						buffer_wie[0]=buffer_wieLPR[0];
						buffer_wie[1]=buffer_wieLPR[1];
						buffer_wie[2]=buffer_wieLPR[2];
						g_cEstadoTxSoft |= LECTURA_WIEG_TX;
						TimeOut_Wiegand=5;
						SalidaW=0;
				}
			}
//----------------------------------------------------------------------------------------------*
			if (SignalAcceso==0)
			{
				if ((seg==cte_seg/2)||(seg==0))
				{
					if (toggle==1)
					{
						ledv=0;
						toggle=0;
					}
					else
					{
						ledv=1;
						toggle=1;
					}
				}
			}
			else
			{
			 	ledv=1;
				Habilita_Lectura=1;								// Habilita Lectura
			}
//----------------------------------------------------------------------------------------------*
			seg--;
			if (seg==0)
			{
  				if (FueraLinea==1)
				{
					if (Seg_OFF!=0)
					{
						Seg_OFF--;
					}
					if (Seg_OFF==0)
					{
 						buffer_bus[0]=OFF_LINE;		
						tx_bus(1);
						Seg_OFF=5;
					}
   				}
				else
				{
					Seg_OFF=5;	
 				}
   				if (TimeOut_Codigo!=0)
				{
			 		TimeOut_Codigo--;
					if (TimeOut_Codigo==0)
					{
						g_cEstadoTxSoft &=~LECTURA_COD_TX;
					 	Rechazo=0;

						Tiquete_Placa=0;
						Tiquete_Salida=0;
						Tipo_Vehiculo=' ';
						send_markCashierAut=0;
						if (FueraLinea==1)
						{
							BorraLCD_L1();
  						}
					}
				}
  				if (TimeOut_Wiegand!=0)
				{
			 		TimeOut_Wiegand--;
					if (TimeOut_Wiegand==0)
					{
						g_cEstadoTxSoft &=~LECTURA_WIEG_TX;
						SalidaW=0;
						Dato_Placa=0;
//						Off_Line_Salida=0;
					}
				}
	 			if (TimeOutLinea!=0)
				{
					TimeOutLinea--;
					if (TimeOutLinea==0)  										/*fuera de linea*/
					{
						TimeOutLinea=TIMEW;
						cont(0x80);
	 					for (k=0;k<16;k++)
						{
	  						vdato(' ');
						}
						g_cEstadoTxSoft &=~LECTURA_COD_TX;
						g_cEstadoTxSoft &=~LECTURA_WIEG_TX;
						g_cEstadoTxSoft &=~COD_PRINT_TX;
						FueraLinea=1;
						Rechazo=0;
						Dato_Placa=0;
						send_markCashierAut=0;
						Tiquete_Placa=0;
						Tiquete_Salida=0;
						Dato_Placa=0;
						cont(0xc0);
	 					for (k=0;linea[k]!='\0';k++)
						{
	  						vdato(linea[k]);
						}
						buffer_bus[0]=OFF_LINE;		
						tx_bus(1);
					}
 				}
				 

				Atascado=0;
				lock1=0;
				lock2=0;

				audio1=0;
				msg1=0;
				audio2=0;
				msg2=0;
				audio3=0;
				msg3=0;
				audio4=0;
				msg4=0;

			
				seg=cte_seg;
				TH0=0X00;								//Inicializa timer									*						
				TL0=0X00;
				TF0=0;
				msg_error=0;


			}

		}
//***********************************************************************************************************
		if ((RetryCmd==1)&&(ready==1))
		{
   			RetryCmd=0;
			for (temp=0; temp<NumDatRetry; temp++)
			{
				buffer_bus[temp]=BufferRetry[temp];
			}
			tx_bus(NumDatRetry);
		}
//***********************************************************************************************************
		if (txACK==1)
		{
			tx_chr(ACK);	 											
			txACK=0;
 		}
//***********************************************************************************************************
 		if( ((g_cEstadoComSoft==POLL_COM_SOF)||(g_cEstadoComSoft==ANALICE_STR_SOF)))  
		{																				 
	

			AtencComSoft();
		}
//***********************************************************************************************************
		if ((Tx_Acceso==1)&&(ready==1))
		{	
			Tx_Acceso=0;
			buffer_bus[0]=ACCESO;
			tx_bus(1);
		
		//	lcd_text(0,0,(unsigned char *) "ENVIANDO ACCESO ");

  			buffer_bus[0]=0x00;
			tx_bus(1);
		}						
//***********************************************************************************************
		relevos_aux();
 	}
  }