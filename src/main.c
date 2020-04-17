#include "../drivers/delay.h"
#include "../drivers/gpio.h"
#include "../drivers/rs232.h"
#include "../drivers/pid_controller.h"
#include "../drivers/ws2813_led.h"
#include "../drivers/dht22.h"
#include <string.h>
#include "segmentlcd.h"

#define 	AVERAGE_SAMPLES 48		//per 30 minutes


void 	inc_dec_irq_handler(uint8_t irq_number);
void 	menu_display(void);


/*
 * public variables
 */
volatile 	bool		sampler_timer_flag=false;
volatile 	uint32_t	rgb_pixel=0x00FFFFFF;

		//menu related
typedef enum {	d_time=0,
				d_temp,
				d_humi,
				adj_r,
				adj_g,
				adj_b,
				adj_hh,
				adj_mm,
				adj_cycle,
				adj_led_start_time,
				adj_days
			}menu_t;
typedef enum {	sm_time=0,
				sm_led_start_time,
				sm_days
			}submenu_time_t;
typedef enum {	sm_current=0,
				sm_min,
				sm_max,
				sm_avg
				}submenu_temp_humi_t;

			menu_t 				menu_mode=d_time;
			uint8_t				g_inten=G_INTEN_DEF,r_inten=R_INTEN_DEF,b_inten=B_INTEN_DEF;		//default values
			bool				right_pressed=false;
			bool				left_pressed=false;
			submenu_time_t		submenu_time=sm_time;
			submenu_temp_humi_t	submenu_temp_humi=sm_current;

				//time related
volatile	uint8_t		sample_counter=0;
volatile	uint64_t	seconds=0;
volatile	uint8_t		mins=0,hours=0;
volatile	uint8_t		days_count=0;
volatile	uint8_t		running_cycle=14;		//default value of 14 hrs
volatile	uint8_t		led_start_time=7;		//7 am is start time
volatile	uint64_t	time_formatted=0;

volatile	dht_data_t 	dht_data;
			bool		read_dht=false;
			uint64_t	humi_current=0;
			uint64_t	temp_current=0;

			//average, max. & min. temp. and humidity related parameters
volatile	uint64_t	temp_avg_c=0;
volatile	uint64_t	temp_min_c=500;
volatile	uint64_t	temp_max_c=0;
volatile	uint64_t	humi_avg_c=0;
volatile	uint64_t	humi_min_c=500;
volatile	uint64_t	humi_max_c=0;
volatile	uint64_t	temp_avg_d=0;
volatile	uint64_t	temp_min_d=0;
volatile	uint64_t	temp_max_d=0;
volatile	uint64_t	humi_avg_d=0;
volatile	uint64_t	humi_min_d=0;
volatile	uint64_t	humi_max_d=0;


			char 		rs232_buf[128];
			char 		lcd_buf[32];



/*
 * shared external functions
 */
void 	inc_dec_irq_handler(uint8_t irq_number){
	if(irq_number==RIGHT_SW){
		right_pressed=true;
		switch(menu_mode){
		case d_time:
			if(submenu_time==sm_time){
				submenu_time=sm_led_start_time;
			}
			else if(submenu_time==sm_led_start_time){
				submenu_time=sm_days;
			}
			else{
				submenu_time=sm_time;
			}
			break;
		case d_temp: case d_humi:
			if(submenu_temp_humi==sm_current){
				submenu_temp_humi=sm_min;
			}
			else if(submenu_temp_humi==sm_min){
				submenu_temp_humi=sm_max;
			}
			else if(submenu_temp_humi==sm_max){
				submenu_temp_humi=sm_avg;
			}
			else{
				submenu_temp_humi=sm_current;
			}
			break;
		case adj_r:
			r_inten+=INTEN_STEP;
			if(r_inten>R_INTEN_MAX){
				r_inten=R_INTEN_MIN;
			}
			if(r_inten<R_INTEN_MIN){
				r_inten=R_INTEN_MAX;
			}
			break;
		case adj_g:
			g_inten+=INTEN_STEP;
			if(g_inten>G_INTEN_MAX){
				g_inten=G_INTEN_MIN;
			}
			if(g_inten<G_INTEN_MIN){
				g_inten=G_INTEN_MAX;
			}
			break;
		case adj_b:
			b_inten+=INTEN_STEP;
			if(b_inten>B_INTEN_MAX){
				b_inten=B_INTEN_MIN;
			}
			if(b_inten<B_INTEN_MIN){
				b_inten=B_INTEN_MAX;
			}
			break;
		case adj_hh:
			hours+=1;
			if(hours>23){
				hours=0;
			}
			break;
		case adj_mm:
			mins+=2;
			if(mins>59){
				mins=0;
			}
			break;
		case adj_cycle:
			if(running_cycle==12){
				running_cycle=14;
			}
			else{
				running_cycle=12;
			}
			break;
		case adj_led_start_time:
			if(led_start_time==7){
				led_start_time=8;
			}
			else{
				led_start_time=7;
			}
			break;
		case adj_days:
			days_count+=1;
			break;
		default:
			;
			break;
		}
	}
	if(irq_number==LEFT_SW){
		left_pressed=true;
		menu_mode++;
		if(menu_mode>adj_days){
			menu_mode=d_time;
		}
	}
	return;
}


