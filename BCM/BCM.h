/*
 * BCM.h
 *
 *  Created on: Oct 20, 2019
 *      Author: Mohammed Nassar
 */

#ifndef BCM_H_
#define BCM_H_



#include "micro_config.h"
#include "std_types.h"
#include "common_macros.h"

typedef enum{
	BCM_ENABLED,
	BCM_DISABLED
	}BCM_Enable_t;

typedef struct //edit the elements
{
	uint8 BCM_ID;
	BCM_Enable_t UART;
	BCM_Enable_t SPI;
	BCM_Enable_t I2C;
} BCM_cfg_t ;

#include "BCM_Pbcfg.h"

typedef const BCM_cfg_t* BCM_cfgPtr_t;


typedef enum{
	BCM_OK,
	BCM_INVALID_CFG,
	BCM_UART_ERROR,
	BCM_INVALID_DATA,
	BCM_MODULE_BUSY,
	BCM_WRONG_ID,
	BCM_SMALL_DATA_BUFFER_SIZE,
	BCM_FRAMING_ERROR,
}EnmBCMError_t;

typedef void (*EnmBCMError_func_ptr)(EnmBCMError_t);

extern EnmBCMError_t BCM_Init (BCM_cfgPtr_t);
extern EnmBCMError_t BCM_DeInit (void);

extern EnmBCMError_t BCM_RxDispatch(void);
extern EnmBCMError_t BCM_TxDispatch(void);


extern EnmBCMError_t BCM_Send(uint8* data_address, uint16 data_size, void_func_ptr TX_consumer);

extern EnmBCMError_t BCM_RX_Setup(uint8* data_address,uint16 data_size, void_func_ptr RX_consumer, EnmBCMError_func_ptr RX_consumer_error );
extern EnmBCMError_t BCM_RX_Unlock(void);

#endif /* BCM_H_ */
