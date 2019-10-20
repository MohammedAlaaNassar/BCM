/*
 * UART_cnfg.h
 *
 *  Created on: Oct 12, 2019
 *      Author: Mohammed
 */

#ifndef UART_CFG_H_
#define UART_CFG_H_

#include "std_types.h"

/* USART MODE */
#define ASYNCHRONOUS  1
#define SYNCHRONOUS   0

/* COMMUNICATION MODEL */
#define P2P           2
#define MASTER        1
#define SLAVE         0

/*  DATA SIZE  */
#define NINE_BITS     4
#define EIGHT_BITS    3
#define SEVEN_BITS    2
#define SIX_BITS      1
#define FIVE_BITS     0

/* PARITY */
#define EVEN          2
#define ODD           1
#define DISABLED      0

/*  STOP PATTERN  */
#define TWO_BITS      1
#define BIT           0

/* TRANSMISSION SPEED */
#define DOUBLE_SPEED  1
#define NORMAL_SPEED  0

/*  BAUD RATE  */
#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 8UL))) - 1)

/* INTERRUPT */
#define INTERRUPT     1
#define POOLING       0

/*  CLK EDGE  */
#define FALLING       1
#define RISING        0


#endif /* UART_CFG_H_ */
