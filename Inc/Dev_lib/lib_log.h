#ifndef LIB_LOG_H__
#define LIB_LOG_H__

#include "main.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

typedef enum{
    LIB_LOG_DISABLE = 0x0,
    LIB_LOG_ERROR   = 0x1,
    LIB_LOG_WARNING = 0x2,
    LIB_LOG_INFO    = 0x3,
    LIB_LOG_DEBUG   = 0x4
    
} ePrioLevel;

typedef void (*LibLogSend_t )(const portCHAR *, uint16_t);
ReturnCode xLibLogInit(LibLogSend_t xLogSendFunction, ePrioLevel eStartPrioritySet);

ReturnCode xLibLogSend(const portCHAR * pucModuleName, ePrioLevel ePriority, const portCHAR * pucMessage, ...);

ReturnCode xLibLogLevelSet(ePrioLevel eStartPrioritySet);

//TODO: Finish it
//ReturnCode xLibLogFiltEn();

#endif //LIB_LOG_H__
