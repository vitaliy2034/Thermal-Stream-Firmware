#include "drv_pump.h"

ReturnCode drv_pump_start(){
    GPIOB->BSRR |= GPIO_BSRR_BR8;
    return DRV_OK;
}

ReturnCode drv_pump_stop(){
    GPIOB->BSRR |= GPIO_BSRR_BS8;
    return DRV_OK;
}
ReturnCode drv_pump_init() {
    
	//Configure GPIOB
    
	//Enable clocking of GPIOB
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    
    //PB8 to output Push-Pull: 
    GPIOB->CRH |= GPIO_CRH_MODE8; 
	GPIOB->CRH &= ~GPIO_CRH_CNF8;
    
	return DRV_OK;
}
