/*
 * dht22.c
 *
 *  Created on: Feb 18, 2020
 *      Author: waseemh
 */
#include "dht22.h"
#include "delay.h"

char rs232_buf[128];


dht_data_t dht_read_iA(void){
	int loop_var=0;
	volatile uint64_t	overall_data=0x0000000000000000;
	volatile uint16_t	counter=0;
	volatile uint8_t	data_array[5]={0,0,0,0,0};
	volatile uint16_t	ref_counter=0;
			 dht_data_t dht_data;

	overall_data=0;

	data_array[0]=0;
	data_array[1]=0;
	data_array[2]=0;
	data_array[3]=0;
	data_array[4]=0;

	if(dht_check_status()){
		while(!(GPIO->P[DHT_PORT].DIN & (1<<DHT_PIN))){
		}
		counter=0;
		while(GPIO->P[DHT_PORT].DIN & (1<<DHT_PIN)){
		}

		DELAY_TIMER->TOP=(0xFFFF);
		for(loop_var=0;loop_var<40;loop_var++){
			DELAY_TIMER->CNT=0;
			DELAY_TIMER->CMD = TIMER_CMD_START;
			while(!(GPIO->P[DHT_PORT].DIN & (1<<DHT_PIN))){	//50 usec time pass
			}
			DELAY_TIMER->CMD = TIMER_CMD_STOP;
			ref_counter=DELAY_TIMER->CNT;

			DELAY_TIMER->CNT=0;
			DELAY_TIMER->CMD = TIMER_CMD_START;
			while((GPIO->P[DHT_PORT].DIN & (1<<DHT_PIN)));
			DELAY_TIMER->CMD = TIMER_CMD_STOP;
			counter=DELAY_TIMER->CNT;
			if(counter<=ref_counter){
				;
			}
			else{
				overall_data|=1<<(39-loop_var);
				data_array[(uint8_t)loop_var/8]|=1<<(7-(loop_var%8));
			}
		}

		dht_data.hum_int=data_array[0];//(uint8_t)(((overall_data & 0x000000FF00000000) >> 32));
		dht_data.hum_dec=data_array[1];//(((overall_data & 0x00000000FF000000) >> 24));
		dht_data.temp_int=data_array[2];//(uint8_t)(((overall_data & 0x0000000000FF0000) >> 16));
		dht_data.temp_dec=data_array[3];//(uint8_t)(((overall_data & 0x000000000000FF00) >> 8));
		dht_data.checksum=data_array[4];//(uint8_t)(((overall_data & 0x00000000000000FF) >> 0));

	}
	else{
		dht_data.hum_dec=0;
		dht_data.hum_int=0;
		dht_data.temp_int=0;
		dht_data.temp_dec=0;
		dht_data.checksum=0;
		debug_str((char *)"Sensor here: did not init.\n");
	}

	return dht_data;
}

bool dht_check_status(void){
	uint8_t	counter=0;		//1 usec delay is 1.55, minimum is 5 i.e. 6.5 usec approx.
	bool	sensor_status=true;

	GPIO_PinModeSet(DHT_PORT, DHT_PIN, gpioModePushPull, 1);
	GPIO->P[DHT_PORT].DOUTCLR |= (1 << DHT_PIN);
	delay_5_33us(DELAY_500);
	delay_5_33us(DELAY_500);
	GPIO->P[DHT_PORT].DOUTSET |= (1 << DHT_PIN);
	delay_5_33us(DELAY_40);
	delay_5_33us(5);
	GPIO_PinModeSet(DHT_PORT, DHT_PIN, gpioModeInput, 1);

	return sensor_status;

}
