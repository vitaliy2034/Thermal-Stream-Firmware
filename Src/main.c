#include "main.h"
#include "drv_bt.h"
#include "stdlib.h"
#include "stdio.h"

static QueueHandle_t  	 m_xCMDQueue;
static SemaphoreHandle_t m_xTXSemaphore;
static SemaphoreHandle_t m_xConfigDBMutex;

struct
{
		int16_t  sRequiredTemperature;
		int16_t  sCurrentTemperature;
		float 	 fSlewRate;
		uint32_t ulStatus;
} xConfigDB;

void vSendResp(bool bIsError, int16_t ulResp){
		uint8_t		pucResp[TSK_BT_RESP_LEN + 1] = {0};

		if(bIsError)
				sprintf((char *)pucResp, "ER%04X", ulResp);
		else
				sprintf((char *)pucResp, "OK%04X", ulResp);
		
		//Take UART TX Semaphore 
		xSemaphoreTake(m_xTXSemaphore, portMAX_DELAY);
		//Send responce
		drv_bt_send_resp(pucResp, TSK_BT_RESP_LEN);
}

void vBTTask(void * pvParams){
	uint8_t 		pucCMD[TSK_BT_CMD_LEN] = {0};
	for(;;){
		xQueueReceive(m_xCMDQueue, pucCMD, portMAX_DELAY);
		//check if error
		BTStatus_t 	eCMDStatus = drv_bt_read_and_clear_status();
		if(eCMDStatus != BT_OK)
		{
				vSendResp(pdTRUE, eCMDStatus);
		}
		//check is the command is "wrtmp"
		else if(memcmp(pucCMD, "wrtmp", TSK_BT_CMD_ONLY_LEN) == 0)
		{
				int16_t sDecodedTemperature = 0;
				for(uint8_t i = TSK_BT_CMD_ONLY_LEN; i < TSK_BT_CMD_LEN; i++)
				{		
						//check is end
						if(pucCMD[i] == '\0' && i > TSK_BT_CMD_ONLY_LEN)
						{
								break;
						}
						//convert ASCII to digit
						else if(0x3A > pucCMD[i] && pucCMD[i] >= 0x30)
						{
								sDecodedTemperature = (sDecodedTemperature * 10) + (pucCMD[i] - 0x30);
						}
						//if hare - there is error
						else
						{
								sDecodedTemperature = 0x7FFF;
								break;
						}
				}
				//Check for valid range and error
				if(sDecodedTemperature > MAX_TEMP || MIN_TEMP > sDecodedTemperature)
				{
						vSendResp(pdTRUE, BT_ERR_PARAM_WRNG);
						continue;
				}
				xSemaphoreTake(m_xConfigDBMutex, portMAX_DELAY);
				xConfigDB.sRequiredTemperature = sDecodedTemperature;
				xSemaphoreGive(m_xConfigDBMutex);
				vSendResp(false, 0);
		}
		//check is the command is "rdtmb"
		//
		
	}
	vTaskDelete(NULL);
}

int main(){
	m_xCMDQueue 	   		= xQueueCreate(TSK_BT_CMD_Q_LEN, DRV_BT_RX_BUFF_LEN);
	m_xTXSemaphore   		= xSemaphoreCreateBinary();
	m_xConfigDBMutex 		= xSemaphoreCreateMutex();
	
	
	ReturnCode xDriverStatus = drv_bt_init(&m_xCMDQueue, &m_xTXSemaphore);
	BaseType_t xTaskStatus   = xTaskCreate(vBTTask, "BTTask", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	vTaskStartScheduler(); 
	return -1;
}
