#ifndef MAIN_H__
#define MAIN_H__

#include "stdbool.h"
#include "string.h"

#include "stm32f10x.h"
#include "device_common.h"
#include "core_cm3.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

#define BT_RESP_PAT_OK  "OK%04d"
#define BT_RESP_PAT_ERR "ER%04X"

//First statments should be equal with BTStatus_t
typedef enum
{
    CMD_RESP_OK             = 0,
    CMD_RESP_ERR_UART_PE    = 1,
    CMD_RESP_ERR_UART_FE    = 2,
    CMD_RESP_ERR_UART_NE    = 4,
    CMD_RESP_ERR_UART_ORE   = 8,
    CMD_RESP_ERR_CMD_OVR    = 16,
    CMD_RESP_ERR_CMD_INV    = 32,
    CMD_RESP_ERR_PARAM_INV  = 64,
} CMDRespCode_t;


typedef enum
{
    REQ_INVALID     = 0,
    REQ_READ_TEMP   = 1,
    REQ_READ_STAT   = 2
} CMDRequestCode_t;

typedef enum
{
    DIR_INVALID         = 0,
    DIR_WRITE_TEMP      = 1,
    DIR_WRITE_LOG_PRIO  = 2
} CMDDirectiveCode_t;

typedef struct
{
  CMDDirectiveCode_t eDirective;
  uint32_t plParams[TSK_BT_MAX_PARAM_CNT];  
} CMDDirective_t;

typedef struct {
    BaseType_t xIsError;
    union
    {
        CMDRespCode_t   eErrCode;
        int16_t         sRespVal;
    }param;
} CMDRespStruct_t;




#endif //MAIN_H__
