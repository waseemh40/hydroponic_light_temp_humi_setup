/*
 * delay.h
 *
 *  Created on: Apr 7, 2017
 *      Author: waseemh
 */

#ifndef DRIVERS_DELAY_H_
#define DRIVERS_DELAY_H_

#include "pinmap.h"
#include "em_timer.h"
#include "em_burtc.h"
#include "em_letimer.h"

	//PWM timer
#define PWM_TIMER 			TIMER0
#define PWM_CLK				cmuClock_TIMER0
#define	PWM_TOP				15				//1.25 usec
#define	PWM_IRQ				TIMER0_IRQn

	//sampler pulse
#define SAMPLER_TIMER		TIMER1
#define SAMPLER_TIMER_CLK	cmuClock_TIMER1
#define	SAMPLER_TIMER_TOP	1172			//(100 msec ==> 1172 cc)
#define	SAMPLER_IRQ			TIMER1_IRQn

	//delay loop timer
#define DELAY_TIMER			TIMER3
#define DELAY_TIMER_CLK		cmuClock_TIMER3
#define DELAY_TIMER_IRQ		TIMER3_IRQn

#define DUTY_100			15
#define DUTY_0				0
#define N_LED				30
#define PWM_CYCLES			(24*N_LED)	//40 for 50 usec delay, and N for LEDS
#define T1H					11
#define T0H					4

#define DELAY_26			19
#define DELAY_80			60
#define DELAY_50			37
#define DELAY_500			370
#define DELAY_40			30
#define DELAY_43			32

/*
 * public variables
 */
/*
 * private functions
 */
/*
 * public functions
 */
void 		pwm_init(void);
void 		pwm_generate(uint32_t cycle_count,uint32_t duty);
void 		pwm_stop(void);


void		delay_init(void);
void 		delay_5_33us(uint32_t usec);

void 		sampler_timer_init(void);
void 		sampler_timer_start(void);
void 		sampler_timer_stop(void);


void 		nop_delay(int count);

#endif /* SRC_DELAY_H_ */
