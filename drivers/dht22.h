/*
 * dht22.h
 *
 *  Created on: Feb 18, 2020
 *      Author: waseemh
 */

#ifndef DRIVERS_DHT22_H_
#define DRIVERS_DHT22_H_

#include "pinmap.h"

typedef struct {
	uint8_t  hum_int;
	uint8_t  hum_dec;
	uint8_t  temp_int;
	uint8_t  temp_dec;
	uint8_t	 checksum;
}dht_data_t;

dht_data_t 	dht_read_iA(void);
bool 		dht_check_status(void);


#endif /* DRIVERS_DHT22_H_ */
