/*
 * UART.c
 *
 *  Created on: Oct 15, 2019
 *      Author: Mohammed
 */


#include "micro_config.h"
#include "std_types.h"
#include "common_macros.h"
#include "UART.h"


/*  AVAILABITITY  */
#define AVAILABLE     1
#define NOT_AVAILABLE 0

/* REQUEST */
#define REQUESTED     1
#define NOT_REQUESTED 0

static volatile void_func_ptr g_TX_ptr = NULL;
static volatile uint8_func_ptr g_RX_ptr = NULL;
static volatile uint8 g_data;
static volatile uint8 UDR_availability_flag = AVAILABLE ;
static volatile uint8 UDR_request_flag = NOT_REQUESTED ;

UART_error_t UART_init(UART_cfgPtr_t UART)
{
	UART_error_t retval = UART_OK ;

    SET_BIT(UCSRC,URSEL); // URSEL= 1 The URSEL must be one when writing the UCSRC

		/* USART MODE */
		switch(UART->USART_mode)
		{
		case SYNCHRONOUS  : SET_BIT(UCSRC,UMSEL); // SYNCHRONOUS
		break;
		case ASYNCHRONOUS : CLEAR_BIT(UCSRC,UMSEL); // ASYNCHRONOUS
		break;
		default: retval = UART_INVALID_CFG ;
		break;
		}

		/* COMMUNICATION MODEL */
		switch(UART->communication_model)
		{
		case SLAVE : SET_BIT(UCSRB,RXEN); // Receiver Enable
		break;
		case MASTER : SET_BIT(UCSRB,TXEN); // Transmitter Enable
		break;
		case P2P : SET_BIT(UCSRB,TXEN); // Transmitter Enable
	               SET_BIT(UCSRB,RXEN); // Receiver Enable
		break;
		default: retval = UART_INVALID_CFG ;
		break;
		}

		/*  DATA SIZE  */
		switch(UART->data_size)
		{
		case FIVE_BITS : UCSRC &= ~( (1<<UCSZ2) | (1<<UCSZ1) | (1<<UCSZ0) ); // UCSZ2=0 UCSZ1=0 UCSZ0=0
		break;
		case SIX_BITS : UCSRC &= ~( (1<<UCSZ2) | (1<<UCSZ1) ); // UCSZ2=0 UCSZ1=0
		UCSRC |=  (1<<UCSZ0) ; // UCSZ0=1
		break;
		case SEVEN_BITS : UCSRC &= ~( (1<<UCSZ2) | (1<<UCSZ0) ); // UCSZ2=0 UCSZ0=0
		UCSRC |=  (1<<UCSZ1) ; // UCSZ1=1
		break;
		case EIGHT_BITS : UCSRC &= ~ (1<<UCSZ2) ; // UCSZ2=0
		UCSRC |=  (1<<UCSZ1) | (1<<UCSZ0) ; // UCSZ1=1 UCSZ0=1
		break;
		case NINE_BITS : UCSRC |= (1<<UCSZ2) | (1<<UCSZ1) | (1<<UCSZ0) ; //UCSZ2=1 UCSZ1=1 UCSZ0=1
		break;
		default: retval = UART_INVALID_CFG ;
		break;
		}

		/* PARITY */
		switch(UART->parity)
		{
		case DISABLED : UCSRC &= ~( (1<<UPM1) | (1<<UPM0) );  // UPM1=0 UPM0=0 ODD DISABLED
		break;
		case ODD : UCSRC |=  (1<<UPM1) | (1<<UPM0) ; // UPM1=1 UPM0=1 ODD PARITY
		break;
		case EVEN : UCSRC &= ~(1<<UPM0); // UPM0=0 EVEN PARITY
		UCSRC |=  (1<<UPM1); // UPM1=1 EVEN PARITY
		break;
		default: retval = UART_INVALID_CFG ;
	    break;
		}

		/*  STOP PATTERN  */
		switch(UART->stop_pattern)
		{
		case BIT : CLEAR_BIT(UCSRC,USBS); // STOP BIT
		break;
		case TWO_BITS : SET_BIT(UCSRC,USBS); // 2 STOP
		break;
		default: retval = UART_INVALID_CFG ;
		break;
		}

		/* TRANSMISSION SPEED */
		switch(UART->tarnsmission_speed)
		{
		case DOUBLE_SPEED : SET_BIT(UCSRA,U2X); // DOUBLE SPEED
		break;
		case NORMAL_SPEED :
		case NA :    CLEAR_BIT(UCSRA,U2X); // NORMAL SPEED OR SYNCHRONOUS
		break;
		default: retval = UART_INVALID_CFG ;
		break;
		}


		/* INTERRUPT */
		switch(UART->interrupt)
		{
		case  POOLING : CLEAR_BIT(UCSRB,RXCIE); // RX COMPLETE DISABLED
		                CLEAR_BIT(UCSRB,TXCIE); // TX COMPLETE  DISABLED
		break;

		case INTERRUPT : {
			sei();  // Global INTERRUPT Enable
			if (UART->communication_model == SLAVE )
			{
				SET_BIT(UCSRB,RXCIE); // RX COMPLETE INTERRUPT ENABLED
			}
			else if (UART->communication_model== MASTER )
			{
				SET_BIT(UCSRB,TXCIE); // TX COMPLETE ENABLED
				//SET_BIT(UCSRB,UDRIE); // Buffer is Empty INTERRUPT ENABLED
			}
			else if (UART->communication_model== P2P )
			{
				SET_BIT(UCSRB,RXCIE); // RX COMPLETE INTERRUPT ENABLED
				SET_BIT(UCSRB,TXCIE); // TX COMPLETE INTERRUPT ENABLED
				//SET_BIT(UCSRB,UDRIE); // Buffer is Empty INTERRUPT ENABLED
			}
			else
			{
		        retval = UART_INVALID_CFG ;
			}
			break;
		}

		default: retval = UART_INVALID_CFG ;
		break;
		}

		/*  CLK EDGE  */
		switch(UART->clk_edge)
		{
		case RISING :
		case NA : CLEAR_BIT(UCSRC,UCPOL); // RISING XCK Edge OR Asynchronous
		break;
		case FALLING : SET_BIT(UCSRC,UCPOL); // FALLING XCK Edge
		break;
		default: retval = UART_INVALID_CFG ;
		break;
		}


		/*  BAUD RATE  */

	/* First 8 bits from the BAUD_PPULLCALE inside UBRRL and last 4 bits in UBRRH*/
		//UBRRH = BAUD_PRESCALE>>8;
		//UBRRL = BAUD_PRESCALE;
		UBRRH = 0;
		UBRRL = 51;

		return retval;
}



