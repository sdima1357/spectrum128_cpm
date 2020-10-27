#include "stm32f4xx_hal.h"
#include "main.h"
#include <string.h>
#include "ili9341.h"
extern RTC_HandleTypeDef hrtc;
int oldSec = -1;
int clock_dispatch(struct SYS_EVENT* ev)
{
	if(ev->message==MESS_OPEN)
	{
		clearFullScreen();
		ev->message = MESS_IDLE;
		return 1;
	}
	else if(ev->message==MESS_IDLE)
	{
		RTC_TimeTypeDef sTime;
		RTC_DateTypeDef Date;
		HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BCD);
		HAL_RTC_GetDate(&hrtc, &Date, RTC_FORMAT_BCD);
		//mprintf("%02x:%02x:%02x\r\n",sTime.Hours,sTime.Minutes,sTime.Seconds);
		if(oldSec!=sTime.Seconds)
		{
			char buffer[0x20];
			sprintf(buffer,"%02x:%02x:%02x",sTime.Hours,sTime.Minutes,sTime.Seconds);
			LCD_Draw_Text2(buffer,(LCD_getWidth()-strlen(buffer)*4*8)/2,LCD_getHeight()/2-16*4, GREEN, 4,16,BLACK);
			sprintf(buffer,"%02x %02x",Date.Date,Date.Month);
			LCD_Draw_Text(buffer,(LCD_getWidth()-strlen(buffer)*4*8)/2,0, GREEN, 4,BLACK);
			oldSec=sTime.Seconds;
			Delay(2);
		}
		Delay(2);
		return 1;
	}
	else if(ev->message==MESS_KEYBOARD)
	{
		if(ev->param1==K_ESC)
		{
			ev->message = MESS_CLOSE;
			return 0;
		}
		ev->message = MESS_IDLE;
		return 1;
	}
	else
	{
		ev->message = MESS_IDLE;
	}
	return 1;
}
