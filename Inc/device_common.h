#ifndef DEVICE_COMMON_H__
#define DEVICE_COMMON_H__

#include "device_config.h"

typedef enum
{
	DRV_OK                  = 0,
	DRV_ERR_INVALID_PARAM   = 1,
	DRV_ERR_BUSY            = 2,
	DRV_ERR_RTOS_START      = 3,
    DRV_ERR_RESOURCE        = 3,
} ReturnCode;

#endif //DEVICE_CONFIG_H__
