#ifndef DEVICE_CONFIG_H__
#define DEVICE_CONFIG_H__
//
//Common defines
//
#define PCLK1     8000000UL
#define MAX_TEMP  120
#define MIN_TEMP  -10
#define STD_Q_TIMEOUT 1000

//
//Common defines for tasks
//

//Length of responce`s queue(should be 2)
#define TSK_CMM_RESP_Q_LEN 2


//
//Logging library defines
//

#define LIB_LOG_BUFF_LEN 250

//
//Peltier driver drv_pelt defines
//

#define DRV_PELT_PWM_PSC  0

#define DRV_PELT_PWM_RES  8

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
#define DRV_BT_EOL_TYPE 1
/*	
	0 - LF, (CR+LF complitable for RX) (Line Feed, Code: 0x0D), OS: Linux
	1 - CR+LF,(LF complitable for RX) OS: Windows
*/

//Length of RX buffer in which the command will be stored
#define DRV_BT_RX_BUFF_LEN   10

//Length of TX buffer in which the responce will be stored
#define DRV_BT_TX_BUFF_LEN   (6 + 2 + LIB_LOG_BUFF_LEN)


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
//Request handler task defines
//

//Length of request`s queue(should be 2)
#define TSK_REQ_HND_REQ_Q_LEN 2

//
//PID task defines
//

#define TSK_PID_DELTA_TIME 10

#define TSK_PID_KOEF_P     0.0f

#define TSK_PID_KOEF_I     0.0f

#define TSK_PID_KOEF_D     0.0f

//Length of temperature`s queue(should be 2)
#define TSK_PID_TEMP_Q_LEN 2

#endif //DEVICE_CONFIG_H__
