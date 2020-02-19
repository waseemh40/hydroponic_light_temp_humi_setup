/*
 * pid_controller.c
 *
 *  Created on: Jun 17, 2018
 *      Author: waseemh
 */


#include "pid_controller.h"

static uint8_t 	gain_kp=1;
static uint8_t	gain_ki=1;		//one-tenth of actual value
static uint8_t 	gain_kd=1;


int 	pid_control(uint16_t set_point,uint16_t feedback){
		static		int			p_error=0;
		static 		int			i_error=0;
		static 		int			d_error=0;
		static 		int			last_error=0;
		static 		int			control_out=0;
		static 		int			shifted_out=0;
//		static 		int			dt=(1>>6);


			//update errors
		p_error=set_point-feedback;
		i_error+=p_error;
		d_error=last_error-p_error;

			//calculate control out
		control_out+= (gain_kp*p_error) + ((gain_ki*i_error)>>6)+(d_error*(32/1000)*gain_kd);		//>>6 is divide by 32msec

			//anti-wind-up control

		if(control_out>=1024){
			control_out=1024;
		}
		if(control_out<=-1024){
			control_out=-1024;
		}
		shifted_out=(int)(512+(control_out/2));
		if(shifted_out<50){
			shifted_out=100;
		}
		/*
		if(control_out<0){
			control_out=(-1*control_out);
		}
		if(control_out>=1024){
			control_out=1024;
		}
		shifted_out=control_out;
		*/
		return shifted_out;

}
void 	pid_change_gains(uint8_t kp,uint8_t ki){
	gain_kp=kp;
	gain_ki=ki;
	return;
}
