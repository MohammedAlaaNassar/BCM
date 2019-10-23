/*
 * BCM.c
 *
 *  Created on: Oct 20, 2019
 *      Author: Mohammed Nassar
 */

#include "UART.h"
#include "UART_Pbcfg.h"
#include "BCM.h"
#include "BCM_cfg.h"

typedef struct {
	uint8 BCM_ID;
	uint16 data_size;
	uint8* data_address;
	uint8 checksum;
	}ST_BCM_buffer;

static ST_BCM_buffer BCM_TX_buffer;                        // TX buffer
static ST_BCM_buffer BCM_RX_buffer;                       // RX buffer

typedef enum{
	TX_IDLE,
	SENDING_BYTE,
	SEND_BYTE_COMPLETE,
	TX_FRAME_COMPLETE,
	}BCM_TX_FSM_t;

static BCM_TX_FSM_t TX_state = TX_IDLE;               // TX state

typedef enum{
	RX_IDLE,
	RECEIVING_BYTE,
	RECEIVE_BYTE_COMPLETE,
	RX_FRAME_COMPLETE,
}BCM_RX_FSM_t;

static BCM_RX_FSM_t RX_state = RX_IDLE;               // RX state

typedef enum{
	BCM_LOCKED,
	BCM_UNLOCKED,
}buffer_state_t;

static buffer_state_t TX_buffer_state = BCM_UNLOCKED; // TX buffer state
static buffer_state_t RX_buffer_state = BCM_UNLOCKED; // RX buffer state

// **** BCM_TX PRIVATE VARIABLES ***//

static uint8 cfg_pointer = 0;
static uint16 data_pointer = 0;
static BCM_Enable_t data_flow = BCM_DISABLED;
static void_func_ptr g_TX_consumer = NULL;
static uint8 FLAG =0;

// **** BCM_TX PRIVATE FUNCTIONS ***//

static void BCM_TX_callConsumer(void);
static void TX_UART_CALLBACK(void);
static EnmBCMError_t BCM_calculate_checksum(uint8* data_address, uint16 data_size);
static EnmBCMError_t BCM_sendNewByte(void);


// **** BCM_RX PRIVATE VARIABLES ***//

static BCM_Enable_t data_flag = BCM_DISABLED ;
static uint16 RX_buffer_Ptr = 0 ;
static uint16 data_size_LB_received = 0 ;
static uint16 data_size_MB_received = 0 ;
static uint16 data_size_received = 0;
static uint8 BCM_RX_checksum = 0 ;
static uint8 g_Data[SIZE_OF_INTERNAL_RX_BUFFER] = {0};
static uint8 g_Data_ISR_INDEX = 0;
static uint8 g_Data_BCM_INDEX = 0;
static void_func_ptr g_RX_consumer = NULL ;
static EnmBCMError_func_ptr g_RX_consumer_error = NULL ;

// **** BCM_RX PRIVATE FUNCTIONS ***//

static void BCM_RX_callConsumer(void);
static void BCM_RX_callConsumer_error(EnmBCMError_t);
static void BCM_Data_Check(uint8);
static void BCM_RX_storingData(void) ;
static void BCM_RX_storingChecksum(void);


EnmBCMError_t BCM_Init(BCM_cfgPtr_t BCM)
{
	EnmBCMError_t retval = BCM_OK;
	UART_error_t UART_error;
	BCM_TX_buffer.BCM_ID = BCM->BCM_ID;
	if(BCM->UART == BCM_ENABLED)
	{
		UART_error = UART_init(&UART);
		if(UART_error)
		{
			retval = BCM_UART_ERROR;
		}
	}
	if(BCM->SPI == BCM_ENABLED)
	{
		//SPI_init(SPI);
	}
	if (BCM->I2C == BCM_ENABLED)
	{
		//I2C_init(I2C);
	}
	return retval;
}


EnmBCMError_t BCM_Send(uint8* data_address, uint16 data_size, void_func_ptr TX_consumer)
{
	EnmBCMError_t retval = BCM_OK;
	if(data_address != NULL)
	{
		if(TX_buffer_state == BCM_UNLOCKED)
		{
			retval = BCM_calculate_checksum(data_address, data_size);
			TX_buffer_state = BCM_LOCKED;
			g_TX_consumer = TX_consumer;
			BCM_TX_buffer.data_size = data_size;
			BCM_TX_buffer.data_address = data_address;
			data_pointer = 0;
			cfg_pointer = 0;
	UART_TX_set_callback(TX_UART_CALLBACK);
			TX_state = SEND_BYTE_COMPLETE;
		}
		else
		{
			retval = BCM_MODULE_BUSY;
		}
	}
	else
	{
		retval = BCM_INVALID_DATA;
	}
	return retval;
}



EnmBCMError_t BCM_TxDispatch(void)
{
	EnmBCMError_t retval = BCM_OK;
	switch(TX_state)
	{
		case TX_IDLE:
		break;

		case SENDING_BYTE:
		                                                // timeout error check
		break;

		case SEND_BYTE_COMPLETE:
			BCM_sendNewByte();                        // check if the byte not send increment counter global
		break;

		case TX_FRAME_COMPLETE:
			cfg_pointer = 0;
			data_flow = 0;
			BCM_TX_callConsumer();
			TX_buffer_state = BCM_UNLOCKED;
			TX_state = TX_IDLE;
		break;
	}
	return retval;
}


