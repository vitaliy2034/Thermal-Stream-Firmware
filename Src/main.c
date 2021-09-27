#include "main.h"
#include "drv_bt.h"
#include "drv_temp.h"
#include "sp_bt_task.h"
#include "lib_log.h"
#include "stdlib.h"
#include "stdio.h"

static QueueHandle_t  	 m_xBTRawCMDQueue;
static QueueHandle_t  	 m_xCMDReqQueue;
static QueueHandle_t  	 m_xCMDRespQueue;
static QueueHandle_t  	 m_xTempQueue;

static SemaphoreHandle_t m_xTXSemaphore;

static SemaphoreHandle_t m_xConfigDBMutex;

struct
{
    temp_t  sRequiredTemperature;
    temp_t  sCurrentTemperature;
    float 	fSlewRate;
} m_xConfigDB;

//TODO: Initialization check


//NOTE: This function called from ISR, so use *fromISR function
void vBTEventHandler (BTEventType_t xEventType, const void * pv_context)
{
    portBASE_TYPE xQueueResult;
    switch(xEventType){
        case BT_EVT_CMD_RX_END: 
            xQueueResult = xQueueSendFromISR(m_xBTRawCMDQueue, pv_context, NULL);
            xLibLogSend(__func__, LIB_LOG_DEBUG, "Command send to task: %s", pv_context);
            if(xQueueResult == pdFALSE)
            {
                xLibLogSend(__func__, LIB_LOG_ERROR, "Error to send, queue is full: %s", pv_context);
            }
            break;
        case BT_EVT_RESP_TX_END:
            xSemaphoreGiveFromISR(m_xTXSemaphore, NULL);
            break;
        case BT_EVT_CMD_RX_ERR:
        {
            //Send empty element to inform about 
            uint8_t pucTmpBuffer[DRV_BT_RX_BUFF_LEN] = {0};
            xLibLogSend(__func__, LIB_LOG_WARNING, "Command reception error, send to task: %04X", *((BTStatus_t *)pv_context));
            xQueueResult = xQueueSendFromISR(m_xBTRawCMDQueue, pucTmpBuffer, NULL);
            if(xQueueResult == pdFALSE)
            {
                xLibLogSend(__func__, LIB_LOG_ERROR, "Error to send command, queue is full: %s", pv_context);
            }
        }break;
        default:
            break;
		}
}


//NOTE: This function called from ISR, so use *fromISR function
void vTempEventHandler(TempEventType_t xEventType, const void * pv_context){
    
    portBASE_TYPE xQueueResult;
    switch(xEventType)
    {
        case TEMP_EVT_READY:
            xQueueResult = xQueueSendFromISR(m_xTempQueue, pv_context, NULL);
            if(xQueueResult == pdFALSE)
            {
                xLibLogSend(__func__, LIB_LOG_ERROR, "Error to send temperature, queue is full.");
            }
            break;
    }

}

static void vSendMessage(const portCHAR * pucMessage, uint16_t ucMsgLen){
    portBASE_TYPE xStatus;
    //Take UART TX Semaphore 
    if(xPortIsInsideInterrupt() == pdFALSE)
    {
        
        xStatus = xSemaphoreTake(m_xTXSemaphore, pdMS_TO_TICKS(STD_Q_TIMEOUT));
        //Check error to take Semaphore in Task
        if(xStatus == pdFALSE)
        {
            //TODO: critical error
            configASSERT(pdFALSE);
        }
    }
    else
    {
        xStatus = xSemaphoreTakeFromISR(m_xTXSemaphore, NULL);
        //Check error to take Semaphore in ISR
        if(xStatus == pdFALSE)
        {
            //Do nothing because it is logs
            return;
        }
    }
    
    //Send responce
    ReturnCode eRetCode = drv_bt_send((uint8_t *)pucMessage, ucMsgLen);
    if(eRetCode !=  DRV_OK)
    {
        //TODO: critical error?
        configASSERT(pdFALSE);
    }
}

