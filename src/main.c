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
void 	menu_system(void);


/*
 * public variables
 */
volatile 	bool		sampler_timer_flag=false;
volatile 	uint32_t	rgb_pixel=0x00FFFFFF;

		//menu related
typedef enum {	g_config=0,
				r_config,
				b_config,
				cycle,
				hours_offset,
				days_offset,
				temp_humi,
				display
			}menu_mode_t;
typedef enum {	hrs=0,
				min,
				temp,
				humi,
				days
			}time_data_t;
typedef enum {	temp_avg=0,
				temp_max,
				temp_min,
				humi_avg,
				humi_max,
				humi_min
			}temp_humi_disp_t;

			menu_mode_t 		menu_mode=display,last_menu_mode=display;
			uint8_t				g_inten=G_INTEN_DEF,r_inten=R_INTEN_DEF,b_inten=B_INTEN_DEF;		//default values
			bool				right_pressed=false;
			time_data_t			time_data=temp;					//display temp by default
			temp_humi_disp_t	temp_humi_disp=temp_avg;		//display avg. temp by default
				//time related
volatile	uint8_t		sample_counter=0;
volatile	uint64_t	seconds=0;
volatile	uint8_t		mins=0,hours=0;
volatile	uint8_t		days_count=0;
volatile	uint8_t		running_cycle=12;		//default value of 12 hrs

volatile	dht_data_t 	dht_data;
			bool		read_dht=false;
			uint32_t	hum_val=0;
			uint32_t	temp_val=0;

			//average, max. & min. temp. and humidity related parameters
volatile	uint32_t	temp_avg_c=0;
volatile	uint32_t	temp_min_c=0;
volatile	uint32_t	temp_max_c=0;
volatile	uint32_t	humi_avg_c=0;
volatile	uint32_t	humi_min_c=0;
volatile	uint32_t	humi_max_c=0;
volatile	uint32_t	temp_avg_d=0;
volatile	uint32_t	temp_min_d=0;
volatile	uint32_t	temp_max_d=0;
volatile	uint32_t	humi_avg_d=0;
volatile	uint32_t	humi_min_d=0;
volatile	uint32_t	humi_max_d=0;


			char 		rs232_buf[128];
			char 		lcd_buf[32];



/*
 * shared external functions
 */
