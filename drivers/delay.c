/*
 * delay.c
 *
 *  Created on: Apr 7, 2017
 *      Author: waseemh
 */


#include "delay.h"

/*
 * external variables
 */
extern bool		sampler_timer_flag;
extern uint32_t	rgb_intensity;
extern bool		pwm_done_flag;

/*
 * public variables
 */
static		bool		pwm_cycle_complete_flag=false;


/*
 * IRQs
 */
		//sampler IRQ handler
void TIMER1_IRQHandler(void)
{
	uint32_t	int_mask=TIMER_IntGet(SAMPLER_TIMER);
	if(int_mask & TIMER_IF_OF){
		sampler_timer_flag=true;
	}
	TIMER_IntClear(SAMPLER_TIMER, int_mask);
	return;
}

void 		pwm_enable(){
	TIMER_Enable(PWM_TIMER,true);
	return;
}

void 		pwm_disable(){
	TIMER_Enable(PWM_TIMER,false);
	return;
}
		//PWM IRQ handler
static 		uint16_t	pwm_cycle_count=0;
static 		uint32_t	pwm_instances_counter=0;


void TIMER0_IRQHandler(void)
{
	uint32_t	int_mask=PWM_TIMER->IF;
	if(int_mask & TIMER_IF_OF){
		pwm_instances_counter++;
		if(pwm_instances_counter>=pwm_cycle_count){
			PWM_TIMER->CMD = TIMER_CMD_STOP;
		}
	}
	PWM_TIMER->IFC=int_mask;
	return;
}
/*
 * private functions
 */

void 		sampler_timer_start(){
	TIMER_Enable(SAMPLER_TIMER,true);
	return;
}

void 		sampler_timer_stop(){
	TIMER_Enable(SAMPLER_TIMER,false);
	return;
}

/*
 * public functions
 */
void 		pwm_init(void){

	CMU_ClockEnable(PWM_CLK, true);

  			//init timer
	TIMER_Init_TypeDef PWMTimerInit = TIMER_INIT_DEFAULT;
  	PWMTimerInit.enable=false;
  	PWMTimerInit.prescale=timerPrescale1;
  	/**********************************
			 1cc=1/12usec
			=> 15cc = 1.25usec
			=> 12cc = 1usec
			=> 11cc = 0.91usec
			=> 6cc  = 0.5usec
			=> 4cc  = 0.3333usec
			=> 3cc  = 0.25usec
			=> 1cc  = 0.083usec
  	*********************************/
  			//set cc mode to PWM
  	TIMER_InitCC_TypeDef PWMTimerCCInit = TIMER_INITCC_DEFAULT;
  	PWMTimerCCInit.cofoa=timerOutputActionToggle ;	//should be ignored
  	PWMTimerCCInit.mode=timerCCModePWM;

	TIMER_InitCC(PWM_TIMER, 0, &PWMTimerCCInit);
	PWM_TIMER->ROUTE |= (TIMER_ROUTE_CC0PEN | TIMER_ROUTE_LOCATION_LOC3); 					//LOC 3, CC0 (PD1)
	TIMER_TopSet(PWM_TIMER, PWM_TOP);
	TIMER_CounterSet(PWM_TIMER, 0);
	TIMER_IntEnable(PWM_TIMER, TIMER_IEN_OF);		//enable OF INT
	NVIC_ClearPendingIRQ(PWM_IRQ);
	NVIC_EnableIRQ(PWM_IRQ);						//enable interrupt vector in NVIC
	TIMER_Init(PWM_TIMER, &PWMTimerInit);

	return;

}

void 		pwm_generate(uint32_t cycle_count,uint32_t duty){

	pwm_cycle_complete_flag=false;
	pwm_cycle_count=cycle_count;

	pwm_instances_counter=0;
	PWM_TIMER->CNT=0;
	PWM_TIMER->CC[0].CCV=duty;
	PWM_TIMER->CMD = TIMER_CMD_START;
	return;
}

void 		pwm_stop(void){
	pwm_disable();
	return;
}



	//control loop sample time timer
void 		sampler_timer_init(void){

	CMU_ClockEnable(SAMPLER_TIMER_CLK, true);
	TIMER_Init_TypeDef SAMPLERTimerInit = TIMER_INIT_DEFAULT;
	SAMPLERTimerInit.enable=false;
	SAMPLERTimerInit.prescale=timerPrescale1024;		//1cc=85.33 usec
	TIMER_TopSet(SAMPLER_TIMER, SAMPLER_TIMER_TOP);
	TIMER_CounterSet(SAMPLER_TIMER, 0);
	TIMER_IntEnable(SAMPLER_TIMER, TIMER_IF_OF);		//enable OF INT
	NVIC_ClearPendingIRQ(SAMPLER_IRQ);
	NVIC_EnableIRQ(SAMPLER_IRQ);						//enable interrupt vector in NVIC
	TIMER_Init(SAMPLER_TIMER, &SAMPLERTimerInit);
	TIMER_Enable(SAMPLER_TIMER,true);					//start TIMER

	return;
}
		//delay timer

void delay_init(void){

	CMU_ClockEnable(DELAY_TIMER_CLK, true);

  	TIMER_Init_TypeDef timerDELAYInit = TIMER_INIT_DEFAULT;
  	timerDELAYInit.enable=false;
  	timerDELAYInit.prescale=timerPrescale16;		//1cc=1.33 usec
	TIMER_TopSet(DELAY_TIMER, 0);
	TIMER_CounterSet(DELAY_TIMER, 0);
	TIMER_IntEnable(DELAY_TIMER, TIMER_IF_OF);		//enable OF INT
	NVIC_ClearPendingIRQ(DELAY_TIMER_IRQ);
	NVIC_EnableIRQ(DELAY_TIMER_IRQ);						//enable interrupt vector in NVIC
	TIMER_Init(DELAY_TIMER, &timerDELAYInit);
	return;

}

void delay_enable(){
	TIMER_Enable(DELAY_TIMER,true);
	return;
}
void delay_disable(){
	TIMER_Enable(DELAY_TIMER,false);
	return;
}

void delay_5_33us(uint32_t usec){
	//if(usec<=5){usec=5;}			//min is 5 cc meaning 5*1.33 or 6.5 usec approx.
	DELAY_TIMER->TOP=(usec);
	DELAY_TIMER->CNT=0;
	DELAY_TIMER->CMD = TIMER_CMD_START;
	while(DELAY_TIMER->CNT<DELAY_TIMER->TOP);
	DELAY_TIMER->CMD = TIMER_CMD_STOP;
	return;
}


void nop_delay(int count){	//250nsec per 5 nops
	int loop_var=0;
	for(loop_var=0;loop_var<count;loop_var++){
		__NOP();
	}
}