static EnmBCMError_t BCM_sendNewByte(void)
{
	EnmBCMError_t retval;
	UART_error_t UART_error;

	if(data_flow == BCM_ENABLED)
	{
		UART_error = UART_sendByte((BCM_TX_buffer.data_address)[data_pointer]);
		DDRC = 0xff;
		PORTC = (BCM_TX_buffer.data_address)[data_pointer];
		if(UART_error)
		{
			retval = BCM_UART_ERROR;
		}
		else
		{
			TX_state = SENDING_BYTE;
			data_pointer++;
			if(data_pointer == BCM_TX_buffer.data_size)
			{
				data_flow = BCM_DISABLED;
			}
		}
	}
	else
	{
		switch(cfg_pointer)
		{
			case 0:
				FLAG++;
				TX_state = SENDING_BYTE;
				UART_error = UART_sendByte(BCM_TX_buffer.BCM_ID);
				DDRC = 0xff;
				PORTC = BCM_TX_buffer.BCM_ID;
				cfg_pointer++;

			break;

			case 1:
				UART_error = UART_sendByte((uint8)(BCM_TX_buffer.data_size)); //not trusted
				DDRC = 0xff;
				PORTC = (uint8)(BCM_TX_buffer.data_size);
				if(UART_error)
				{
					retval = BCM_UART_ERROR;
				}
				else
				{
					cfg_pointer++;
					TX_state = SENDING_BYTE;
				}
			break;

			case 2:
				UART_error = UART_sendByte( (uint8)((BCM_TX_buffer.data_size)>>8) );
				DDRC = 0xff;
				PORTC = (uint8)((BCM_TX_buffer.data_size)>>8);
				if(UART_error)
				{
					retval = BCM_UART_ERROR;
				}
				else
				{
					cfg_pointer++;
					TX_state = SENDING_BYTE;
					data_flow = BCM_ENABLED;
				}
			break;

			case 3:
				UART_error = UART_sendByte(BCM_TX_buffer.checksum);
				DDRC = 0xff;
				PORTC = BCM_TX_buffer.checksum;
				if(UART_error)
				{
					retval = BCM_UART_ERROR;
				}
				else
				{
					TX_state = TX_FRAME_COMPLETE;
				}
			break;
		}
	}
	return retval;
}

static EnmBCMError_t BCM_calculate_checksum(uint8* data_address, uint16 data_size)
{
	EnmBCMError_t retval = BCM_OK;
	uint8 loop_index;
	BCM_TX_buffer.checksum = 0;
	if(data_address == NULL)
	{
		retval = BCM_INVALID_DATA;
	}
	else
	{
		for(loop_index = 0; loop_index < data_size; loop_index++)
		{
			BCM_TX_buffer.checksum +=  data_address[loop_index];
		}
	}
	return retval;
}

static void BCM_TX_callConsumer(void)
{
	if( g_TX_consumer != NULL)
	{
		g_TX_consumer();
	}
}

void TX_UART_CALLBACK(void)
{
	if(TX_state != TX_FRAME_COMPLETE)
	{
		TX_state = SEND_BYTE_COMPLETE;
	}
}



// UART cfg must be 8 bit (ENSURE FOR THAT)


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//**************************  RX  ****************************//

// Setup RX Buffer

EnmBCMError_t BCM_RX_Setup(uint8* data_address,uint16 data_size, void_func_ptr RX_consumer, EnmBCMError_func_ptr RX_consumer_error )
{
	EnmBCMError_t retval = BCM_OK;
 if ( data_flag == BCM_DISABLED )
 {
	 RX_buffer_state = BCM_UNLOCKED ;
	if ( ( data_address != NULL ) && ( data_size != (uint16)0 ) )
	{
		BCM_RX_buffer.data_address = data_address ;
		BCM_RX_buffer.data_size = data_size ;
		BCM_RX_buffer.checksum = 0;
		g_RX_consumer = RX_consumer ;
		g_RX_consumer_error = RX_consumer_error;
		UART_RX_set_callback(BCM_Data_Check);
	}
	else
	{
		retval = BCM_INVALID_DATA;
	}
 }
 else
 {
	 retval = BCM_MODULE_BUSY ;
 }
	return retval;
}


void BCM_Data_Check(uint8 Data)
{

	g_Data[g_Data_ISR_INDEX] = Data;                                   // Immediately STORE UDR
	g_Data_ISR_INDEX++;
	RX_state = RECEIVING_BYTE ;
	                                                                            //g_Data = Data;
}

EnmBCMError_t BCM_RX_Unlock(void)
{
	EnmBCMError_t retval = BCM_OK;
	RX_buffer_state = BCM_UNLOCKED;
	return retval;
}

