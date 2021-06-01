#include <reg51.h>
#include "pto_paralelo.h"

sbit bus_clk=P3^6;
sbit busy=P3^5;		  				//														*
sbit ready=P3^4;	
#define T_MS		20   				// Base para 1ms en Espera a tx Bus
#define MAX_CHR		30   				// Maximo Numero CHR a recibir del secundario
extern unsigned char xdata buffer_bus[];
extern unsigned char num_data;


extern void wait_ancho (void);
extern void wait_long (void);
/*-------------------------------------------------------------------------------------------------------------

---------------------------------------------------------------------------------------------------------------*/
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
