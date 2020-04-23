/*
 * ws2813_led.h
 *
 *  Created on: Feb 18, 2020
 *      Author: waseemh
 */

#ifndef DRIVERS_WS2813_LED_H_
#define DRIVERS_WS2813_LED_H_


#include "pinmap.h"


#define LED_COUNT 	30
#define LED_COUNT_2 32
#define PIXEL_SIZE 	24



void chalo_batti(uint32_t intensity);
void chalo_batti_2(uint32_t intensity);
uint32_t convert_colour_to_pixel(bool g, bool r, bool b, uint8_t intensity);
void decode_rgb_mode(void);
void precise_delay(int count);
uint32_t convert_inten_pixel(uint8_t g_inten,uint8_t r_inten,uint8_t b_inten);

#define G_INTEN_MAX		255
#define G_INTEN_MIN		0
#define G_INTEN_DEF		255
#define R_INTEN_MAX		255
#define R_INTEN_MIN		0
#define R_INTEN_DEF		255
#define B_INTEN_MAX		255
#define B_INTEN_MIN		0
#define B_INTEN_DEF		255
#define INTEN_STEP		10


#endif /* DRIVERS_WS2813_LED_H_ */
