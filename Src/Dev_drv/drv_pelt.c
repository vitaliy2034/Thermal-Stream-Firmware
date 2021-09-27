#include "drv_pelt.h"



ReturnCode drv_pelt_init () {
    
    //Enable clocking of TIM2
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	
	//Enable clocking of GPIOB
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
	
	//Configure GPIOB
    
    //PB6 and PB7 to output Push-Pull: 
    GPIOB->CRL |= GPIO_CRL_MODE6 | GPIO_CRL_MODE7; 
	GPIOB->CRL &= ~(GPIO_CRL_CNF6 | GPIO_CRL_CNF7);
    
    //PB10 and PB11 to alternete output Push-Pull:
	GPIOB->CRH |= GPIO_CRH_MODE10 | GPIO_CRH_MODE11; 
	GPIOB->CRH |= GPIO_CRH_CNF10_1 | GPIO_CRH_CNF11_1;
    
    //Remap alternate functions PB10 and PB11 to TIM2 CH3 and CH4
    AFIO->MAPR |= AFIO_MAPR_TIM2_REMAP_1;
	
    //Configure TIM2 to PWM output

    //Enable auto-reload register preinstall
    TIM2->CR1 |= TIM_CR1_ARPE;
    
	//Setup prescaler to 600
    TIM2->PSC = 600;
    
    //Set PWM mode 1 for 4,3 capture chanels
    TIM2->CCMR2 |= TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2;  
    
    //Enable preload register on TIM2->CCR3 and TIM2->CCR4
    TIM2->CCMR2 |= TIM_CCMR2_OC3PE | TIM_CCMR2_OC4PE;
    
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
