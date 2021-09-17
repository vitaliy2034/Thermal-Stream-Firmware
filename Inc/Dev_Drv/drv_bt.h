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
} BTStatus_t;

#define BT_OVF_CMD_BUFF {'O', '\0'}

typedef enum
{
		BT_EVT_CMD_RX_END  = 0x00,
		BT_EVT_RESP_TX_END = 0x01,
		BT_EVT_UART_ERR    = 0x02
} BTEventType_t;


typedef void (*BTEventHandler_t)(BTEventType_t xEventType, const void * pv_context);

//Initialize bluetooth driver
ReturnCode drv_bt_init(BTEventHandler_t xEventHandler);

//Send response function
ReturnCode drv_bt_send_resp(const uint8_t * resp, uint8_t len);

//Read last command status
BTStatus_t drv_bt_read_and_clear_status();

#endif //DRV_BT_H__
