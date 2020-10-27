#include "main.h"
#include "fatfs.h"
#include "ff_gen_drv.h"
#include "user_diskio.h"
#include "stm32_adafruit_sd.h"
#include "ili9341.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
char baseDirA[0x20];
int  rfilesA ;
char fnameBuff[0x100];
int  modeFile  = 0;
int  baseline = 0;
int  selection = 0;
int  invalidMemory = 0;
int z48_z128_dialog_dispatch(struct SYS_EVENT* ev);
int mp3_play_dispatch(struct SYS_EVENT* ev);
int z48_z128_dispatch(struct SYS_EVENT* ev);
int isDirtyMemory();
void DirtyMemoryClear();
int getModeFile()
{
	return modeFile;
}
int setModeFile(mode)
{
	modeFile = mode;
	return modeFile;
}
int isDirtyMemory()
{
	return invalidMemory;
}
void DirtyMemoryClear()
{
	invalidMemory = 0;
}
void DirtyMemorySet()
{
	invalidMemory = 1;
}

uint8_t * getRamBuffer0x4000(int k);
int getFileType(char* name);
void clearKey();


char* getFileName()
{
	return fnameBuff;
}
#define MAX_FILES 320
typedef uint8_t* p_uint8;
p_uint8* strListArray;
int      strCounter;
uint8_t* currAllock;
void initStrList()
{
	currAllock  = getRamBuffer0x4000(6);
	strListArray = currAllock;
	currAllock  += sizeof(p_uint8)*MAX_FILES;
	strCounter = 0;
}
void putStr(uint8_t attr,char* str)
{
	if(strCounter<MAX_FILES-1)
	{
		int rlen = strlen(str);
		strListArray[strCounter] = currAllock;
		currAllock += rlen+2;
		*strListArray[strCounter] = attr;
		memcpy(strListArray[strCounter]+1,str,rlen+1);
		strCounter++;
	}

}

int readDirIntoList(char* currentDir)
{
	FILINFO 	MyFileInfo;
	DIR 		MyDirectory;
	FRESULT 	res;
//	taskENTER_CRITICAL();
	res = f_opendir(&MyDirectory,currentDir);
	int files = 0;
	initStrList();
	if(res == FR_OK)
	{

		printf("open dir res ok\r\n");
		for (;;)
		{
			      res = f_readdir(&MyDirectory, &MyFileInfo);
			      if(res != FR_OK || MyFileInfo.fname[0] == 0)
				break;
			      //~ if(numFiles<LCD_getHeight()/CHAR_HEIGHT-2)
			      {
				     // if(files<NUM_LINES)
				      {
					      //strcpy(Lines[files].cacheLine,MyFileInfo.fname);
				    	 // printf("file:: %7d %x	%s \r\n",MyFileInfo.fsize,MyFileInfo.fattrib,MyFileInfo.fname);
				    	  int ft = getFileType(MyFileInfo.fname);
				    	  if(ft)
				    	  {
				    		  //putStr(MyFileInfo.fattrib,MyFileInfo.fname);
				    		  putStr(ft,MyFileInfo.fname);
				    	  }
					      files++;
				      }
				      //~ const uint16_t yScr = (240-192)/2-4;
				      //~ const uint16_t xScr = (320-256)/2-4;
				      //~ int lp = (snumFiles-baseline);
				      //~ if(lp>=0&&lp<192/8-1)
				      //~ {
						//~ LCD_Draw_Text(MyFileInfo.fname,xScr,yScr+(snumFiles-baseline)*8, GREEN, 1, (selection==snumFiles)?YELLOW:BLACK);
				      //~ }
			      }
		}
      }
	else if(res==FR_NO_FILESYSTEM)
	{
		printf("res==FR_NO_FILESYSTEM\r\n");
		//~ LCD_Draw_Text("format?",10,130, GREEN, 2, BLACK);
	}
	else if(res==FR_NO_PATH)
	{
		printf("res==FR_NO_PATH\r\n");
		//~ LCD_Draw_Text("format?",10,130, GREEN, 2, BLACK);
	}
	else
	{
		printf("res==%d???\r\n",res);
		//~ LCD_Draw_Text("open fail",10,130, GREEN, 2, BLACK);
	}
    f_closedir(&MyDirectory);
 //   taskEXIT_CRITICAL();
    return files;
}
int nes_dispatch(struct SYS_EVENT* ev);