UART_error_t UART_sendByte(const uint8 data)
{
	UART_error_t retval = UART_OK ;
	if  (UART.interrupt== POOLING  )
	{

		while(BIT_IS_CLEAR(UCSRA,UDRE)){} // UDRE flag is set when the TX buffer (UDR) is empty

		UDR = data; ///Put the required data in the UDR register

	}
	else if  (UART.interrupt== INTERRUPT  )
	{
		if(UDR_availability_flag == AVAILABLE)
		{
			UDR = data ;
			UDR_availability_flag = NOT_AVAILABLE;
		}
		else
		{
			g_data = data;
			UDR_request_flag = REQUESTED;
		}

	}
	return retval;
}


UART_error_t UART_receiveByte(uint8 * data)
{
	UART_error_t retval = UART_OK ;
	if ( UART.interrupt== POOLING  )
	{
		while(BIT_IS_CLEAR(UCSRA,RXC)){} //RXC flag is set when the UART receive data
		*data=UDR; // Read the received data from  (UDR)
	}

   return retval;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////

UART_error_t UART_TX_set_callback(void_func_ptr app_ptr)
{
	g_TX_ptr = app_ptr;

	return UART_OK ;
}

UART_error_t UART_RX_set_callback( uint8_func_ptr app_ptr )
{
	g_RX_ptr = app_ptr;
	return UART_OK ;
}



ISR(USART_RXC_vect)
{
	if (g_RX_ptr != NULL)
	{
	g_RX_ptr(UDR);
	}
}


ISR(USART_TXC_vect)
{
	if ( UDR_request_flag == REQUESTED )
	{
		UDR = g_data;
		UDR_request_flag = NOT_REQUESTED ;
	}
	else
	{
		UDR_availability_flag = AVAILABLE;
	}

	if (g_TX_ptr != NULL)
	{
	g_TX_ptr();
	}
}
