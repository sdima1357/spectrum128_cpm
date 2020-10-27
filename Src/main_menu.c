#include "main.h"
#include "ili9341.h"

int testTouch_dispatch(struct SYS_EVENT* ev);
int clock_dispatch(struct SYS_EVENT* ev);
int file_dispatch(struct SYS_EVENT* ev);
#define ITEMS_ON_SCREEN  3
#define strMenuArraySize 8

void StandbyMode(void)
{
  /* Enable Power Clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* Allow access to Backup */
  HAL_PWR_EnableBkUpAccess();

  /* Reset RTC Domain */
  __HAL_RCC_BACKUPRESET_FORCE();
  __HAL_RCC_BACKUPRESET_RELEASE();

  /* Disable all used wakeup sources: Pin1(PA.0) */
  HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);

  /* Clear all related wakeup flags */
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

  /* Re-enable all used wakeup sources: Pin1(PA.0) */
  HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);

  /*## Enter Standby Mode ####################################################*/
  /* Request to enter STANDBY mode  */
  HAL_PWR_EnterSTANDBYMode();
}

struct sMenuState
{
	int header_position;
	int header_position0;
	//int item_position;
	int lines_vpos[strMenuArraySize];
	int lines_vpos0[strMenuArraySize];
} MenuState;


char* strMenuArray [strMenuArraySize] =
{
		"  ZX48 ",
		" ZX128 ",
		"  MP3  ",
		" Paint ",
		" Calibr",
		" SysInf",
		" CLOCK ",
		" STB "
};

int nofrendo_main(int argc, char *argv[]);

#define LAST strMenuArraySize
int menu_dispatch(struct SYS_EVENT* ev)
{
	if(ev->message!=0)
	{
		//printf("dummy mess   = %x\r\n",ev->message);
	}
	if(ev->message==MESS_OPEN)
	{
		MenuState.header_position = 0;
		MenuState.header_position0 = 0;
		for(int i=0;i<strMenuArraySize;i++)
		{
			MenuState.lines_vpos0[i] = 0;
			MenuState.lines_vpos[i] = 0;
		}
		ev->message = MESS_REPAINT;
		return 1;
	}
	if(ev->message==MESS_REPAINT)
	{
		clearFullScreen();
		for(int k=0;k<ITEMS_ON_SCREEN;k++)
		{
			char numBuff[0x10];
			sprintf(numBuff,"%s",strMenuArray[k+MenuState.header_position0]);
			if(k+MenuState.header_position0 != MenuState.header_position)
			{
				LCD_Draw_Text(numBuff,k*LCD_getWidth()/ITEMS_ON_SCREEN,0, WHITE, 2,BLACK);
			}
			else
			{
				LCD_Draw_Text(numBuff,k*LCD_getWidth()/ITEMS_ON_SCREEN,0, BLACK, 2,WHITE);
			}
		}
		Delay(2);
		ev->message = MESS_IDLE;
		return 1;

	}
	else if(ev->message==MESS_KEYBOARD)
	{
		if(ev->param1==K_UP)
		{
			//if(MenuState.item_position>0)
			{
			//	MenuState.item_position--;
			}
		}
		else if(ev->param1==K_DOWN)
		{

		}
		else if(ev->param1==K_LEFT)
		{
			if(MenuState.header_position>0)
			{
				MenuState.header_position--;
				if(MenuState.header_position - MenuState.header_position0<0)
				{
					MenuState.header_position0 = MenuState.header_position;
				}
			}
		}
		else if(ev->param1==K_RIGHT)
		{
			if(MenuState.header_position<strMenuArraySize-1)
			{
				MenuState.header_position++;
				if(MenuState.header_position-MenuState.header_position0>ITEMS_ON_SCREEN-1)
				{
					MenuState.header_position0 =MenuState.header_position-ITEMS_ON_SCREEN+1;
				}
			}
		}
		else if(ev->param1==K_FIRE)
		{
			if(MenuState.header_position==4)
			{
				ReCalibrateTouch(1);
			}
			else  if(MenuState.header_position==3)
			{
				setCurrentFunc(&testTouch_dispatch);
				ev->message = MESS_OPEN;
				clearKey();
				return 1;
				//testTouch();
			}
			else  if(MenuState.header_position==2)
			{
				setCurrentFunc(&file_dispatch);
				ev->param1 = 2;
				ev->message = MESS_OPEN;
				clearKey();
				return 1;
				//testTouch();
			}
			else  if(MenuState.header_position==1)
			{
				setCurrentFunc(&file_dispatch);
				ev->param1 = 1;
				ev->message = MESS_OPEN;
				clearKey();
				return 1;
				//testTouch();
			}
			else  if(MenuState.header_position==0)
			{
				setCurrentFunc(&file_dispatch);
				ev->param1 = 0;
				ev->message = MESS_OPEN;
				clearKey();
				return 1;
				//testTouch();
			}
			else  if(MenuState.header_position==6)
			{
				setCurrentFunc(&clock_dispatch);
				ev->message = MESS_OPEN;
				clearKey();
				return 1;
			}
			else  if(MenuState.header_position==7)
			{
				//ili9341_DisplayOff();
				//StandbyMode();
				setCurrentFunc(&file_dispatch);
				ev->param1 = 3;
				ev->message = MESS_OPEN;
				clearKey();
				return 1;
			}

		}

		ev->message=MESS_REPAINT;
		clearKey();
		return 1;
	}
	else if(ev->message==MESS_IDLE)
	{
//		printf("dummy mess   = %x\r\n",ev->message);
//		printf("dummy param1 = %x\r\n",ev->param1);
//		printf("dummy param2 = %x\r\n",ev->param2);
		Delay(1);
	}
	else if(ev->message==MESS_CLOSE)
	{
		ev->message=MESS_REPAINT;
		return 1;
	}
	ev->message=MESS_IDLE;
    return 1;
}