int file_dispatch(struct SYS_EVENT* ev)
{
	const int NUmL =  192/8-1;
	if(ev->message)
	{
		//printf("file_dispatch  = %x\r\n",ev->message);
	}
	if(ev->message==MESS_OPEN)
	{
		modeFile = ev->param1;
		clearFullScreen();
		printf("get card init ok\r\n");


		BSP_SD_CardInfo CardInfo;
		BSP_SD_GetCardInfo(&CardInfo);
		printf("CardBlockSize: 		%d\r\n",CardInfo.CardBlockSize);
		printf("CardCapacity: 	%ld\r\n",CardInfo.CardCapacity);
		printf("LogBlockNbr:	%d\r\n",CardInfo.LogBlockNbr);
		printf("LogBlockSize:	%d\r\n",CardInfo.LogBlockSize);
		rfilesA = 0;
		f_mount(NULL,USERPath, 0);
		if(f_mount(&USERFatFS,USERPath, 0) != FR_OK)
		{
			printf("f_mount Error!\r\n");
			ev->message = MESS_CLOSE;
			return 0;
		}
		rfilesA = readDirIntoList(baseDirA);
		DirtyMemoryClear();
		printf("rfilesA==%d modeFile = %d\r\n",rfilesA,modeFile);

		/* Check the mounted device */
		if(modeFile==3)
		{
			strcpy(baseDirA,"/NES");
		}
		else if(modeFile==2)
		{
			strcpy(baseDirA,"/MP3");
		}
		else if (modeFile==1)
		{
			strcpy(baseDirA,"/Z128");
		}
		else if (modeFile==0)
		{
			strcpy(baseDirA,"/Z48");
		}
		rfilesA = readDirIntoList(baseDirA);
		DirtyMemoryClear();
		printf("rfilesA==%d modeFile = %d\r\n",rfilesA,modeFile);
		ev->message = MESS_REPAINT;
		return 1;
	}
	else if(ev->message==MESS_REPAINT)
	{
		if(isDirtyMemory())
		{
			ev->message=MESS_OPEN;
			DirtyMemoryClear();
			ev->param1 = modeFile;
			return 1;
		}
		else
		{
			int k;
			//LCD_fillRect(0,0,LCD_getWidth(),LCD_getHeight(),BLACK);
			//printf("paint_ rfilesA %d  selection %d \r\n",rfilesA,selection);
			if(selection >= rfilesA)
			{
				selection-=(selection-rfilesA)+1;
				baseline = selection- NUmL+1;
				if(baseline<0) baseline = 0;
			}
			//printf("paint_ rfilesA %d  selection %d \r\n",rfilesA,selection);
			for(k = 0;k <NUmL;k++)
			{
				int n = baseline + k;
				const uint16_t yScr = 0;
				const uint16_t xScr = 16;
				char buffer[0x21];
				buffer[0x20] = 0;
				memset(buffer,' ',0x20);
				char * type[] = {"0:","T:","Z:","M:","N:"};
				LCD_Draw_Text("  ",xScr-16,yScr+(k)*8, WHITE, 1, BLACK);
				if(n<rfilesA)
				{
					char * pnt = &strListArray[n][1];
					for(int p=0;p<0x20;p++)
					{
						if(*pnt)
						{
							buffer[p] = *pnt;
							pnt++;
						}
					}
					LCD_Draw_Text(type[strListArray[n][0]],xScr-16,yScr+(k)*8, (selection==n)?BLACK:WHITE, 1, (selection==n)?WHITE:BLACK);
				}
				LCD_Draw_Text(buffer,xScr,yScr+(k)*8, (selection==n)?BLACK:WHITE, 1, (selection==n)?WHITE:BLACK);

			}
			Delay(100);
			clearKey();
			ev->message = MESS_IDLE;
			char* fname = (char*)(strListArray[selection]+1);
			//printf("paint_ok rfilesA %d  selection %d %s\r\n",rfilesA,selection,fname);
			return 1;
		}
	}
	else if(ev->message==MESS_IDLE)
	{
		return 1;
	}
	else if(ev->message==MESS_KEYBOARD)
	{
		if(ev->param1==K_ESC)
		{

			ev->message = MESS_CLOSE;
			return 0;
		}
		else if(ev->param1==K_DOWN)
		{
			if(selection<rfilesA-1)
			{
				selection++;
			}
			if(selection-NUmL+1>baseline)
			{
				baseline++;
			}
			ev->message = MESS_REPAINT;
			return 0;
		}
		else if(ev->param1==K_UP)
		{
			if(selection>0)
			{
				selection--;
			}
			if(baseline>selection)
			{
				baseline--;
			}
			ev->message = MESS_REPAINT;
			return 0;
		}
		else if(ev->param1==K_FIRE) //fire
		{
			char* fname = (char*)(strListArray[selection]+1);
			printf("selected filename %s\r\n",strListArray[selection]+1);
			//selNum = selection;
			//make full filename
			strcpy(fnameBuff,baseDirA);
			strcpy(fnameBuff+strlen(fnameBuff),"/");
			strcpy(fnameBuff+strlen(fnameBuff),fname);

			if((modeFile==0)||(modeFile==1))
			{
				setCurrentFunc(&z48_z128_dispatch);
			}
			else if(modeFile==3)
			{
				//nes loop;
				setCurrentFunc(&nes_dispatch);
			}
			else
			{
				setCurrentFunc(&mp3_play_dispatch);
			}
			ev->message = MESS_OPEN;
			return 0;
		}
		ev->message = MESS_IDLE;
		return 1;
	}
	ev->message = MESS_IDLE;
	return 1;
}

