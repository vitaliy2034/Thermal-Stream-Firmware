#include "drv_temp.h"



static uint8_t m_pucTxBuffer[DRV_BT_TX_BUFF_LEN] = {0};
static uint8_t m_ucTxBufferPointer = 0;
static uint8_t m_ucTxBufferLength  = 0;




ReturnCode drv_temp_init(){

	

	
	//Enable clocking of ADC1
	RCC->APB1ENR |= RCC_APB2ENR_ADC1EN;
	
	//Enable clocking of GPIOA
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	
	//Configure GPIOA: PA0 to ANALOG for ADC1
	GPIOA->CRL &= ~GPIO_CRL_MODE0; 
	GPIOA->CRL &= ~GPIO_CRL_CNF0;

	//Configure ADC1
	ADC1->CR1 |= ADC_CR1_EOCIE;
//	ADC1->CR2 |= ADC_
	//Set prioryty of USART2 interupt
	//NVIC_SetPriority(USART2_IRQn, 6);
	
	//Enable USART2 interupt
	//NVIC_EnableIRQ(USART2_IRQn);
	
	return DRV_OK;
}
