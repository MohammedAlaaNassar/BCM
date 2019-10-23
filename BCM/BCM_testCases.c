/*
 * BCM_RX_testCases.c
 *
 *  Created on: Oct 20, 2019
 *      Author: Mohammed Nassar
 *
 *
 */
#include "LCD.h" // FOR  TESTING
#include "BCM.h"
#include "BCM_Pbcfg.h"


#define RX               // define TX  for TX test Cases  OR  RX for  RX test Cases



#ifdef TX

void print_Screen(void)
{
	LCD_clearScreen();
	LCD_displayString("SENT");
}


#endif
#ifdef RX

uint8 g_data_address[50]={0};

void app(void)
{
	LCD_clearScreen();
	LCD_displayString("Data received");    // FOR  TESTING
     _delay_ms(300);
	LCD_clearScreen();     // FOR  TESTING

for(uint8 i=0; i <10 ;i++)
{
	LCD_intgerToString(g_data_address[i]); // FOR  TESTING
}
}

void error(EnmBCMError_t BCM_ERROR)
{
	LCD_clearScreen();
	switch(BCM_ERROR)
	{
	case BCM_INVALID_CFG:LCD_displayString("BCM INVALID CFG"); // FOR  TESTING
	break;
	case BCM_UART_ERROR:LCD_displayString("BCM UART ERROR");// FOR  TESTING
	break;
	case BCM_INVALID_DATA:LCD_displayString("BCM INVALID DATA");// FOR  TESTING
	break;
	case BCM_MODULE_BUSY:LCD_displayString("BCM MODULE BUSY"); // FOR  TESTING
	break;
	case BCM_WRONG_ID: LCD_displayString("BCM WRONG ID");// FOR  TESTING
	break;
	case BCM_SMALL_DATA_BUFFER_SIZE: LCD_displayString("BCM SMALL DATA BUFFER SIZE"); // FOR  TESTING
	break;
	case BCM_FRAMING_ERROR: LCD_displayString("BCM FRAMING ERROR");// FOR  TESTING
	break;
	default:
	break;
	}
}

#endif

int main()
{
    LCD_init(); // FOR  TESTING
    LCD_clearScreen(); // FOR  TESTING

	BCM_Init (&BCM);

#ifdef RX

	uint8* data_address = g_data_address ;
	BCM_RX_Setup( data_address,50, app , error );

#endif
#ifdef TX

	_delay_ms(2000);
	uint8 data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	BCM_Send(data, 10, print_Screen);


#endif

	while(1){

	BCM_RxDispatch();
	BCM_TxDispatch();
#ifdef TX
_delay_ms(2000);
#endif

	}

return 0;
}
