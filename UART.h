/*
 * UART.h
 *
 *  Created on: Oct 11, 2019
 *      Author: Mohammed
 */

#ifndef UART_H_
#define UART_H_


#include "UART_cfg.h"

typedef struct
{
	uint8 USART_mode;
	uint8 communication_model;
	uint8 data_size;
	uint8 parity;
	uint8 stop_pattern;
	uint8 tarnsmission_speed;
	uint8 interrupt;
	uint8 clk_edge;
	uint32 baud_rate;
} UART_cfg_t ;

typedef const UART_cfg_t* UART_cfgPtr_t;

typedef uint8 UART_error_t;
#define UART_OK            0
#define UART_INVALID_CFG   1
#define UART_PARITY_ERROR  2
#define UART_FRAMING_ERROR 3

extern UART_error_t UART_init(UART_cfgPtr_t);
extern UART_error_t UART_sendByte(const uint8);
extern UART_error_t UART_receiveByte(uint8 *);

extern UART_error_t UART_TX_set_callback(void_func_ptr app_ptr);
extern UART_error_t UART_RX_set_callback(uint8_func_ptr app_ptr );


#endif /* UART_H_ */
