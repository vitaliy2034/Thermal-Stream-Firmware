#ifndef SP_BT_TASK__
#define SP_BT_TASK__

#include "stm32f10x.h"

int16_t sSPSplitDigits(int32_t * plDigArr, uint8_t * pucDataArr, 
											 int8_t cSplitChar, int8_t cEndChar, 
											 uint16_t usDigCnt, uint16_t usBuffLen);


#endif //SP_BT_TASK__