#ifndef DRV_TEMP_H__
#define DRV_TEMP_H__

#include "main.h"



//Initialize bluetooth driver
ReturnCode drv_temp_init(QueueHandle_t *pxTempQueue);

//Read last command status
void drv_temp_read_and_clear_status();

#endif //DRV_TEMP_H__