int main(void)
{
	 /*
	  ********************* Chip initialization*************
	  */
			CHIP_Init();
			CMU_OscillatorEnable(cmuOsc_HFXO, true, true);
			CMU_ClockSelectSet (cmuClock_HF, cmuSelect_HFXO);
			CMU_ClockDivSet (cmuClock_CORE,cmuClkDiv_1);
			CMU_ClockDivSet (cmuClock_HFPER ,cmuClkDiv_4);

	 /*
	  *******************************************************
	  */
			gpio_init();
			delay_init();
			sampler_timer_init();
			rs232_init();
			rs232_enable();



			SegmentLCD_Init(false);

			GPIO_PinOutClear(OUT_PORT, LED);
			GPIO_PinOutClear(OUT_PORT, LED_2);
			SegmentLCD_AllOn();

			SegmentLCD_AllOff();
			GPIO_PinOutSet(OUT_PORT, LED);
			GPIO_PinOutSet(OUT_PORT, LED_2);

			GPIO_PinOutClear(OUT_PORT, LED_2);
			GPIO_PinOutSet(OUT_PORT, LED);
			SegmentLCD_Write((const char *)"HELLO");
			sprintf(rs232_buf,"iA will work\n");
			debug_str(rs232_buf);

			rgb_pixel=convert_inten_pixel(g_inten,r_inten,b_inten);
			chalo_batti(rgb_pixel);

			while (1) {
				if(sampler_timer_flag)
					{

						sampler_timer_flag=false;

						sample_counter++;
						if(sample_counter>=10){
							sample_counter=0;
							seconds++;
							if(seconds>59){
								seconds=0;
								mins++;
								if(mins>59){
									mins=0;
									hours++;
									if(hours>23){
										hours=0;
										days_count++;
											//update temp. & humi. display values
										temp_avg_d=(uint64_t)(temp_avg_c/AVERAGE_SAMPLES);
										temp_max_d=(uint64_t)(temp_max_c);
										temp_min_d=(uint64_t)(temp_min_c);
										temp_avg_c=0;
										temp_max_c=0;
										temp_min_c=500;
										humi_avg_d=(uint64_t)(humi_avg_c/AVERAGE_SAMPLES);
										humi_max_d=(uint64_t)(humi_max_c);
										humi_min_d=(uint64_t)(humi_min_c);
										humi_avg_c=0;
										humi_max_c=0;
										humi_min_c=500;

									}
								}
										// temperature and humidity averaging
								if(mins==30){				//30 minutes sampling
									temp_avg_c+=temp_current;
									humi_avg_c+=humi_current;

								}
										//update the led on next minute after start/time adjustment...
								if(hours < led_start_time || hours >= (led_start_time+running_cycle)){
									rgb_pixel=convert_inten_pixel(G_INTEN_MIN,R_INTEN_MIN,B_INTEN_MIN);
									chalo_batti(rgb_pixel);
								}
								else{
									rgb_pixel=convert_inten_pixel(g_inten,r_inten,b_inten);
									chalo_batti(rgb_pixel);
								}
								time_formatted=(hours*100)+mins;
							}
							if(seconds%2==0){
								read_dht=true;
							}
						}

						if(read_dht){
							dht_data.hum_dec=0;
							dht_data.hum_int=0;
							dht_data.temp_int=0;
							dht_data.temp_dec=0;
							dht_data.checksum=0;
							dht_data=dht_read_iA();
							humi_current= ((uint32_t)((dht_data.hum_int<< 8) + dht_data.hum_dec));///(float)10.0);
							temp_current= ((uint32_t)((dht_data.temp_int << 8) + dht_data.temp_dec));///(float)10.0);
							/*uint64_t sum=dht_data.hum_dec+dht_data.hum_int+dht_data.temp_int+dht_data.temp_dec;
							sprintf(rs232_buf,"H_I=%2d H_D=%2d T_I=%2d T_D=%2d S=%d %4x C=%2x \t hum_val=%3d \t temp_val=%3d",dht_data.hum_int,dht_data.hum_dec,dht_data.temp_int,dht_data.temp_dec,sum,sum,dht_data.checksum,humi_current,temp_current);
							debug_str(rs232_buf);
							sprintf(rs232_buf,"Pixel mode=%2x\n",menu_mode);
							debug_str(rs232_buf);*/
							read_dht=false;
								//max. and min. values
							if(humi_min_c>humi_current){
								humi_min_c=humi_current;
							}
							if(humi_max_c<humi_current){
								humi_max_c=humi_current;
							}
							if(temp_min_c>temp_current){
								temp_min_c=temp_current;
							}
							if(temp_max_c<temp_current){
								temp_max_c=temp_current;
							}
						}
							//menu related update
						if(right_pressed || left_pressed || seconds%60==0){
							menu_display();
							right_pressed=false;left_pressed=false;

						}

					}

			  }

}

