#include "drv_pelt.h"

ReturnCode drv_pelt_mode_set(PeltNum_t xPelt, PeltMode_t xMode){
    
    if(xMode !=  DRV_PELT_MODE_COLD && xMode !=  DRV_PELT_MODE_HEAT)
        return DRV_ERR_INVALID_PARAM;
    
    if(xPelt == DRV_PELT_1)
    {
        GPIOB->BSRR |= (xMode == DRV_PELT_MODE_COLD)? GPIO_BSRR_BS6 : GPIO_BSRR_BS6;
    }
    else if(xPelt == DRV_PELT_2)
    {
        GPIOB->BSRR |= (xMode == DRV_PELT_MODE_COLD)? GPIO_BSRR_BS7 : GPIO_BSRR_BS7;
    }
    else
    {
        return DRV_ERR_INVALID_PARAM;
    }
    
    return DRV_OK;
}

ReturnCode drv_pelt_pwm_duty_set(PeltNum_t xPelt, uint16_t usDuty){
    if(xPelt == DRV_PELT_1)
    {
        TIM2->CCR3 = usDuty;
    }
    else if(xPelt == DRV_PELT_2)
    {
        TIM2->CCR4 = usDuty;
    }
    else
    {
        return DRV_ERR_INVALID_PARAM;
    }
    
    return DRV_OK;
}

ReturnCode drv_pelt_init (void) {
    
    //Enable clocking of TIM2
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	
	//Enable clocking of GPIOB
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
	
    //Enable clocking of AFIO
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    
	//Configure GPIOB
    
    //PB6 and PB7 to output Push-Pull: 
    GPIOB->CRL |= GPIO_CRL_MODE6 | GPIO_CRL_MODE7; 
	GPIOB->CRL &= ~(GPIO_CRL_CNF6 | GPIO_CRL_CNF7);
    
    //PB10 and PB11 to alternete output Push-Pull:
	GPIOB->CRH |= GPIO_CRH_MODE10 | GPIO_CRH_MODE11; 
	GPIOB->CRH |= GPIO_CRH_CNF10_1 | GPIO_CRH_CNF11_1;
    GPIOB->CRH &= ~(GPIO_CRH_CNF10_0 | GPIO_CRH_CNF11_0);
    
    //Remap alternate functions PB10 and PB11 to TIM2 CH3 and CH4
    AFIO->MAPR |= AFIO_MAPR_TIM2_REMAP_1;
	
    //Configure TIM2 to PWM output

    //Enable auto-reload register preinstall
    TIM2->CR1 |= TIM_CR1_ARPE;
    
    //Set auto-reload register corresponds to resolution setuped by user
    TIM2->ARR = (1 << DRV_PELT_PWM_RES) - 1;
    
    
	//Setup prescaler 
    TIM2->PSC = DRV_PELT_PWM_PSC;
    
    //Set PWM mode 1 for 4,3 capture chanels
    TIM2->CCMR2 |= TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2;  
    
    //Enable preload register on TIM2->CCR3 and TIM2->CCR4
    TIM2->CCMR2 |= TIM_CCMR2_OC3PE | TIM_CCMR2_OC4PE;
    
    //Enable output pins
    TIM2->CCER |= TIM_CCER_CC3E | TIM_CCER_CC4E; 
    
    //Enable timer 
    TIM2->CR1 |= TIM_CR1_CEN;
    
	return DRV_OK;
}
