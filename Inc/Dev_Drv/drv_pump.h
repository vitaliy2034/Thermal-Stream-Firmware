#ifndef DRV_PUMP_H__
#define DRV_PUMP_H__

#include "main.h"

//TODO: FIx tecnical debt 

//Initialize temperature sensor driver
ReturnCode drv_pump_init();

//Start pump function
ReturnCode drv_pump_start();

//Set peltie pwm duty 
ReturnCode drv_pump_stop();

#endif //DRV_PUMP_H__
