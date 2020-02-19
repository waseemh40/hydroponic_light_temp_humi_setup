/*
 * pid_controller.h
 *
 *  Created on: Jun 17, 2018
 *      Author: waseemh
 */

#ifndef DRIVERS_PID_CONTROLLER_H_
#define DRIVERS_PID_CONTROLLER_H_

#include "pinmap.h"


int 	pid_control(uint16_t set_point,uint16_t feedback);
void 		pid_change_gains(uint8_t kp,uint8_t ki);

#endif /* DRIVERS_PID_CONTROLLER_H_ */
