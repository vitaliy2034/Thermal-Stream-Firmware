#ifndef DRV_PELT_H__
#define DRV_PELT_H__

#include "main.h"

//TODO: add ability to configure 

typedef enum{
    DRV_PELT_1 = 0x0,
    DRV_PELT_2 = 0x1
}PeltNum_t;

typedef enum{
    DRV_PELT_MODE_HEAT = 0x0,
    DRV_PELT_MODE_COLD = 0x1
}PeltMode_t;

//Initialize temperature sensor driver
ReturnCode drv_pelt_init();

//Set peltie working mode
ReturnCode drv_pelt_mode_set(PeltNum_t xPelt, PeltMode_t xMode);

//Set peltie pwm duty 
ReturnCode drv_pelt_pwm_duty_set(PeltNum_t xPelt, uint16_t usDuty);

#endif //DRV_PELT_H__