void 	inc_dec_irq_handler(uint8_t irq_number){
	if(irq_number==RIGHT_SW){
		right_pressed=true;
	}
	if(irq_number==LEFT_SW){
		menu_mode++;
		if(menu_mode>display){
			menu_mode=g_config;
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
			chalo_batti( rgb_pixel);

			while (1) {
				if(sampler_timer_flag)
					{

						sampler_timer_flag=false;

						sample_counter++;
						if(sample_counter>=10){
							sample_counter=0;
							seconds++;
							if(seconds%2==0){
								read_dht=true;
							}
							if(seconds%60==0){
								mins++;
								if(mins>59){
									mins=0;
								}
							}
							if(seconds%3600==0){
								hours++;
								if(hours>23){
									hours=0;
									rgb_pixel=convert_inten_pixel(g_inten,r_inten,b_inten);
									chalo_batti( rgb_pixel);
									days_count++;
										//update temp. & humi. display values
									temp_avg_d=(uint32_t)(temp_avg_c/AVERAGE_SAMPLES);
									temp_max_d=(uint32_t)(temp_max_c);
									temp_min_d=(uint32_t)(temp_min_c);
									temp_avg_c=0;
									temp_max_c=0;
									temp_min_c=0;
									humi_avg_d=(uint32_t)(humi_avg_c/AVERAGE_SAMPLES);
									humi_max_d=(uint32_t)(humi_max_c);
									humi_min_d=(uint32_t)(humi_min_c);
									humi_avg_c=0;
									humi_max_c=0;
									humi_min_c=0;

								}
								if(hours>=running_cycle){
									rgb_pixel=convert_inten_pixel(G_INTEN_MIN,R_INTEN_MIN,B_INTEN_MIN);
									chalo_batti( rgb_pixel);
								}
							}
						}


						if(read_dht){
							dht_data.hum_dec=0;
							dht_data.hum_int=0;
							dht_data.temp_int=0;
							dht_data.temp_dec=0;
							dht_data.checksum=0;
							dht_data=dht_read_iA();
							hum_val= ((uint32_t)((dht_data.hum_int<< 8) + dht_data.hum_dec));///(float)10.0);
							temp_val= ((uint32_t)((dht_data.temp_int << 8) + dht_data.temp_dec));///(float)10.0);
							uint32_t sum=dht_data.hum_dec+dht_data.hum_int+dht_data.temp_int+dht_data.temp_dec;
							sprintf(rs232_buf,"H_I=%2d H_D=%2d T_I=%2d T_D=%2d S=%d %4x C=%2x \t hum_val=%3d \t temp_val=%3d",dht_data.hum_int,dht_data.hum_dec,dht_data.temp_int,dht_data.temp_dec,sum,sum,dht_data.checksum,hum_val,temp_val);
							debug_str(rs232_buf);
							sprintf(rs232_buf,"Pixel mode=%2x\n",menu_mode);
							debug_str(rs232_buf);
							read_dht=false;
							//avergae related
							if(seconds%1800==0){		//30 minutes sampling
									//temperature related
								temp_avg_c+=temp_val;
								if(temp_min_c>temp_val){
									temp_min_c=temp_val;
								}
								if(temp_max_c<temp_val){
									temp_max_c=temp_val;
								}
									//temperature related
								humi_avg_c+=hum_val;
								if(humi_min_c>hum_val){
									humi_min_c=hum_val;
								}
								if(humi_max_c<hum_val){
									humi_max_c=hum_val;
								}

							}
						}
							//menu related update @ 10 Hz
						if(right_pressed || last_menu_mode!=menu_mode || seconds%60==0){
							menu_system();
							last_menu_mode=menu_mode;
						}

					}

			  }

}

void menu_system(void){
	switch(menu_mode){
		case r_config:
			if(right_pressed){
				r_inten+=INTEN_STEP;
				if(r_inten>R_INTEN_MAX){
					r_inten=R_INTEN_MIN;
				}
				if(r_inten<R_INTEN_MIN){
					r_inten=R_INTEN_MAX;
				}
				right_pressed=false;
			}
			SegmentLCD_AllOff();
			sprintf((const char *)lcd_buf,"R_INTEN");
			SegmentLCD_Write((const char *)lcd_buf);
			SegmentLCD_Number(r_inten);
			break;
		case g_config:
			if(right_pressed){
				g_inten+=INTEN_STEP;
				if(g_inten>G_INTEN_MAX){
					g_inten=G_INTEN_MIN;
				}
				if(g_inten<G_INTEN_MIN){
					g_inten=G_INTEN_MAX;
				}
				right_pressed=false;
			}
			SegmentLCD_AllOff();
			sprintf((const char *)lcd_buf,"G_INTEN");
			SegmentLCD_Write((const char *)lcd_buf);
			SegmentLCD_Number(g_inten);
			break;
		case b_config:
			if(right_pressed){
				b_inten+=INTEN_STEP;
				if(b_inten>B_INTEN_MAX){
					b_inten=B_INTEN_MIN;
				}
				if(b_inten<B_INTEN_MIN){
					b_inten=B_INTEN_MAX;
				}
				right_pressed=false;
			}
			SegmentLCD_AllOff();
			sprintf((const char *)lcd_buf,"B_INTEN");
			SegmentLCD_Write((const char *)lcd_buf);
			SegmentLCD_Number(b_inten);
			break;
		case display:
			SegmentLCD_AllOff();
			if(right_pressed){
				time_data++;
				if(time_data>days){
					time_data=hrs;
				}
				right_pressed=false;
			}
			if(time_data==hrs){
				sprintf((const char *)lcd_buf,"Hours");
				SegmentLCD_Write((const char *)lcd_buf);
				SegmentLCD_Number(hours);
			}
			else if(time_data==min){
				sprintf((const char *)lcd_buf,"Mins");
				SegmentLCD_Write((const char *)lcd_buf);
				SegmentLCD_Number(mins);
			}
			else if(time_data==temp){
				sprintf((const char *)lcd_buf,"Temp.");
				SegmentLCD_Write((const char *)lcd_buf);
				SegmentLCD_Number(temp_val);
			}
			else if(time_data==humi){
				sprintf((const char *)lcd_buf,"Humi");
				SegmentLCD_Write((const char *)lcd_buf);
				SegmentLCD_Number(hum_val);
			}
			else if(time_data==days){
				sprintf((const char *)lcd_buf,"Days");
				SegmentLCD_Write((const char *)lcd_buf);
				SegmentLCD_Number(days_count);
			}
			else{
				SegmentLCD_Write((const char *)"Invalid");
				time_data=0;
			}
			break;
		case cycle:
			if(right_pressed){
				if(running_cycle==12){
					running_cycle=16;
				}
				else if(running_cycle==16){
					running_cycle=12;
				}
				else{
					;
				}
				right_pressed=false;
			}
			SegmentLCD_AllOff();
			sprintf((const char *)lcd_buf,"LED_cyc.");
			SegmentLCD_Write((const char *)lcd_buf);
			SegmentLCD_Number(running_cycle);
			break;
		case hours_offset:
			if(right_pressed){
				hours+=1;
				if(hours>23){
					hours=0;
				}
				right_pressed=false;
			}
			SegmentLCD_AllOff();
			sprintf((const char *)lcd_buf,"Hrs_off.");
			SegmentLCD_Write((const char *)lcd_buf);
			SegmentLCD_Number(hours);
			break;
		case days_offset:
			if(right_pressed){
				days_count+=1;
				right_pressed=false;
			}
			SegmentLCD_AllOff();
			sprintf((const char *)lcd_buf,"Days_off.");
			SegmentLCD_Write((const char *)lcd_buf);
			SegmentLCD_Number(days_count);
			break;
		case temp_humi:
			SegmentLCD_AllOff();
			if(right_pressed){
				temp_humi_disp++;
				if(temp_humi_disp>humi_min){
					temp_humi_disp=temp_avg;
				}
				right_pressed=false;
			}
			if(temp_humi_disp==temp_avg){
				sprintf((const char *)lcd_buf,"T_AVG");
				SegmentLCD_Write((const char *)lcd_buf);
				SegmentLCD_Number(temp_avg_d);
			}
			else if(temp_humi_disp==temp_max){
				sprintf((const char *)lcd_buf,"T_MAX");
				SegmentLCD_Write((const char *)lcd_buf);
				SegmentLCD_Number(temp_max_d);
			}
			else if(temp_humi_disp==temp_min){
				sprintf((const char *)lcd_buf,"T_MIN");
				SegmentLCD_Write((const char *)lcd_buf);
				SegmentLCD_Number(temp_min_d);
			}
			else if(temp_humi_disp==humi_avg){
				sprintf((const char *)lcd_buf,"H_AVG");
				SegmentLCD_Write((const char *)lcd_buf);
				SegmentLCD_Number(humi_avg_d);
			}
			else if(temp_humi_disp==humi_max){
				sprintf((const char *)lcd_buf,"H_MAX");
				SegmentLCD_Write((const char *)lcd_buf);
				SegmentLCD_Number(humi_max_d);
			}
			else if(temp_humi_disp==humi_min){
				sprintf((const char *)lcd_buf,"H_MIN");
				SegmentLCD_Write((const char *)lcd_buf);
				SegmentLCD_Number(humi_min_d);
			}
			else{
				SegmentLCD_Write((const char *)"Invalid");
				temp_humi_disp=0;
			}
			break;
		default:
			menu_mode=display;
			break;

	}
}


