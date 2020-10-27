/*
 * mp3play.c
 *
 *  Created on: Jan 29, 2019
 *      Author: gpu
 */
#include "pub/mp3dec.h"
#include "pub/mp3common.h"
#include <ff.h>
#include <string.h>
#include "main.h"
#include "ili9341.h"
uint16_t scanJSK();
uint8_t * getRamBuffer0x4000(int k);
void push_pair(uint16_t,uint16_t);
int waitFor();

//#define 	BUFFSZ 0x1000
#define 	BUFFSZ 0x1000
static      int volume = MAX_VOLUME;


struct sMP3_PL
{
	HMP3Decoder  decoder;
	uint8_t*	 buffer;
	int16_t*	 out_buffer;
	uint8_t* 	 inbuf;
	FIL 		 MyFile;
	int 		 bytesLeft;
	int 		 cont;
	int 		 pos;
	float 		 timeLogSec;
	MP3FrameInfo mp3FrameInfo;
	uint32_t 	 bytesPlayed;
	int  printLog;
}MP3_PL;
#include "stm32f4xx_hal.h"
#include "main.h"
#include "ili9341.h"
char* getFileName();

int mp3_play_dispatch(struct SYS_EVENT* ev)
{
	if(ev->message==MESS_OPEN)
	{
		LCD_fillRect(0,0,LCD_getWidth(),LCD_getHeight(),BLACK);
		if(mp3_playOpen(getFileName()))
		{
			ev->message = MESS_IDLE;
			return 1;
		}
		else
		{
			cleanAudioBuffer();
			ev->message = MESS_CLOSE;
			return 1;
		}
	}
	else if(ev->message==MESS_IDLE)
	{
		if(mp3_cont())
		{

		}
		else
		{
			f_close(&MP3_PL.MyFile);
			cleanAudioBuffer();
			ev->message = MESS_CLOSE;
		}
		return 1;
	}
	else if(ev->message==MESS_KEYBOARD)
	{
		if(ev->param1==K_ESC)
		{
			f_close(&MP3_PL.MyFile);
			cleanAudioBuffer();
			ev->message = MESS_CLOSE;
			return 0;
		}
		else if(ev->param1==K_UP)
		{
			setVolume0(getVolume0()+1);
			//volume+=100;
			//if(volume>MAX_VOLUME) volume= MAX_VOLUME;
		}
		else if(ev->param1==K_DOWN)
		{
			setVolume0(getVolume0()-1);
			//volume-= 100;
			//if(volume<100) volume= 100;
		}
		ev->message = MESS_IDLE;
		clearKey();
		return 1;
	}
	else
	{
		ev->message = MESS_IDLE;
	}
	return 1;
}
size_t read_file (FIL  *file, uint8_t *buf, uint32_t sizeofbuf,int * bytes)
{
	size_t BytesReadfile=10 ;
	int realRead = 0;
	int bts = sizeofbuf;
	while(BytesReadfile&&bts>0)
	{
		BytesReadfile = 0;
		int res = f_read (file, buf , (bts>512)?512:bts, &BytesReadfile);
		realRead+=BytesReadfile;
		buf  += BytesReadfile;
		bts -= BytesReadfile;
	}
	*bytes = realRead;
    return realRead;
}



