#include "main.h"
#include "drv_bt.h"
#include "sp_bt_task.h"
#include "stdlib.h"
#include "stdio.h"

static QueueHandle_t  	 m_xBTRawCMDQueue;
static QueueHandle_t  	 m_xCMDReqQueue;
static QueueHandle_t  	 m_xCMDRespQueue;
static SemaphoreHandle_t m_xTXSemaphore;
static SemaphoreHandle_t m_xConfigDBMutex;

struct
{
    int16_t  sRequiredTemperature;
    int16_t  sCurrentTemperature;
    float 	 fSlewRate;
    uint32_t ulStatus;
} xConfigDB;


//NOTE: This function called from ISR, so use *fromISR function
void vBTEventHandler (BTEventType_t xEventType, const void * pv_context)
{
    
    switch(xEventType){
        case BT_EVT_CMD_RX_END: 
            configASSERT(xQueueSendFromISR(m_xBTRawCMDQueue, pv_context, NULL) != pdTRUE);
            break;
        case BT_EVT_RESP_TX_END:
            xSemaphoreGiveFromISR(m_xTXSemaphore, NULL);
            break;
        case BT_EVT_CMD_RX_ERR:
        {
            //
            uint8_t pucTmpBuffer[DRV_BT_RX_BUFF_LEN] = {0};
            configASSERT(xQueueSendFromISR(m_xBTRawCMDQueue, pv_context, NULL) != pdTRUE);
        }break;
        default:
            break;
		}


}
void vRequestHandlerTask(void * pbParam){
    CMDRequest_t eRequest;
    CMDRespStruct_t xResponce;
    BaseType_t xQueueResult;
    for(;;){
        xQueueReceive(m_xCMDReqQueue, &eRequest, portMAX_DELAY);
        switch(eRequest){
            case REQ_READ_TEMP:	
                xResponce.xIsError = pdFALSE;
                xResponce.param.sRespVal = 32;
                break;
            case REQ_READ_STAT:
                xResponce.xIsError = pdFALSE;
                xResponce.param.sRespVal = 32;
                break;
            case REQ_INVALID:
                xResponce.xIsError = pdTRUE;
                xResponce.param.eErrCode = CMD_RESP_ERR_CMD_INV;
                break;
            default:
                //IMPOSIBLE SITUATION FIX IT
                break;
        }
        xQueueResult = xQueueSend(m_xCMDRespQueue, &xResponce, pdMS_TO_TICKS(STD_Q_TIMEOUT));
        if(xQueueResult == pdFALSE)
        {
            //TODO: ERROR
        }
    }
    vTaskDelete(NULL);
}
void vBTTask(void * pvParams){
    uint8_t pucCMD[TSK_BT_CMD_LEN] = {0};
    int16_t sParamCnt = 0;
    int32_t plParamArr[TSK_BT_MAX_PARAM_CNT];
    uint8_t	pucResp[TSK_BT_RESP_LEN + 1] = {0};
    
    CMDRespStruct_t xResponce;
    BaseType_t xQueueResult;
    for(;;){
        xQueueReceive(m_xBTRawCMDQueue, pucCMD, portMAX_DELAY);
        //check if error
        BTStatus_t 	eBTStatus = drv_bt_read_and_clear_status();
        if(eBTStatus == BT_OK)
        {
            //Check and get parameters 
            sParamCnt = sSPSplitDigits(plParamArr,(pucCMD + TSK_BT_CMD_ONLY_LEN), \
                        TSK_BT_PARAM_SPLT_CH, '\n', \
                        TSK_BT_MAX_PARAM_CNT, (TSK_BT_CMD_LEN - TSK_BT_CMD_ONLY_LEN));
            
            //is that directive check(every direvtive has a parameter)
			if(sParamCnt > 1)
            {
                //Search directicve
                //check is the request is "wrtmp"
                if(memcmp(pucCMD, "wrtmp", TSK_BT_CMD_ONLY_LEN) == 0)
                {
                    
                }
            }	
            else if(sParamCnt == 0)//is that request check(every request has no parameters)
            {
                //Search requests
                CMDRequest_t eRequest = REQ_INVALID;
                //check is the request is "rdtmp"
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
                }
                //Wait for ReqHandlerTask answer
                xQueueResult = xQueueReceive(m_xCMDRespQueue, &xResponce, pdMS_TO_TICKS(STD_Q_TIMEOUT));
                if(xQueueResult == pdFALSE)
                {
                    //TODO: Critical Error???
                }
            }
            else //error handling 
            {
                xResponce.xIsError = pdFALSE;
                xResponce.param.eErrCode = CMD_RESP_ERR_PARAM_INV;
			}
            //Form string responce for user
            if(xResponce.xIsError == pdTRUE)
            {
                sprintf((char *)pucResp, BT_RESP_PAT_ERR, xResponce.param.eErrCode);
            }
            else
            {
                sprintf((char *)pucResp, BT_RESP_PAT_OK, xResponce.param.sRespVal);
            }
        }
        else //if have any drv_bt errors
        {
            //Generate error responce(Hare we use BTStatus_t, because BTStatus_t error codes are same with BTResp_t)
            sprintf((char *)pucResp, BT_RESP_PAT_ERR, eBTStatus);
        }
        //Take UART TX Semaphore 
        xSemaphoreTake(m_xTXSemaphore, pdMS_TO_TICKS(STD_Q_TIMEOUT));
        //Send responce
		ReturnCode eRetCode = drv_bt_send_resp(pucResp, TSK_BT_RESP_LEN);
        if(eRetCode !=  DRV_OK)
        {
			//TODO: critical error?
        }
			
	}
    vTaskDelete(NULL);
}

int main(void){
    m_xBTRawCMDQueue 	    = xQueueCreate(TSK_BT_CMD_Q_LEN, DRV_BT_RX_BUFF_LEN);
    m_xCMDReqQueue 			= xQueueCreate(TSK_REQ_HND_REQ_Q_LEN, sizeof(CMDRequest_t));
    m_xCMDRespQueue 		= xQueueCreate(TSK_CMM_RESP_Q_LEN, sizeof(CMDRespStruct_t));
    m_xTXSemaphore   		= xSemaphoreCreateBinary();
    m_xConfigDBMutex 		= xSemaphoreCreateMutex();
		
		
    ReturnCode xDriverStatus = drv_bt_init(vBTEventHandler);
    BaseType_t xTaskStatus;
    xTaskStatus = xTaskCreate(vBTTask, "BTTask", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskStatus = xTaskCreate(vRequestHandlerTask, "RequestHandlerTask", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    vTaskStartScheduler(); 
    return -1;
}