void vRequestHandlerTask(void * pbParam){
    CMDRequest_t eRequest;
    CMDRespStruct_t xResponce;
    BaseType_t xQueueResult;
    
    //Give semaphore after start
    xSemaphoreGive(m_xTXSemaphore);
    
    for(;;){
        xQueueReceive(m_xCMDReqQueue, &eRequest, portMAX_DELAY);
        switch(eRequest){
            case REQ_READ_TEMP:	
                xResponce.xIsError = pdFALSE;
                xSemaphoreTake(m_xConfigDBMutex, portMAX_DELAY);
                xResponce.param.sRespVal = m_xConfigDB.sCurrentTemperature;
                xSemaphoreGive(m_xConfigDBMutex);
                break;
            case REQ_READ_STAT:
                xResponce.xIsError = pdFALSE;
                xResponce.param.sRespVal = 32;
                break;
            case REQ_INVALID:
                xResponce.xIsError = pdTRUE;
                xResponce.param.eErrCode = CMD_RESP_ERR_CMD_INV;
                xLibLogSend(__func__, LIB_LOG_WARNING, "Invalis command recived"); 
                break;
            default:
                xLibLogSend(__func__, LIB_LOG_ERROR, "Invalis situation, default worked: %d", __LINE__); 
                break;
        }
        xQueueResult = xQueueSend(m_xCMDRespQueue, &xResponce, pdMS_TO_TICKS(STD_Q_TIMEOUT));
        if(xQueueResult == pdFALSE)
        {
            //TODO: ERROR
            xLibLogSend(__func__, LIB_LOG_ERROR, "Error to send responce, queue is full for long time");
        }
    }
    vTaskDelete(NULL);
}
void vBTTask(void * pvParams){
    portCHAR pucCMD[TSK_BT_CMD_LEN] = {0};
    int16_t  sParamCnt = 0;
    int32_t  plParamArr[TSK_BT_MAX_PARAM_CNT];
    portCHAR pucResp[TSK_BT_RESP_LEN + 1] = {0};
    
    CMDRespStruct_t xResponce;
    BaseType_t xQueueResult;
    for(;;){
        xQueueReceive(m_xBTRawCMDQueue, pucCMD, portMAX_DELAY);
        xLibLogSend(__func__, LIB_LOG_DEBUG, "Command Recived: %s", pucCMD);
        //check if BT driver error
        BTStatus_t 	eBTStatus = drv_bt_read_status();
        if(eBTStatus == BT_OK)
        {
            xLibLogSend(__func__, LIB_LOG_DEBUG, "BT is OK");
            //Check and get parameters 
            sParamCnt = sSPSplitDigits(plParamArr, (uint8_t *)(pucCMD + TSK_BT_CMD_ONLY_LEN), \
                        TSK_BT_PARAM_SPLT_CH, '\n', \
                        TSK_BT_MAX_PARAM_CNT, (TSK_BT_CMD_LEN - TSK_BT_CMD_ONLY_LEN));
            
            xLibLogSend(__func__, LIB_LOG_DEBUG, "Command Parameters count: %hd", sParamCnt);
            
            //Is that directive check(every direvtive has a parameter)
			if(sParamCnt > 1)
            {
                //Search directicve
                //check is the request is "wrtmp"
                if(memcmp(pucCMD, "wrtmp", TSK_BT_CMD_ONLY_LEN) == 0)
                {
                    
                }
            }	
            else if(sParamCnt == 0)//Is that request check(every request has no parameters)
            {
                //Search requests
                CMDRequest_t eRequest = REQ_INVALID;
                //Check is the request is "rdtmp"
                if(memcmp(pucCMD, "rdtmp", TSK_BT_CMD_ONLY_LEN) == 0)
                {
                    eRequest = REQ_READ_TEMP;
                }
                else if(memcmp(pucCMD, "rdsts", TSK_BT_CMD_ONLY_LEN) == 0)
                {
                    eRequest = REQ_READ_STAT;
                }
				//TODO: ??
                //Send to ReqHandlerTask through request queue
                xQueueResult = xQueueSend(m_xCMDReqQueue, &eRequest, pdMS_TO_TICKS(STD_Q_TIMEOUT));
                if(xQueueResult == pdFALSE)
                {
                    //TODO: Critical Error???
                    xLibLogSend(__func__, LIB_LOG_ERROR, "Can not send request, queue is full for long time");
                }
                //Wait for ReqHandlerTask answer
                xQueueResult = xQueueReceive(m_xCMDRespQueue, &xResponce, pdMS_TO_TICKS(STD_Q_TIMEOUT));
                if(xQueueResult == pdFALSE)
                {
                    //TODO: Critical Error???
                    xLibLogSend(__func__, LIB_LOG_ERROR, "Can not recive responce for long time");
                }
            }
            else //error handling 
            {
                xResponce.xIsError = pdFALSE;
                xResponce.param.eErrCode = CMD_RESP_ERR_PARAM_INV;
                xLibLogSend(__func__, LIB_LOG_WARNING, "Wrong parameters are entered");
			}
            //Form string responce for user
            if(xResponce.xIsError == pdTRUE)
            {
                sprintf(pucResp, BT_RESP_PAT_ERR, xResponce.param.eErrCode);
            }
            else
            {
                sprintf(pucResp, BT_RESP_PAT_OK, xResponce.param.sRespVal);
            }
        }
        else //If have any BT driver errors
        {
            //Generate error responce(Hare we use BTStatus_t, because BTStatus_t error codes are same with BTResp_t)
            sprintf(pucResp, BT_RESP_PAT_ERR, eBTStatus);
            xLibLogSend(__func__, LIB_LOG_ERROR, "Bad BT-UART connection: %04X", eBTStatus);
        }
        vSendMessage(pucResp, TSK_BT_RESP_LEN);
        //clear status of BT driver
        drv_bt_clear_status();
	}
    vTaskDelete(NULL);
}

