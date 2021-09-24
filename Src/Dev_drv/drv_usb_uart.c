#include "drv_usb_uart.h"

static uint8_t m_pucTxBuffer[DRV_BT_TX_BUFF_LEN] = {0};
static uint8_t m_ucTxBufferPointer = 0;
static uint8_t m_ucTxBufferLength  = 0;

static BTEventHandler_t m_xEventHandler;

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
        //Generate event that transmition is end and UART ready to transmit
        else
        {
            USART2->CR1 &= ~(USART_CR1_TXEIE);
            m_xEventHandler(BT_EVT_RESP_TX_END, NULL);
        }
    }
    //if transaction error detected, generate error event and block BT driver
    if(ulUartStatus & (USART_SR_PE | USART_SR_FE | USART_SR_NE | USART_SR_ORE))
    {
        //Set error bits
        m_eBTStatus |= ulUartStatus & (BT_ERR_UART_PE | BT_ERR_UART_FE | BT_ERR_UART_NE | BT_ERR_UART_ORE);
		
        //Clear buffer
        memset(pucRxBuffer, 0, DRV_BT_RX_BUFF_LEN);
        ucRxBufferPointer = 0;
		
        //Generate error event
        m_xEventHandler(BT_EVT_CMD_RX_ERR, (const BTStatus_t *)&m_eBTStatus);
        
    }
    if(ulUartStatus & USART_SR_RXNE)
	{
        uint16_t usRxSymbol = USART2->DR;
				
        // ignore input error not reset(cleared)
        if(m_eBTStatus != BT_OK)
            return;
				
        // ignore space symbol
        if(usRxSymbol == ' ')
            return;
        
        // ignore '\r' symbol
        if(usRxSymbol == '\r')
            return;
				
        // if '\n' symbol, send command to queue
        if(usRxSymbol == '\n')
        {
            // Generate "command is recived" event
            m_xEventHandler(BT_EVT_CMD_RX_END, pucRxBuffer);
			
            //Clear buffer
            memset(pucRxBuffer, 0, DRV_BT_RX_BUFF_LEN);
            ucRxBufferPointer = 0;
            return;
        }
        // if command length more than DRV_BT_RX_BUFF_LEN, generate error event and block BT driver
		if(ucRxBufferPointer == DRV_BT_RX_BUFF_LEN)
        {
			
            //Clear buffer
            memset(pucRxBuffer, 0, DRV_BT_RX_BUFF_LEN);
            ucRxBufferPointer = 0;
			
            //Generate error event
            m_xEventHandler(BT_EVT_CMD_RX_ERR, (const BTStatus_t *)&m_eBTStatus);
			
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

BTStatus_t drv_bt_read_status(){
    return m_eBTStatus;
}

void drv_bt_clear_status(){
    m_eBTStatus = BT_OK;
}
ReturnCode drv_bt_init(BTEventHandler_t xEventHandler){
    //Check is parameters are correct
    if(xEventHandler == NULL)
        return DRV_ERR_INVALID_PARAM;
		
    //save pointers
    m_xEventHandler = xEventHandler;
		
    //set good status
    m_eBTStatus = BT_OK;
		
    //Enable clocking of USART2
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
		
    //Enable clocking of GPIOA
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
		
    //Configure GPIOA: PA2(TX) to AF PP, PA3(RX) to IN FL for USART2
    GPIOA->CRL |= GPIO_CRL_MODE2_0; //set speed of PA2 to 10Mhz
    GPIOA->CRL &= ~GPIO_CRL_CNF2_0;
    GPIOA->CRL |= GPIO_CRL_CNF2_1 | GPIO_CRL_CNF3_0; 

    //Configure USART2

    //BAUD RATE calculating
    float 	 USARTDIV 		= PCLK1 / ((float)(DRV_BT_UART_BAUD_RATE * 16));
    uint16_t DIV_Mantissa   = (uint16_t)USARTDIV;
    uint8_t  DIV_Fraction   = (uint8_t)((USARTDIV - DIV_Mantissa) * 16.0f);
    USART2->BRR 	    	= (uint16_t)((DIV_Mantissa << 4UL) | DIV_Fraction); //set baud rate of USART2

    //Enable transmiter, reciver pins and RXNE, TXE interrupts
    USART2->CR1 |= USART_CR1_RXNEIE | USART_CR1_TE | USART_CR1_RE;

    //Configure parity check
#if DRV_BT_UART_PAR_CHECK == 1
    USART2->CR1 |= USART_CR1_M | USART_CR1_PCE | USART_CR1_PS;
#elif DRV_BT_UART_PAR_CHECK == 2
    USART2->CR1 |= USART_CR1_M | USART_CR1_PCE;
#endif	
    
    //Set stop bit count
#if DRV_BT_UART_STOP_BIT == 1
    USART2->CR2 &= ~USART_CR2_STOP;
#elif DRV_BT_UART_STOP_BIT == 2
    USART2->CR2 |= USART_CR2_STOP_1;
#endif
    // Enable EI(Error Interrupt)
    USART2->CR3 |= USART_CR3_EIE;
    //enable USART2
    USART2->CR1 |= USART_CR1_UE; 

    //Set prioryty of USART2 interupt
    NVIC_SetPriority(USART2_IRQn, 6);

    //Enable USART2 interupt
    NVIC_EnableIRQ(USART2_IRQn);

    return DRV_OK;
}
