/*
 * ws2813_led.c
 *
 *  Created on: Feb 18, 2020
 *      Author: waseemh
 */
#include "ws2813_led.h"
#include "delay.h"
	/*
	 *
	 * private function
	 *
	 */

volatile bool		g_pixel=false;
volatile bool		r_pixel=true;
volatile bool		b_pixel=true;
volatile int		rgb_mode=0;


void precise_delay(int count){
	int loop_var=0;
	for(loop_var=0;loop_var<count;loop_var++){
		__NOP();
		__NOP();
		__NOP();
		__NOP();
		__NOP();
	}		//252nsec per 5 nops
}
	/*
	 *
	 * public functions
	 *
	 */
uint32_t convert_colour_to_pixel(bool g, bool r, bool b, uint8_t intensity){
	uint32_t temp=0;
	if(g){
		temp|=(intensity<<16);
	}
	if(r){
		temp|=(intensity<<8);
	}
	if(b){
		temp|=(intensity<<0);
	}
	return temp;
}
uint32_t convert_inten_pixel(uint8_t g_inten,uint8_t r_inten,uint8_t b_inten){
	uint32_t temp=0;

	temp|=(g_inten<<16) | (r_inten<<8) | (b_inten<<0);

	return temp;
}
void decode_rgb_mode(void){

	switch(rgb_mode){
	case 0:
		r_pixel=true;g_pixel=false;b_pixel=false;
		break;
	case 1:
		r_pixel=false;g_pixel=true;b_pixel=false;
		break;
	case 2:
		r_pixel=false;g_pixel=false;b_pixel=true;
			break;
	case 3:
		r_pixel=true;g_pixel=true;b_pixel=false;
			break;
	case 4:
		r_pixel=true;g_pixel=false;b_pixel=true;
			break;
	case 5:
		r_pixel=false;g_pixel=true;b_pixel=true;
			break;
	case 6:
		r_pixel=true;g_pixel=true;b_pixel=true;
			break;
	default:
		r_pixel=true;g_pixel=false;b_pixel=true;
		break;
	}

}

void chalo_batti(uint32_t intensity){
	int 		outer_loop_var=0,inner_loop_var=0;
	uint32_t	extracted_bit=0;

			//reset signal
	GPIO->P[PWM_PORT].DOUTSET |= (1 << PWM_PIN);
	delay_5_33us(5);
	GPIO->P[PWM_PORT].DOUTCLR |= (1 << PWM_PIN);
	delay_5_33us(188);
	GPIO->P[PWM_PORT].DOUTSET |= (1 << PWM_PIN);
			//Alhumdulilah led control
	for(outer_loop_var=0;outer_loop_var<LED_COUNT;outer_loop_var++){
		for(inner_loop_var=0;inner_loop_var<PIXEL_SIZE;inner_loop_var++){
			extracted_bit=(uint32_t)(0x00800000 & intensity<<inner_loop_var);
			GPIO->P[PWM_PORT].DOUTSET |= (1 << PWM_PIN);
			if(extracted_bit){
				precise_delay(4);
				GPIO->P[PWM_PORT].DOUTCLR |= (1 << PWM_PIN);
				precise_delay(1);
			}else{
				precise_delay(1);
				GPIO->P[PWM_PORT].DOUTCLR |= (1 << PWM_PIN);
				precise_delay(4);
			}

		}

	}

}

