#ifndef DEVICE_CONFIG_H__
#define DEVICE_CONFIG_H__
//
//Common defines
//
#define PCLK1			8000000UL
#define MAX_TEMP  120
#define MIN_TEMP  -10

//
//Common defines for tasks
//

//Length of responce`s queue(should be 2)
#define TSK_CMM_RESP_Q_LEN 2

//
//Bluetooth driver drv_bt defines
//

// Baud rate in bods per second
#define DRV_BT_UART_BAUD_RATE 9600

// Stop bit count		
#define DRV_BT_UART_STOP_BIT	1
/* 
	1 - 1 Stop Bit
	2 - 2 Stop Bits
*/

//Parity check
#define DRV_BT_UART_PAR_CHECK 0
/* 
	0 - No parity check
	1 - ODD parity check
	2 - EVEN parity check
*/

//End of line type
#define DRV_BT_EOL_TYPE 2
/*	
	0 - LF, (CR+LF complitable for RX) (Line Feed, Code: 0x0D), OS: Linux
	1 - CR (Carriage Return, Code: 0x0A), OS: oldMacOS
	2 - CR+LF,(LF complitable for RX) OS: Windows
*/

//Length of RX buffer in which the command will be stored
#define DRV_BT_RX_BUFF_LEN   10

//Length of TX buffer in which the responce will be stored
#define DRV_BT_TX_BUFF_LEN   (6 + 2)


//
//Bluetooth task defines
//

//Length of queue of raw commands(should be 2)
#define TSK_BT_CMD_Q_LEN		 2

//Length of responce
#define TSK_BT_RESP_LEN			 6

//Max length of command	with parameters
#define TSK_BT_CMD_LEN			 DRV_BT_RX_BUFF_LEN

//Length of instruction
#define TSK_BT_CMD_ONLY_LEN  5

//Max parameter count of command 
#define TSK_BT_MAX_PARAM_CNT 3

//Parameter split character
#define TSK_BT_PARAM_SPLT_CH '_'

//
//Request handler defines
//

//Length of request`s queue(should be 2)
#define TSK_REQ_HND_REQ_Q_LEN 2


#endif //DEVICE_CONFIG_H__
