#include "sp_bt_task.h"


//fix tecnical debt
int16_t sSPSplitDigits(int32_t * plDigArr, uint8_t * pucDataArr, int8_t cSplitChar, int8_t cEndChar, uint16_t usDigCnt, uint16_t usBuffLen)
{
    if(pucDataArr[0] == '\0' || pucDataArr[0] == '\n')
    {
        return 0;
    }
    for(uint8_t i = 0; (pucDataArr[i] != '\0' && pucDataArr[i] != '\n') ; i++)
	{		
        if(pucDataArr[i] == '-' && i == 0)
            continue;
        //convert ASCII to digit
        if(0x3A > pucDataArr[i]  && pucDataArr[i] >= 0x30)
        {
            plDigArr[0] = (plDigArr[0] * 10) + (pucDataArr[i] - 0x30);
		}
        //if hare - there is error
		else
		{
            return -1;
		}
	}
    
    return 1;
}