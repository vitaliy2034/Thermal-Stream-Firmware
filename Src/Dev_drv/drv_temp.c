#include "drv_temp.h"

static QueueHandle_t  	 *m_pxCMDQueue;
static SemaphoreHandle_t *m_pxTXSemaphore;

static BTStatus_t				m_eBTStatus;

static uint8_t m_pucTxBuffer[DRV_BT_TX_BUFF_LEN] = {0};
static uint8_t m_ucTxBufferPointer = 0;
static uint8_t m_ucTxBufferLength  = 0;


void USART2_IRQHandler(void){
		static uint8_t pucRxBuffer[DRV_BT_RX_BUFF_LEN] = {0};
		static uint8_t ucRxBufferPointer = 0;

		uint32_t ulUartStatus = USART2->SR;
		if(ulUartStatus & USART_SR_TXE)
		{
				//Send data if TX buffer not empty
				if(m_ucTxBufferPointer < m_ucTxBufferLength)
				{
						USART2->DR = m_pucTxBuffer[m_ucTxBufferPointer];
						m_ucTxBufferPointer++;
				}
				//Give semaphore that TX is ready to transmit
				else
				{
						USART2->CR1 &= ~(USART_CR1_TXEIE);
						xSemaphoreGiveFromISR(*m_pxTXSemaphore, NULL);
				}
		}
		if(ulUartStatus & (USART_SR_PE | USART_SR_FE | USART_SR_NE | USART_SR_ORE))
		{
				//Set error bits
				m_eBTStatus |= ulUartStatus & (BT_ERR_UART_PE | BT_ERR_UART_FE | BT_ERR_UART_NE | BT_ERR_UART_ORE);
		
				//Clear buffer
				memset(pucRxBuffer, 0, DRV_BT_RX_BUFF_LEN);
				ucRxBufferPointer = 0;
		
				// send empty command to start BT task
				if(xQueueSendFromISR(*m_pxCMDQueue, pucRxBuffer, 0) == pdFALSE)
						m_eBTStatus |= BT_ERR_CMD_OV;
		
		}
		if(ulUartStatus & USART_SR_RXNE)
		{
				uint16_t usRxSymbol = USART2->DR;
		
				// ignore space symbol
				if(usRxSymbol == ' ')
						return;

				
#if (DRV_BT_EOL_TYPE == 0) || (DRV_BT_EOL_TYPE == 2)
				// ignore '\r' symbol
				if(usRxSymbol == '\r')
						return;
				
				// if '\n' symbol, send command to queue
				if(usRxSymbol == '\n')
#else
				// if '\r' symbol, send command to queue
				if(usRxSymbol == '\r')
#endif
				{
						// send command
						if(xQueueSendFromISR(*m_pxCMDQueue, pucRxBuffer, 0) == pdFALSE)
								m_eBTStatus |= BT_ERR_CMD_OV;
			
						//Clear buffer
						memset(pucRxBuffer, 0, DRV_BT_RX_BUFF_LEN);
						ucRxBufferPointer = 0;
						return;
				}
				// if command length more than DRV_BT_RX_BUFF_LEN, send empty command
				if(ucRxBufferPointer == DRV_BT_RX_BUFF_LEN)
				{
						//Set BT_ERR_CMD_OV error bit
						m_eBTStatus |= BT_ERR_CMD_OV;
			
						//Clear buffer
						memset(pucRxBuffer, 0, DRV_BT_RX_BUFF_LEN);
						ucRxBufferPointer = 0;
			
						// send empty command to start BT task
						if(xQueueSendFromISR(*m_pxCMDQueue, pucRxBuffer, 0) == pdFALSE)
								m_eBTStatus |= BT_ERR_CMD_OV;
			
						return;
				}
				pucRxBuffer[ucRxBufferPointer] = usRxSymbol;
				ucRxBufferPointer++;
	}
}

ReturnCode drv_bt_send_resp(const uint8_t * pucResp, uint8_t ucLen){
	
		//Check input parameters	
		if(pucResp == NULL || ucLen >= DRV_BT_TX_BUFF_LEN || ucLen == 0)
				return DRV_ERR_INVALID_PARAM;
	
		//Check is the system busy
		if(m_ucTxBufferPointer != m_ucTxBufferLength)
				return DRV_ERR_BUSY;
		
		//Disable interrupts
		taskENTER_CRITICAL();
		
		//Copy parameters
		memcpy(m_pucTxBuffer, pucResp, ucLen); 
		//Reset pointer
		m_ucTxBufferPointer = 0;
#if DRV_BT_EOL_TYPE == 2		
		m_ucTxBufferLength = ucLen + 2;
		memcpy(m_pucTxBuffer + ucLen, "\r\n", 2);
#else		
		m_ucTxBufferLength = ucLen + 1;
#if DRV_BT_EOL_TYPE == 0
		m_pucTxBuffer[ucLen] = '\n'; 
#else
		m_pucTxBuffer[ucLen] = '\r';
#endif		
#endif
		
		//Enable TXE interrupt
		USART2->CR1 |= USART_CR1_TXEIE;
		
		//Enable interrupts
		taskEXIT_CRITICAL();
		return DRV_OK;
}
BTStatus_t drv_bt_read_and_clear_status(){
		BTStatus_t temp = m_eBTStatus;
		m_eBTStatus 		= BT_OK;
		return temp;
}

ReturnCode drv_bt_init(QueueHandle_t *pxCMDQueue, SemaphoreHandle_t *pxTXSemaphore){
	//Check is parameters are correct
	if(pxCMDQueue == NULL || pxTXSemaphore == NULL)
		return DRV_ERR_INVALID_PARAM;
	
	//save pointers
	m_pxCMDQueue 		= pxCMDQueue;
	m_pxTXSemaphore = pxTXSemaphore;

	m_eBTStatus = BT_OK;
	
	//Enable clocking of ADC1
	RCC->APB1ENR |= RCC_APB2ENR_ADC1EN;
	
	//Enable clocking of GPIOA
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	
	//Configure GPIOA: PA0 to ANALOG for ADC1
	GPIOA->CRL &= ~GPIO_CRL_MODE0; 
	GPIOA->CRL &= ~GPIO_CRL_CNF0;

	//Configure ADC1
	ADC1->CR1 |= ADC_CR1_EOCIE;
	ADC1->CR2 |= ADC_
	//Set prioryty of USART2 interupt
	NVIC_SetPriority(USART2_IRQn, 6);
	
	//Enable USART2 interupt
	NVIC_EnableIRQ(USART2_IRQn);
	
	return DRV_OK;
}