EnmBCMError_t BCM_RxDispatch(void)
{
	EnmBCMError_t retval = BCM_OK;

	if (g_Data_ISR_INDEX == SIZE_OF_INTERNAL_RX_BUFFER )                 // DON'T OVER FLOW -----> if BUFFER FULL GO TO INDEX 0
	{
		g_Data_ISR_INDEX = 0 ;
	}


	switch(RX_state)
	{

	case RECEIVING_BYTE: // save data count ++     count+1 +!1 lenght and check data length


		if  ( ( RX_buffer_Ptr == 0 ) && ( BCM.BCM_ID == g_Data[g_Data_BCM_INDEX] ) && ( RX_buffer_state == BCM_UNLOCKED ) &&  ( data_flag == BCM_DISABLED ) )
		{
			BCM_RX_buffer.BCM_ID = g_Data[g_Data_BCM_INDEX] ;
			data_flag = BCM_ENABLED ;
			RX_state = RECEIVE_BYTE_COMPLETE ;
		}

		else if ( ( RX_buffer_Ptr > 0 ) && ( RX_buffer_Ptr < 3 )  &&  ( data_flag == BCM_ENABLED ) )     // RECEIVING data size
		{
			switch(RX_buffer_Ptr)
			{
			case 1: data_size_LB_received = (uint16)g_Data[g_Data_BCM_INDEX] ;
			break;

			case 2: data_size_MB_received = (uint16)g_Data[g_Data_BCM_INDEX] ;
			        data_size_received = data_size_LB_received;
			        data_size_received |= data_size_MB_received << 8 ;                             // STORING DATA SIZE

			if  ( data_size_received > BCM_RX_buffer.data_size)
			{
				RX_state = RX_IDLE ;
				BCM_RX_callConsumer_error( BCM_SMALL_DATA_BUFFER_SIZE );                                 // ERROR
			}
			break;
			}

			RX_state = RECEIVE_BYTE_COMPLETE ;
		}

		else if ( ( RX_buffer_Ptr > ( data_size_received + 2 ) ) &&  ( data_flag == BCM_ENABLED ) )
		{
			BCM_RX_storingChecksum();                                                        // STORING CHECK SUM
			RX_state = RECEIVE_BYTE_COMPLETE ;
		}
		else if ( (  RX_buffer_Ptr > 2  )  &&  ( data_flag == BCM_ENABLED ) )
		{
			                                                           // call function save data in data Ptr in buffer
			BCM_RX_storingData();                                               // STORING DATA

			RX_state = RECEIVE_BYTE_COMPLETE ;
		}
		else                                                                // this DATA DOES NOT BELONG TO US
		{
			data_flag = BCM_DISABLED ;
			RX_state = RX_IDLE ;
		}
		break;

	case RECEIVE_BYTE_COMPLETE:
		                             // check for data received length --> RX_state = RECEIVING_BYTE ; OR RX_state = RX_FRAME_COMPLETE:

		if ( ( RX_buffer_Ptr <= (data_size_received + 2 ) ) || ( RX_buffer_Ptr < 3 ) )
		{
			RX_state = RX_IDLE ;
		}
		else if ( ( RX_buffer_Ptr > 2 ) && ( RX_buffer_Ptr > (data_size_received + 2 )   ) )
		{
			RX_state = RX_FRAME_COMPLETE;
		}


		if ( g_Data_BCM_INDEX == SIZE_OF_INTERNAL_RX_BUFFER )
		{
			g_Data_BCM_INDEX=0;
		}
		else
		{
			g_Data_BCM_INDEX++;
		}

		RX_buffer_Ptr++;                                                // INCREMENT RX_buffer_Ptr to RECEIVE NEW BYTE

		break;

	case RX_FRAME_COMPLETE:                                            // ---> Data received successfully or Error happened
		RX_buffer_state = BCM_LOCKED ;
		RX_state = RX_IDLE ;
		data_flag = BCM_DISABLED;
		if ( BCM_RX_buffer.checksum == BCM_RX_checksum )
		{
			BCM_RX_callConsumer();                                        // check your data and unlock my buffer
		}
		else
		{
			BCM_RX_callConsumer_error( BCM_FRAMING_ERROR );                   // ERROR
		}

		break;

	case RX_IDLE :                                                           // DO NOTHING

		break;
	}

	return retval;
}

void BCM_RX_callConsumer(void)
{
	if (g_RX_consumer != NULL)
	{
		g_RX_consumer();
	}
}

void BCM_RX_callConsumer_error(EnmBCMError_t ERROR)
{
	if (g_RX_consumer_error != NULL)
	{
		g_RX_consumer_error(ERROR);
	}
}

void BCM_RX_storingData(void)                                          //  RECEIVING_BYTE ---> STORING DATA
{
	*(BCM_RX_buffer.data_address) = g_Data[g_Data_BCM_INDEX] ;              // STORING DATA in user buffer
	BCM_RX_buffer.checksum += g_Data[g_Data_BCM_INDEX] ;
	BCM_RX_buffer.data_address += 1 ;
}

void BCM_RX_storingChecksum(void)                                      // STORING checksum
{
	BCM_RX_checksum = g_Data[g_Data_BCM_INDEX] ;
}