void vPIDTask(void *pvParam){
    BaseType_t xResult;
    temp_t xTempBuffer;
    for(;;){
        //Start temperature read
        drv_temp_read_int();
      
        //Wait until drv_temp send temperature value
        //should changhe wait time 
        xQueueReceive(m_xTempQueue, &xTempBuffer, portMAX_DELAY);
        
        xLibLogSend(__func__, LIB_LOG_DEBUG, "Temperature recived: %f", xTempBuffer);
        
        //should changhe wait time 
        xResult = xSemaphoreTake(m_xConfigDBMutex, portMAX_DELAY);
        
        m_xConfigDB.sCurrentTemperature = xTempBuffer; 
        
        xSemaphoreGive(m_xConfigDBMutex);
        vTaskDelay(100);
    }
    vTaskDelete(NULL);
}

int main(void){
    m_xBTRawCMDQueue    = xQueueCreate(TSK_BT_CMD_Q_LEN, DRV_BT_RX_BUFF_LEN);
    m_xCMDReqQueue 		= xQueueCreate(TSK_REQ_HND_REQ_Q_LEN, sizeof(CMDRequest_t));
    m_xCMDRespQueue 	= xQueueCreate(TSK_CMM_RESP_Q_LEN, sizeof(CMDRespStruct_t));
    m_xTempQueue   	    = xQueueCreate(TSK_PID_TEMP_Q_LEN, sizeof(temp_t));
    
    m_xTXSemaphore  	= xSemaphoreCreateBinary();
    
    m_xConfigDBMutex    = xSemaphoreCreateMutex();
	
    ReturnCode xStatus = drv_bt_init(vBTEventHandler);
    xStatus |= drv_temp_init(vTempEventHandler);
    xStatus |= xLibLogInit(vSendMessage, LIB_LOG_DEBUG);
    
    configASSERT(!xStatus);
    
    BaseType_t xTaskStatus;
    xTaskStatus = xTaskCreate(vBTTask, "BTTask", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskStatus &= xTaskCreate(vRequestHandlerTask, "RequestHandlerTask", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskStatus &= xTaskCreate(vPIDTask, "PIDTask", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    
    configASSERT(xTaskStatus);
    
    vTaskStartScheduler(); 
    return -1;
}
