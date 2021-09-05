#ifndef DRV_BT_H__
#define DRV_BT_H__

#include "main.h"

typedef enum
{
	BT_OK 					= 0,
	BT_ERR_UART_PE  = 1,
	BT_ERR_UART_FE  = 2,
	BT_ERR_UART_NE  = 4,
	BT_ERR_UART_ORE = 8,
	BT_ERR_CMD_OV   = 16,
	BT_ERR_CMD_WRNG = 32,
	BT_ERR_PARAM_WRNG = 64
} BTStatus_t;

//Initialize bluetooth driver
ReturnCode drv_bt_init(QueueHandle_t *pxCMDQueue, SemaphoreHandle_t *pxTXSemaphore);

//Send response function
ReturnCode drv_bt_send_resp(const uint8_t * resp, uint8_t len);

//Read last command status
BTStatus_t drv_bt_read_and_clear_status();

#endif //DRV_BT_H__
