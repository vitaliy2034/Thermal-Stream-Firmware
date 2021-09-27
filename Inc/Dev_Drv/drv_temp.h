#ifndef DRV_TEMP_H__
#define DRV_TEMP_H__

#include "main.h"

//TODO: add ability to configure 

typedef enum
{
    TEMP_EVT_READY = 0x00
} TempEventType_t;

typedef float temp_t;

typedef void (*TempEventHandler_t)(TempEventType_t xEventType, const void * pv_context);

//Initialize temperature sensor driver
ReturnCode drv_temp_init(TempEventHandler_t xEventHandler);

//Read temperature from interrupt
ReturnCode drv_temp_read_int(void);

//Calibrate ADC
ReturnCode drv_temp_adc_calib(void);

//TODO: Develop this feature
//ReturnCode drv_temp_calibrate();
#endif //DRV_TEMP_H__
