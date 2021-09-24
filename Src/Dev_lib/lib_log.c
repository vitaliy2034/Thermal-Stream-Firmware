#include "lib_log.h"

#define LIB_LOG_PRIO_STR_LEN 4

static ePrioLevel m_ePrioLevel;
static LibLogSend_t m_xSendMessage;

static BaseType_t _xPrioCheck(ePrioLevel ePrioToCheck){
    //Check is parameter in range of values
    if( LIB_LOG_DEBUG >= ePrioToCheck && ePrioToCheck >= LIB_LOG_DISABLE)
        return pdTRUE;
    else
        return pdFALSE;
}

ReturnCode xLibLogInit(LibLogSend_t xLogSendFunction, ePrioLevel eStartPriority){
    //Check input parameters
    if(_xPrioCheck(eStartPriority) == pdFALSE || xLogSendFunction == NULL)
        return DRV_ERR_INVALID_PARAM;
    
    //Set start parameters
    m_ePrioLevel   = eStartPriority;
    m_xSendMessage = xLogSendFunction;
    
    return DRV_OK;
}

ReturnCode xLibLogSend(const portCHAR * pucModuleName, ePrioLevel ePriority, const portCHAR * pucMessage, ...){
    //Check input parameters
    if(_xPrioCheck(ePriority) == pdFALSE || ePriority == 0 || pucModuleName == NULL)
        return DRV_ERR_INVALID_PARAM;
    
    //Check does program should write message
    if(ePriority > m_ePrioLevel)
        return DRV_OK;
    
    portCHAR pucLogBuffer[LIB_LOG_BUFF_LEN] = {0};
    
    portCHAR pucPrioStr[LIB_LOG_PRIO_STR_LEN];
    //Add type
    switch(ePriority)
    {
        case LIB_LOG_DEBUG:
            strcpy(pucPrioStr, "dbg");
            break;
        case LIB_LOG_INFO:
            strcpy(pucPrioStr, "inf");
            break;
        case LIB_LOG_WARNING:
            strcpy(pucPrioStr, "wrn");
            break;
        case LIB_LOG_ERROR:
            strcpy(pucPrioStr, "err");
            break;
        default:
            strcpy(pucPrioStr, "udf");
            break;
    
    }
    uint16_t usTitleLen = sprintf(pucLogBuffer, "#%s:%s:", pucPrioStr, pucModuleName);
    if(pucMessage != NULL && usTitleLen > 0)
    {
        va_list args;
    
        va_start (args, pucMessage);
        usTitleLen = (uint16_t)vsprintf ((pucLogBuffer + usTitleLen),  pucMessage, args);
    
        va_end (args);
    }
    
    if(usTitleLen <= 0)
        return DRV_ERR_RESOURCE;
    
    m_xSendMessage(pucLogBuffer, (uint16_t)strlen(pucLogBuffer));
    
    return DRV_OK;
}

ReturnCode xLibLogLevelSet(ePrioLevel eStartPrioritySet){
    //Check input parameters
    if(_xPrioCheck(eStartPrioritySet) == pdFALSE)
        return DRV_ERR_INVALID_PARAM;
    
    portENTER_CRITICAL();
    m_ePrioLevel =  eStartPrioritySet;
    portEXIT_CRITICAL();
    return DRV_OK;
}
