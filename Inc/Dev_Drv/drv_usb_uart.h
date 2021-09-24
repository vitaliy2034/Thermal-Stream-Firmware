#ifndef DRV_USB_UART_H__
#define DRV_USB_UART_H__

#include "main.h"



typedef enum
{
    BT_EVT_CMD_RX_END   = 0x00,
    USB_UART_EVT_TX_END = 0x01,
    BT_EVT_CMD_RX_ERR   = 0x02,
} UsbUartEventType_t;


typedef void (*UsbUartEventHandler_t)(UsbUartEventType_t xEventType, const void * pv_context);

//Initialize usb-uart driver
ReturnCode drv_usb_uart_init(UsbUartEventHandler_t xEventHandler);

//Send function
ReturnCode drv_usb_uart_send(const uint8_t * text, uint8_t len);

#endif //DRV_USB_UART_H__
