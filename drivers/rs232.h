/*
 * rs232.h
 *
 *  Created on: Jun 4, 2018
 *      Author: waseemh
 */

#ifndef DRIVERS_RS232_H_
#define DRIVERS_RS232_H_


#include "pinmap.h"
#include "em_usart.h"
#include "fifo_rs232.h"

#define 	RS232_BAUDRATE 		9600
#define 	RS232_USART			USART1
#define 	RS232_USART_CLK		cmuClock_USART1
#define 	RS232_RX_IRQn		USART1_RX_IRQn
#define 	RS232_TX_IRQn		USART1_TX_IRQn
#define 	RS232_TX_ISR		USART1_TX_IRQHandler
#define 	RS232_RX_ISR		USART1_RX_IRQHandler

void 	rs232_init(void);
void 	rs232_enable(void);
void 	rs232_disable(void);
int 	rs232_transmit_string(const unsigned char* data,uint8_t length);
int 	rs232_transmit_char(unsigned char data );
char 	rs232_receive( void );			//not required in current scenario;implemented as blocking, change to INT if required
void 	rs232_reset(void);
void 	rs232_shutdown(void);
void 	debug_str(char *buf);

#endif /* DRIVERS_RS232_H_ */