int mp3_playOpen(char* filename)
{
	MP3_PL.cont = 1;
	MP3_PL.bytesPlayed = 0;
    MP3_PL.buffer = getRamBuffer0x4000(0);
    MP3_PL.out_buffer= MP3_PL.buffer+BUFFSZ*2;
	MP3_PL.decoder = MP3InitDecoder();
    if(!f_open(&MP3_PL.MyFile,(TCHAR*)filename, FA_READ)==FR_OK)
    {
    	printf("error open file %s\r\n",filename);
    	return 0;
    }
    UINT bytes;
    read_file(&MP3_PL.MyFile,MP3_PL.buffer,BUFFSZ*2,&bytes);
    MP3_PL.bytesLeft = bytes;
    MP3_PL.bytesPlayed += bytes;
    MP3_PL.inbuf =  MP3_PL.buffer;
	MP3_PL.pos = 0;
	MP3_PL.cont = 1;
	MP3_PL.pos = MP3FindSyncWord(MP3_PL.buffer,MP3_PL.bytesLeft);
	while(MP3_PL.pos<0)
	{
		read_file(&MP3_PL.MyFile,MP3_PL.buffer,BUFFSZ*2,&bytes);
		MP3_PL.pos = MP3FindSyncWord(MP3_PL.buffer,MP3_PL.bytesLeft);
		MP3_PL.bytesPlayed += bytes;
		if(bytes==0)
		{
			f_close(&MP3_PL.MyFile);
			printf("not mp3 file??\r\n");
			return 0;
		}
	}
	MP3_PL.inbuf     += MP3_PL.pos;
	MP3_PL.bytesLeft-=  MP3_PL.pos;
	printf(" pos = %d\r\n",MP3_PL.pos);
	MP3_PL.printLog = 1;
	return 1;
}
int mp3_cont()
{
    UINT bytes;
	if(MP3_PL.bytesLeft)
	{
		//~ if(!ifs)
		//~ {
			//~ PRINT("error read");
			//~ bytesLeft = 0;
			//~ cont = false;
		//~ }
		if(MP3_PL.bytesLeft<BUFFSZ&&MP3_PL.cont)
		{
			memcpy(MP3_PL.buffer,MP3_PL.inbuf,MP3_PL.bytesLeft);

			//ifs.read((char*)&buff[bytesLeft],BUFFSZ);
			read_file(&MP3_PL.MyFile,MP3_PL.buffer+MP3_PL.bytesLeft,BUFFSZ,&bytes);
			MP3_PL.bytesPlayed += bytes;
			if(!bytes)
			{
				MP3_PL.cont =  0;
			}
			else
			{
				MP3_PL.bytesLeft+=bytes;
				MP3_PL.inbuf = MP3_PL.buffer;
			}
		}
		MP3_PL.pos = MP3FindSyncWord(MP3_PL.inbuf,MP3_PL.bytesLeft);
//		mprintf(" pos = %d\r\n",MP3_PL.pos);
		if(MP3_PL.pos>=0)
		{
			MP3_PL.inbuf     += MP3_PL.pos;
			MP3_PL.bytesLeft -= MP3_PL.pos;
			int res = MP3GetNextFrameInfo(MP3_PL.decoder,&MP3_PL.mp3FrameInfo,MP3_PL.inbuf);
			if(res>=0)
			{
				if(MP3_PL.printLog)
				{
					printf(" mp3FrameInfo.bitrate = %d\r\n",MP3_PL.mp3FrameInfo.bitrate);
					printf(" mp3FrameInfo.nChans = %d\r\n",MP3_PL.mp3FrameInfo.nChans);
					printf(" mp3FrameInfo.samprate = %d\r\n",MP3_PL.mp3FrameInfo.samprate);
					printf(" mp3FrameInfo.bitsPerSample = %d\r\n",MP3_PL.mp3FrameInfo.bitsPerSample);
					printf(" mp3FrameInfo.outputSamps = %d\r\n",MP3_PL.mp3FrameInfo.outputSamps);
					printf(" mp3FrameInfo.layer = %d\r\n",MP3_PL.mp3FrameInfo.layer);
					MP3_PL.printLog = 0;
				}
				int dres = MP3Decode(MP3_PL.decoder,&MP3_PL.inbuf,&MP3_PL.bytesLeft,MP3_PL.out_buffer,0);
				if(dres>=0)
				{
					int timewait = waitFor();
					int k;
					MP3_PL.timeLogSec+=(1.0f/MP3_PL.mp3FrameInfo.samprate)*MP3_PL.mp3FrameInfo.outputSamps/MP3_PL.mp3FrameInfo.nChans;
#ifdef DAC_OUTPUT
					//DAC output 12 bit =  0-4095
					if(MP3_PL.mp3FrameInfo.nChans==2)
					{
						for(k=0;k<MP3_PL.mp3FrameInfo.outputSamps/2;k++)
						{
							push_pair((MP3_PL.out_buffer[k*2+0]+0x8000)*4096>>16,(MP3_PL.out_buffer[k*2+0]+0x8000)*4096>>16);
						}
					}
					else
					{
						for(k=0;k<MP3_PL.mp3FrameInfo.outputSamps;k++)
						{
							push_pair((MP3_PL.out_buffer[k]+0x8000)*4096>>16,(MP3_PL.out_buffer[k]+0x8000)*4096>>16);
						}
					}
#else
					//PWM output 10.89 bit  0-1903
					if(MP3_PL.mp3FrameInfo.nChans==2)
					{
						for(k=0;k<MP3_PL.mp3FrameInfo.outputSamps/2;k++)
						{
							//push_pair((out_buffer[k*2+0]+0x8000)>>6,(out_buffer[k*2+0]+0x8000)>>6);
							push_pair((MP3_PL.out_buffer[k*2+0]+0x8000)*volume>>16,(MP3_PL.out_buffer[k*2+1]+0x8000)*volume>>16);
						}
						//printf("MP3_PL.mp3FrameInfo.outputSamps = %d\n",MP3_PL.mp3FrameInfo.outputSamps);
					}
					else
					{
						for(k=0;k<MP3_PL.mp3FrameInfo.outputSamps;k++)
						{
							push_pair((MP3_PL.out_buffer[k]+0x8000)*volume>>16,(MP3_PL.out_buffer[k]+0x8000)*volume>>16);
						}
					}
#endif

					MP3GetLastFrameInfo(MP3_PL.decoder,&MP3_PL.mp3FrameInfo);

					//mprintf("wait %02d ms %0.2f s\r",timewait,MP3_PL.timeLogSec);

					char numBuff[0x40];
					sprintf(numBuff,"bytes %d wait= %04d ms %0.2f",MP3_PL.bytesPlayed,timewait,MP3_PL.timeLogSec);
					LCD_Draw_Text(numBuff,24,100, GREEN, 1,BLACK);

				}
			}
			else
			{
				MP3_PL.pos = MP3FindSyncWord(MP3_PL.buffer,MP3_PL.bytesLeft);
				MP3_PL.inbuf    += MP3_PL.pos;
				MP3_PL.bytesLeft-= MP3_PL.pos;
				printf("MP3GetNextFrameInfo error bytesLeft = %d\r\n",MP3_PL.bytesLeft);
				if(MP3_PL.pos==0)
				{
					MP3_PL.bytesLeft = 0;
					MP3_PL.cont = 0;
				}
			}

		}
		else
		{
			printf("MP3FindSyncWord error bytesLeft = %d\r\n",MP3_PL.bytesLeft);
			MP3_PL.bytesLeft = 0;
			MP3_PL.cont = 0;
		}
	return 1;
	}
	return 0;
}
