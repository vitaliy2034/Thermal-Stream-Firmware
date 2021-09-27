#include "drv_temp.h"

static TempEventHandler_t m_xEventHandler;

//Coeficient for linear aproximation
const temp_t k = -0.0576, b = 150.3;

void ADC1_2_IRQHandler(void){
    uint16_t adc_result = ADC1->DR;
    
    temp_t temp_result = ((temp_t)adc_result) * k + b;
    
    m_xEventHandler(TEMP_EVT_READY, &temp_result);
}

ReturnCode drv_temp_read_int(void){
    if(m_xEventHandler == NULL)
        return DRV_ERR_INVALID_STATE;
    
    ADC1->CR2 |= ADC_CR2_ADON;
    
    return DRV_OK;
}

ReturnCode drv_temp_init (TempEventHandler_t xEventHandler) {
	//Check input parameters
    if(xEventHandler == NULL)
        return DRV_ERR_INVALID_PARAM;
    
    //Check is drv unitialized
    if(m_xEventHandler != NULL)
        return DRV_ERR_INVALID_STATE;
    
    //Set variables
    m_xEventHandler = xEventHandler;
    
    //Enable clocking of ADC1
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	
	//Enable clocking of GPIOA
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	
	//Configure GPIOA: PA0 to ANALOG for ADC1
	GPIOA->CRL &= ~GPIO_CRL_MODE0; 
	GPIOA->CRL &= ~GPIO_CRL_CNF0;
    
	//Configure ADC1

    //Enable ADC
    ADC1->CR2 |= ADC_CR2_ADON;
    
    //Enable ADC1 interrupt
	ADC1->CR1 |= ADC_CR1_EOCIE;
    
    //Set sample time 239.5 cycles
	ADC1->SMPR2 |= ADC_SMPR2_SMP0;
	
    //Configuring interrupts
    
    //Set prioryty of ADC1 interupt
    NVIC_SetPriority(ADC1_2_IRQn, 6);
    
    //Enable ADC1 interupt
    NVIC_EnableIRQ(ADC1_2_IRQn);
    
    //Make ADC1 calibration(shoul be delay 2 ADC clocks befor start calibration)
    ADC1->CR2 |= ADC_CR2_CAL;
    while(ADC1->CR2 & ADC_CR2_CAL);
    
	return DRV_OK;
}