void menu_display(void){
	switch(menu_mode){
		case adj_r:
			SegmentLCD_AllOff();
			sprintf((const char *)lcd_buf,"R_INTEN");
			SegmentLCD_Write((const char *)lcd_buf);
			SegmentLCD_Number(r_inten);
			break;
		case adj_g:
			SegmentLCD_AllOff();
			sprintf((const char *)lcd_buf,"G_INTEN");
			SegmentLCD_Write((const char *)lcd_buf);
			SegmentLCD_Number(g_inten);
			break;
		case adj_b:
			SegmentLCD_AllOff();
			sprintf((const char *)lcd_buf,"B_INTEN");
			SegmentLCD_Write((const char *)lcd_buf);
			SegmentLCD_Number(b_inten);
			break;
		case adj_cycle:
			SegmentLCD_AllOff();
			sprintf((const char *)lcd_buf,"LED_cyc.");
			SegmentLCD_Write((const char *)lcd_buf);
			SegmentLCD_Number(running_cycle);
			break;
		case adj_hh:
			SegmentLCD_AllOff();
			sprintf((const char *)lcd_buf,"Hrs_off.");
			SegmentLCD_Write((const char *)lcd_buf);
			SegmentLCD_Number(hours);
			break;
		case adj_mm:
			SegmentLCD_AllOff();
			sprintf((const char *)lcd_buf,"Mins_off.");
			SegmentLCD_Write((const char *)lcd_buf);
			SegmentLCD_Number(mins);
			break;
		case adj_led_start_time:
			SegmentLCD_AllOff();
			SegmentLCD_AllOff();
			sprintf((const char *)lcd_buf,"Start_off.");
			SegmentLCD_Write((const char *)lcd_buf);
			SegmentLCD_Number(led_start_time);
			break;
		case adj_days:
			SegmentLCD_AllOff();
			SegmentLCD_AllOff();
			sprintf((const char *)lcd_buf,"Day_off.");
			SegmentLCD_Write((const char *)lcd_buf);
			SegmentLCD_Number(days_count);
			break;
		case d_time:
			SegmentLCD_AllOff();
			switch(submenu_time){
			case sm_time:
				sprintf((const char *)lcd_buf,"Time");
				SegmentLCD_Write((const char *)lcd_buf);
				SegmentLCD_Number(time_formatted);
				break;
			case sm_led_start_time:
				sprintf((const char *)lcd_buf,"Start_time");
				SegmentLCD_Write((const char *)lcd_buf);
				SegmentLCD_Number(led_start_time);
				break;
			case sm_days:
				sprintf((const char *)lcd_buf,"Days");
				SegmentLCD_Write((const char *)lcd_buf);
				SegmentLCD_Number(days_count);
				break;
			default:
				sprintf((const char *)lcd_buf,"Invalid");
				SegmentLCD_Write((const char *)lcd_buf);
				break;
			}
			break;
			case d_temp:
				SegmentLCD_AllOff();
				switch(submenu_temp_humi){
				case sm_current:
					sprintf((const char *)lcd_buf,"T_cur");
					SegmentLCD_Write((const char *)lcd_buf);
					SegmentLCD_Number(temp_current);
					break;
				case sm_min:
					sprintf((const char *)lcd_buf,"T_min");
					SegmentLCD_Write((const char *)lcd_buf);
					SegmentLCD_Number(temp_min_d);
					break;
				case sm_max:
					sprintf((const char *)lcd_buf,"T_max");
					SegmentLCD_Write((const char *)lcd_buf);
					SegmentLCD_Number(temp_max_d);
					break;
				case sm_avg:
					sprintf((const char *)lcd_buf,"T_avg");
					SegmentLCD_Write((const char *)lcd_buf);
					SegmentLCD_Number(temp_avg_d);
					break;
				default:
					sprintf((const char *)lcd_buf,"Invalid");
					SegmentLCD_Write((const char *)lcd_buf);
					break;
				}
				break;
			case d_humi:
					SegmentLCD_AllOff();
					switch(submenu_temp_humi){
					case sm_current:
						sprintf((const char *)lcd_buf,"H_cur");
						SegmentLCD_Write((const char *)lcd_buf);
						SegmentLCD_Number(humi_current);
						break;
					case sm_min:
						sprintf((const char *)lcd_buf,"H_min.");
						SegmentLCD_Write((const char *)lcd_buf);
						SegmentLCD_Number(humi_min_d);
						break;
					case sm_max:
						sprintf((const char *)lcd_buf,"H_max");
						SegmentLCD_Write((const char *)lcd_buf);
						SegmentLCD_Number(humi_max_d);
						break;
					case sm_avg:
						sprintf((const char *)lcd_buf,"H_avg");
						SegmentLCD_Write((const char *)lcd_buf);
						SegmentLCD_Number(humi_avg_d);
						break;
					default:
						sprintf((const char *)lcd_buf,"Invalid");
						SegmentLCD_Write((const char *)lcd_buf);
						break;
					}
					break;
		default:
			menu_mode=d_time;
			break;

	}
}


