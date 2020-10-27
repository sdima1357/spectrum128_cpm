/*
 * cpm_machine.c
 *
 *  Created on: Oct 25, 2020
 *      Author: dima
 */
#include "z80.h"
#include "main.h"
#include "ili9341.h"
#include "fonts.h"

extern int32_t tstates;
extern int32_t interrupts_enabled_at;
#define BSIZE 0x100
uint8_t RBUFFER[BSIZE];
__IO ITStatus UartReadyT = RESET;

volatile int  RHEAD=0;
volatile int  RTAIL=0;
//volatile int  ascii_c;

void rclear()
{
	RHEAD=0;
	RTAIL=0;
}

void rput(uint8_t data)
{
	RBUFFER[RHEAD] = data;
	RHEAD = (RHEAD+1)&(BSIZE-1);
}
uint8_t rget()
{
	uint8_t res = RBUFFER[RTAIL];
	RTAIL = (RTAIL+1)&(BSIZE-1);
	return res;
}
int rsize()
{
	return (RHEAD+BSIZE-RTAIL)&(BSIZE-1);
}

#include "fonts.h"
void LCD_Draw_Char2Font(sFONT* font,char Character, int16_t X, int16_t Y, uint16_t Colour, uint16_t SizeX,uint16_t SizeY, uint16_t Background_Colour)
{
	//~ flagReinit = 1;
		uint8_t 	function_char;
    int16_t 	i,j;

	function_char = Character;

        if (function_char < 0x20)
	{
		Character = 0;
	}
	else
	{
		function_char -= 32;
	}

	char temp[font->Width];
	uint8_t k;
	for( k = 0; k<font->Width; k++)
	{
		temp[k] = font->table[function_char*font->Width+k];
	}

    // Draw pixels
	LCD_fillRect(X, Y, font->Width*SizeX, font->Height*SizeY, Background_Colour);

	int rw = (font->Width+7)/8;
	for (j=0; j<font->Height; j++)
	{
		uint8_t* pnt =  font->table+j*rw+function_char*font->Height*rw;
		for (i=0; i<font->Width; i++)
		{
			uint8_t bt = pnt[i/8];
			if(bt&(1<<(7-(i&7))))
			{
				LCD_fillRect(X+(i*SizeX), Y+(j*SizeY), SizeX,SizeY, Colour);
			}
		}
	}
/*

    for (j=0; j<font->Width; j++) {
        for (i=0; i<font->Height; i++) {
            if (temp[i] & (1<<(font->Width-1-j))) {
								//~ LCD_fillRect(X+(j*Size), Y+(i*Size), Size,Size, Colour);
							LCD_fillRect(X+(j*SizeX), Y+(i*SizeY), SizeX,SizeY, Colour);
            }
        }
    }
   */
}
void  LCD_Draw_TextFont(sFONT* font,const char* Text, int16_t X, int16_t Y, uint16_t Colour, uint16_t Size, uint16_t Background_Colour)
{
    while (*Text) {
        LCD_Draw_Char2Font(font,*Text, X, Y, Colour, Size,Size, Background_Colour);
        X += font->Width*Size;
	Text++;
    }
}
void  LCD_Draw_Text2Font(sFONT* font,const char* Text, int16_t X, int16_t Y, uint16_t Colour, uint16_t SizeX, uint16_t SizeY,uint16_t Background_Colour)
{
    while (*Text) {
        LCD_Draw_Char2Font(font,*Text, X, Y, Colour, SizeX,SizeY, Background_Colour);
        X += font->Width*SizeX;
	Text++;
    }
}

#define BELL 0x7
#define BS   0x8
// form feed
#define FF   0xc
#define DEL  0x7f

// \n
#define NL   0xa

// \r
#define CR   0xd


#define CTRL_A  1
#define CTRL_S  0x13
#define CTRL_D  0x4
#define CTRL_F  0x6
#define CTRL_G  0x7

#define CTRL_H   0x8
#define BSPACE   0x8

#define CTRL_J  0xA
#define CTRL_K  0xB
#define CTRL_L  0xC
#define CTRL_Q  0x11
#define CTRL_W  0x17
#define CTRL_E  0x5
#define CTRL_R  0x12
#define CTRL_T  0x14
#define CTRL_Y  0x19
#define CTRL_U  0x15
#define CTRL_I  0x9
#define CTRL_O  0xf
#define CTRL_P  0x10


#define CTRL_Z  0x1a
#define CTRL_X  0x18
#define CTRL_C  0x03
#define CTRL_V  0x16
#define CTRL_B  0x02
#define CTRL_N  0x0e
#define CTRL_M  0xd





#if 0
struct TB
{
	int FW;
	int FH;
	int BW;
	int BH;
	uint8_t * Buffer;
	uint8_t * BufferShadow;
	int cursorX;
	int cursorY;
	int bDirty;

}TBD;

void setTermBuffer(int orient,int fontW,int fontH)
{
	//LCD_setRotation(LANDSCAPE_FLIP);
	//LCD_setRotation(PORTRAIT);
	//LCD_setRotation(LANDSCAPE);
	TBD.FW = fontW;//CHAR_WIDTH;
	TBD.FH = fontH;//CHAR_HEIGHT;
	//LCD_setRotation(orient);
	//LCD_setRotation(PORTRAIT);
	TBD.BW =LCD_getWidth()/(TBD.FW*CHAR_WIDTH);
	TBD.BH =LCD_getHeight()/(TBD.FH*CHAR_HEIGHT);
	/*
    if(TBD.Buffer)
    {
    	free(TBD.Buffer);
    }
    if(TBD.BufferShadow)
    {
    	free(TBD.BufferShadow);
    }
    */
	TBD.Buffer = malloc(TBD.BW*TBD.BH) ;
	TBD.BufferShadow = malloc(TBD.BW*TBD.BH) ;
	TBD.cursorX = 0;
	TBD.cursorY = 0;

	printf("TBD.BW = %d,TBD.BH=%d\n",TBD.BW,TBD.BH);

	memset(TBD.Buffer,0,TBD.BW*TBD.BH);
	memset(TBD.BufferShadow,1,TBD.BW*TBD.BH);
}


uint8_t buff[17];
int     mode = 0;
int oldX;
int oldY;

int first = 10;




void putCharCurs(uint8_t ch)
{
	if(first>0)
	{
		printf("%x %x %x\n",TBD.cursorX,TBD.cursorY,ch);
		first--;
	}
	if(ch==27)
	{
		if( mode!=0 )
		{
			mode = 0;
			return;
		}
		else
		{
			mode = 1;
			return ;
		}
	}
	if(mode)
	{
		if(mode<16)
		{
			int X;
			int Y;
			char ch0;
			buff[mode-1] = ch;
			buff[mode]   = 0;
			if(strcmp(buff,"[H")==0)
			{
				TBD.cursorX =0;
				TBD.cursorY =0;
				mode = 0;
				return ;
			}
			else if(strcmp(buff,"[K")==0)
			{
				TBD.bDirty = 1;
				for(int k=TBD.cursorX;k<TBD.BW;k++)
				{
					TBD.Buffer[TBD.cursorY*TBD.BW+k] = 0;
				}
				mode = 0;
				return ;
			}
			else if(strcmp(buff,"[s")==0)
			{
				oldX = 	TBD.cursorX;
				oldY = 	TBD.cursorY;
				mode = 0;
				return ;
			}
			else if(strcmp(buff,"[u")==0)
			{
				TBD.cursorX = oldX;
				TBD.cursorY = oldY;
				mode = 0;
				return ;
			}
			else if(strcmp(buff,"[2J")==0)
			{
				TBD.bDirty = 1;
				TBD.cursorX = 0;
				TBD.cursorY = 0;
				memset(TBD.Buffer,0,TBD.BW*TBD.BH);
				mode = 0;
				return ;
			}
			else if(strcmp(buff,"[0J")==0)
			{
				TBD.bDirty = 1;
				int pos = TBD.cursorY*TBD.BW+TBD.cursorX;
				memset(TBD.Buffer+pos,0,TBD.BW*TBD.BH-pos);
				mode = 0;
				return ;
			}
			else if(strcmp(buff,"[J")==0)
			{
				TBD.bDirty = 1;
				int pos = TBD.cursorY*TBD.BW+TBD.cursorX;
				memset(TBD.Buffer+pos,0,TBD.BW*TBD.BH-pos);
				mode = 0;
				return ;
			}
			else if(strcmp(buff,"[L")==0)
			{
				TBD.bDirty = 1;
				int pos = TBD.cursorY*TBD.BW;//+TBD.cursorX;
				for(int k=pos;k<TBD.BW*(TBD.BH-1);k++)
				{
						TBD.Buffer[k] = TBD.Buffer[k+TBD.BW];
				}
				memset(TBD.Buffer+TBD.BW*(TBD.BH-1),0,TBD.BW);
				mode = 0;
				return ;
			}
			else if(ch=='A' && sscanf(buff,"[%dA",&X)==2)
			{
				TBD.bDirty = 1;
				TBD.cursorY -= X;
				mode = 0;
				return ;
			}
			else if(ch=='B' && sscanf(buff,"[%dB",&X)==1)
			{
				TBD.bDirty = 1;
				//int pos = TBD.cursorY*TBD.BW+TBD.cursorX;
				//memset(TBD.Buffer+pos,0,TBD.BW*TBD.BH-pos);
				TBD.cursorY += X;
				mode = 0;
				return ;
			}
			else if(ch=='C' && sscanf(buff,"[%dC",&X)==1)
			{
				TBD.bDirty = 1;
				//int pos = TBD.cursorY*TBD.BW+TBD.cursorX;
				//memset(TBD.Buffer+pos,0,TBD.BW*TBD.BH-pos);
				TBD.cursorX += X;
				mode = 0;
				return ;
			}
			else if(ch=='D' && sscanf(buff,"[%dD",&X)==1)
			{
				TBD.bDirty = 1;
				//int pos = TBD.cursorY*TBD.BW+TBD.cursorX;
				//memset(TBD.Buffer+pos,0,TBD.BW*TBD.BH-pos);
				TBD.cursorX -= X;
				mode = 0;
				return ;
			}
			else if(ch=='H'  && sscanf(buff,"[%d;%dH",&X,&Y)==2)
			{
				TBD.bDirty = 1;
				//int pos = TBD.cursorY*TBD.BW+TBD.cursorX;
				//memset(TBD.Buffer+pos,0,TBD.BW*TBD.BH-pos);
				TBD.cursorY = X?(X-1):0;
				TBD.cursorX = Y?(Y-1):0;
				mode = 0;
				return ;
			}
			mode++;
			return ;
		}
	}

	if(ch=='\r')
	{
		return ;
		//ch = '\n' ;
	}
	if(ch==FF)
	{
		TBD.bDirty = 1;
		TBD.cursorX = 0;
		TBD.cursorY = 0;
		memset(TBD.Buffer,0,TBD.BW*TBD.BH);
		return ;
		//ch = '\n' ;
	}
	TBD.bDirty = 1;
	int pos = TBD.cursorY*TBD.BW+TBD.cursorX;
	if(ch!='\n')
	{
		TBD.Buffer[pos] = ch;
		TBD.cursorX++;
	}
	if(TBD.cursorX>=TBD.BW||ch=='\n')
	{
		TBD.cursorY++;
		TBD.cursorX = 0;
		if(TBD.cursorY>=TBD.BH)
		{
			for(int k=0;k<TBD.BW*(TBD.BH-1);k++)
			{
					TBD.Buffer[k] = TBD.Buffer[k+TBD.BW];
			}
			TBD.cursorY = TBD.BH -1;
			memset(TBD.Buffer+TBD.BW*(TBD.BH-1),0,TBD.BW);
		}
	}
}
void paint()
{
	 if(TBD.Buffer)
	 {
		 if(TBD.bDirty)
		 {
			// toDispl();
			 for(int y=0;y<TBD.BH;y++)
			 {
				 for(int x=0;x<TBD.BW;x++)
				 {
					 int pos = y*TBD.BW+x;
					 //LCD_Draw_Char2(TBD.Buffer[pos], x*TBD.FW*CHAR_WIDTH, y*TBD.FH*CHAR_HEIGHT,WHITE, TBD.FW,TBD.FH,BLACK);
					 uint8_t fg[3] = {255,255,0};
					 //uint16_t
					// LCD_Draw_Char2(TBD.Buffer[pos], x*TBD.FW*CHAR_WIDTH, y*TBD.FH*CHAR_HEIGHT,color_convertRGB_to16(fg), TBD.FW,TBD.FH,BLUE);
					 //LCD_Draw_Char2(TBD.Buffer[pos], x*TBD.FW*CHAR_WIDTH, y*TBD.FH*CHAR_HEIGHT,YELLOW, TBD.FW,TBD.FH,color_convertRGB_to16(fg));
					 //LCD_Draw_Char2(TBD.Buffer[pos], x*TBD.FW*CHAR_WIDTH, y*TBD.FH*CHAR_HEIGHT,YELLOW, TBD.FW,TBD.FH,BLUE);

					 if(TBD.Buffer[pos]!= TBD.BufferShadow[pos])
					 {
						 TBD.BufferShadow[pos] = TBD.Buffer[pos];
						 LCD_Draw_Char2Font(&Font4x8,TBD.Buffer[pos], x*TBD.FW*CHAR_WIDTH, y*TBD.FH*CHAR_HEIGHT,GREEN, TBD.FW,TBD.FH,BLACK);
					 }
					 if(y==TBD.cursorY && x==TBD.cursorX)
					 {
						 TBD.BufferShadow[pos] = -1;
						 LCD_Draw_Char2Font(&Font4x8,TBD.Buffer[pos], x*TBD.FW*CHAR_WIDTH, y*TBD.FH*CHAR_HEIGHT,BLACK, TBD.FW,TBD.FH,GREEN);
					 }
				 }
			 }
			 //LCD_fillRect(TBD.cursorX*TBD.FW*CHAR_WIDTH,TBD.cursorY*TBD.FH*CHAR_HEIGHT,)
			 TBD.bDirty = 0;
			// toADC();
		 }
	 }
}
#else
struct TB
{
	int FW;
	int FH;
	int BW;
	int BH;
	uint8_t * Buffer;
	uint8_t * BufferShadow;

	uint8_t   currAttribute;

	int cursorX;
	int cursorY;
	int bDirty;

}TBD;

void setTermBuffer(int orient,int fontW,int fontH)
{
	//LCD_setRotation(LANDSCAPE_FLIP);
	//LCD_setRotation(PORTRAIT);
	//LCD_setRotation(LANDSCAPE);
	TBD.FW = fontW;//CHAR_WIDTH;
	TBD.FH = fontH;//CHAR_HEIGHT;
	//LCD_setRotation(orient);
	TBD.BW =LCD_getWidth()/(TBD.FW*CHAR_WIDTH);
	TBD.BH =LCD_getHeight()/(TBD.FH*CHAR_HEIGHT);
	TBD.currAttribute = 0;
	/*
    if(TBD.Buffer)
    {
    	free(TBD.Buffer);
    }
    if(TBD.BufferShadow)
    {
    	free(TBD.BufferShadow);
    }
    */
	TBD.Buffer =(uint8_t *) malloc(TBD.BW*TBD.BH*2) ;
	TBD.BufferShadow = (uint8_t *) malloc(TBD.BW*TBD.BH*2) ;
	TBD.cursorX = 0;
	TBD.cursorY = 0;

	printf("TBD.BW = %d,TBD.BH=%d\n",TBD.BW,TBD.BH);

	memset(TBD.Buffer,0,TBD.BW*TBD.BH*2);
	memset(TBD.BufferShadow,1,TBD.BW*TBD.BH*2);
}


char buff[17];
int     mode = 0;
int oldX;
int oldY;

void LCD_Draw_Char2(char Character, int16_t X, int16_t Y, uint16_t Colour, uint16_t SizeX,uint16_t SizeY, uint16_t Background_Colour);


void putCharCurs(uint8_t ch)
{
	if(ch==27)
	{
		if( mode!=0 )
		{
			mode = 0;
			return;
		}
		else
		{
			mode = 1;
			return ;
		}
	}
	if(mode)
	{
		if(mode<16)
		{
			int X;
			int Y;
			char ch0;
			buff[mode-1] = ch;
			buff[mode]   = 0;
			if(isalpha(ch))
			{
				//printf("ESC %s\n",buff);
			}
			if(ch=='m')
			{

				if(strcmp(buff,"[0m")==0||strcmp(buff,"[m")==0)
				{
					TBD.currAttribute = 0;
					mode = 0;
				}
				else if(strcmp(buff,"[1m")==0)
				{
					TBD.currAttribute = 0x10;
					mode = 0;
				}
				else if(strcmp(buff,"[7m")==0)
				{
					TBD.currAttribute = 0x30;
					mode = 0;
				}
				mode = 0;
				return ;
			}
			else if(ch=='H')
			{
				if(strcmp(buff,"[H")==0)
				{
					TBD.bDirty   = 1;
					TBD.cursorX = 0;
					TBD.cursorY = 0;
					mode = 0;
					return ;
				}
				else if(sscanf(buff,"[%d;%dH",&X,&Y)==2)
				{
					TBD.bDirty = 1;
					//int pos = TBD.cursorY*TBD.BW+TBD.cursorX;
					//memset(TBD.Buffer+pos,0,TBD.BW*TBD.BH-pos);
					TBD.cursorY = X?(X-1):0;
					TBD.cursorX = Y?(Y-1):0;
					mode = 0;
					return ;
				}
			}
			else if(ch=='f')
			{
				if(sscanf(buff,"[%d;%df",&X,&Y)==2)
				{
					TBD.bDirty = 1;
					//int pos = TBD.cursorY*TBD.BW+TBD.cursorX;
					//memset(TBD.Buffer+pos,0,TBD.BW*TBD.BH-pos);
					TBD.cursorY = X?(X-1):0;
					TBD.cursorX = Y?(Y-1):0;
					mode = 0;
					return ;
				}
			}
			else if(ch=='J')
			{
				if(strcmp(buff,"[2J")==0)
				{
					TBD.bDirty = 1;
					TBD.cursorX = 0;
					TBD.cursorY = 0;
					memset(TBD.Buffer,0,TBD.BW*TBD.BH*2);
					mode = 0;
					return ;
				}
				else if(strcmp(buff,"[0J")==0)
				{
					TBD.bDirty = 1;
					int pos = TBD.cursorY*TBD.BW+TBD.cursorX;
					memset(TBD.Buffer+pos*2,0,TBD.BW*TBD.BH*2-pos*2);
					mode = 0;
					return ;
				}
				else if(strcmp(buff,"[J")==0)
				{
					TBD.bDirty = 1;
					int pos = TBD.cursorY*TBD.BW+TBD.cursorX;
					memset(TBD.Buffer+pos*2,0,TBD.BW*TBD.BH*2-pos*2);
					mode = 0;
					return ;
				}
			}

			else if(ch=='A')
			{
				if(sscanf(buff,"[%dA",&X)==2)
				{
					TBD.bDirty = 1;
					TBD.cursorY -= X;
					mode = 0;
					return ;
				}
			}
			else if(ch=='B')
			{
				if(sscanf(buff,"[%dB",&X)==1)
				{
					TBD.bDirty = 1;
					//int pos = TBD.cursorY*TBD.BW+TBD.cursorX;
					//memset(TBD.Buffer+pos,0,TBD.BW*TBD.BH-pos);
					TBD.cursorY += X;
					mode = 0;
					return ;
				}
			}
			else if(ch=='C')
			{

				if(sscanf(buff,"[%dC",&X)==1)
				{
					TBD.bDirty = 1;
					//int pos = TBD.cursorY*TBD.BW+TBD.cursorX;
					//memset(TBD.Buffer+pos,0,TBD.BW*TBD.BH-pos);
					TBD.cursorX += X;
					mode = 0;
					return ;
				}
			}
			else if(ch=='D')
			{
				if(sscanf(buff,"[%dD",&X)==1)
				{
					TBD.bDirty = 1;
					//int pos = TBD.cursorY*TBD.BW+TBD.cursorX;
					//memset(TBD.Buffer+pos,0,TBD.BW*TBD.BH-pos);
					TBD.cursorX -= X;
					mode = 0;
					return ;
				}
			}
			else if(ch=='m')
			{
				//printf("%s\n",buff);
				mode = 0;
				return ;
			}
			else if(ch=='h')
			{
				//printf("%s\n",buff);
				mode = 0;
				return ;
			}
			else if(ch=='K')
			{
				if(strcmp(buff,"[K")==0||strcmp(buff,"[0K")==0)
				{
					TBD.bDirty = 1;
					for(int k=TBD.cursorX*2;k<TBD.BW*2;k++)
					{
						TBD.Buffer[(TBD.cursorY*TBD.BW*2+k)] = 0;
					}
					mode = 0;
					return ;
				}
				else if(strcmp(buff,"[1K")==0)
				{
					TBD.bDirty = 1;
					for(int k=0;k<TBD.cursorX*2;k++)
					{
						TBD.Buffer[(TBD.cursorY*TBD.BW*2+k)] = 0;
					}
					mode = 0;
					return ;
				}
				else if(strcmp(buff,"[2K")==0)
				{
					TBD.bDirty = 1;
					for(int k=0;k<k<TBD.BW*2;k++)
					{
						TBD.Buffer[(TBD.cursorY*TBD.BW*2+k)] = 0;
					}
					mode = 0;
					return ;
				}
			}
			else if(ch=='L')
			{
				if(strcmp(buff,"[L")==0)
				{
					TBD.bDirty = 1;
					int pos = TBD.cursorY*TBD.BW;//+TBD.cursorX;
					for(int k=pos*2;k<TBD.BW*(TBD.BH-1)*2;k++)
					{
							TBD.Buffer[k] = TBD.Buffer[k+TBD.BW*2];
					}
					memset(TBD.Buffer+TBD.BW*(TBD.BH-1)*2,0,TBD.BW*2);
					mode = 0;
					return ;
				}
				else if (strcmp(buff,"[1L")==0)
				{

					TBD.bDirty = 1;
					mode = 0;
					return ;
				}

			}
			else if(strcmp(buff,"[s")==0)
			{
				oldX = 	TBD.cursorX;
				oldY = 	TBD.cursorY;
				mode = 0;
				return ;
			}
			else if(strcmp(buff,"[u")==0)
			{
				TBD.cursorX = oldX;
				TBD.cursorY = oldY;
				mode = 0;
				return ;
			}
			mode++;
			return ;
		}
	}
	if(ch==0x8)
	{
		TBD.bDirty   = 1;
		TBD.cursorX --;
		if(TBD.cursorX<0)TBD.cursorX = 0;
		return ;
	}
	//printf("char %x %d\n",ch,ch);
	if(ch=='\r')
	{
		return ;
		//ch = '\n' ;
	}
	if(ch==FF)
	{
		TBD.bDirty = 1;
		TBD.cursorX = 0;
		TBD.cursorY = 0;
		memset(TBD.Buffer,0,TBD.BW*TBD.BH*2);
		return ;
		//ch = '\n' ;
	}

	TBD.bDirty = 1;
	int pos = TBD.cursorY*TBD.BW+TBD.cursorX;
	if(ch!='\n')
	{
		TBD.Buffer[pos*2] = ch;
		TBD.Buffer[pos*2+1]  = TBD.currAttribute;
		TBD.cursorX++;
	}
	if(TBD.cursorX>=TBD.BW||ch=='\n')
	{
		TBD.cursorY++;
		TBD.cursorX = 0;
		if(TBD.cursorY>=TBD.BH)
		{
			for(int k=0;k<TBD.BW*(TBD.BH-1)*2;k++)
			{
					TBD.Buffer[k] = TBD.Buffer[k+TBD.BW*2];
			}
			TBD.cursorY = TBD.BH -1;
			memset(TBD.Buffer+TBD.BW*(TBD.BH-1)*2,0,TBD.BW*2);
		}
	}
}


uint16_t FG[] =
{
	WHITE,
	GREEN,
	DGREEN,
	YELLOW,

	MAGENTA,
	CYAN,
	NAVY,
	DCYAN,

	MAROON,
	PURPLE,
	OLIVE,
	LGRAY,

	RED,
	ORANGE,
	BROWN,
	BLUE
};
uint16_t BG[] =
{
	BLACK,
	GREEN,
	DGREEN,
	YELLOW,

	MAGENTA,
	CYAN,
	NAVY,
	DCYAN,

	MAROON,
	PURPLE,
	OLIVE,
	LGRAY,

	RED,
	ORANGE,
	BROWN,
	BLUE
};

void paint()
{
	 if(TBD.Buffer)
	 {
		 if(TBD.bDirty)
		 {
			 for(int y=0;y<TBD.BH;y++)
			 {
				 for(int x=0;x<TBD.BW;x++)
				 {
					 int pos = y*TBD.BW+x;
					 //LCD_Draw_Char2(TBD.Buffer[pos], x*TBD.FW*CHAR_WIDTH, y*TBD.FH*CHAR_HEIGHT,WHITE, TBD.FW,TBD.FH,BLACK);
					 uint8_t fg[3] = {255,255,0};
					 //uint16_t
					// LCD_Draw_Char2(TBD.Buffer[pos], x*TBD.FW*CHAR_WIDTH, y*TBD.FH*CHAR_HEIGHT,color_convertRGB_to16(fg), TBD.FW,TBD.FH,BLUE);
					 //LCD_Draw_Char2(TBD.Buffer[pos], x*TBD.FW*CHAR_WIDTH, y*TBD.FH*CHAR_HEIGHT,YELLOW, TBD.FW,TBD.FH,color_convertRGB_to16(fg));
					 //LCD_Draw_Char2(TBD.Buffer[pos], x*TBD.FW*CHAR_WIDTH, y*TBD.FH*CHAR_HEIGHT,YELLOW, TBD.FW,TBD.FH,BLUE);

					 if(TBD.Buffer[pos*2]!= TBD.BufferShadow[pos*2])
					 {
						 TBD.BufferShadow[pos*2] = TBD.Buffer[pos*2];
						 uint8_t color = TBD.Buffer[pos*2+1];
						 //LCD_Draw_Char2(TBD.Buffer[pos*2], x*TBD.FW*CHAR_WIDTH, y*TBD.FH*CHAR_HEIGHT,FG[color>>4], TBD.FW,TBD.FH,BG[color&0xf]);
						 LCD_Draw_Char2Font(&Font4x8,TBD.Buffer[pos*2], x*TBD.FW*CHAR_WIDTH, y*TBD.FH*CHAR_HEIGHT,FG[color>>4], TBD.FW,TBD.FH,BG[color&0xf]);
					 }
					 if(y==TBD.cursorY && x==TBD.cursorX)
					 {
						 TBD.BufferShadow[pos*2] = -1;
						 uint8_t color = TBD.Buffer[pos*2+1];
						 //LCD_Draw_Char2(TBD.Buffer[pos*2], x*TBD.FW*CHAR_WIDTH, y*TBD.FH*CHAR_HEIGHT,BG[color&0xf], TBD.FW,TBD.FH,FG[color>>4]);
						 LCD_Draw_Char2Font(&Font4x8,TBD.Buffer[pos*2], x*TBD.FW*CHAR_WIDTH, y*TBD.FH*CHAR_HEIGHT,BG[color&0xf], TBD.FW,TBD.FH,FG[color>>4]);
					 }
				 }
			 }
			 //LCD_fillRect(TBD.cursorX*TBD.FW*CHAR_WIDTH,TBD.cursorY*TBD.FH*CHAR_HEIGHT,)
			 TBD.bDirty = 0;
		 }
	 }
}

#endif


u8 *RAM = NULL;

extern const uint8_t cpmBas[];
int n_romActive = 0;

void putString(char* str)
{
	for(int k=0;k<strlen(str);k++)
	{
		putCharCurs(str[k]);
	}
}
extern uint8_t   RAMP[4][0x4000];
uint8_t * getRamPage(int k);
void init_cpm()
{
	RAM = &RAMP[0][0];//getRamBuffer0x4000(0);
	n_romActive = 0;
	rclear();
	TBD.Buffer = NULL;

	setTermBuffer(LANDSCAPE,1,1);
	paint();
	//Delay(10);
	printf("init_cpm()\n");
	//LCD_Draw_TextFont(&Font4x8,"Hello this is the test string",0,0,GREEN,1,BLACK);
	//LCD_Draw_TextFont(&Font4x8,"Hello this is the test string",0,8,GREEN,1,BLACK);
	//LCD_Draw_TextFont(&Font4x8,"Hello this is the test string",0,16,GREEN,1,BLACK);
}
//#include "../Add/boot_rom.h"
//#include "../Add/CMON32.h"
#include "../Add/CPM_BASIC.h"

//http://land-boards.com/blwiki/index.php?title=Mc-2g-102

//int pages[4] = {0,1,2,3};
//int page_entry = 0;


extern const unsigned long eprom_length ;
u8  peek_cpm(uint16_t addr)
{

	u8 res;
	res = RAM[addr];
	if(addr < 0x2000 && !n_romActive)
	{
		res = eprom[addr];
	}
	return res;
}
void  poke_cpm(uint16_t addr,uint8_t value)
{
	RAM[addr]=value;
}

#define RSD_DATA    0x88
#define RSD_CONTROL 0x89
#define RSD_STATUS  0x89
#define RSD_LBA0    0x8A
#define RSD_LBA1    0x8B
#define RSD_LBA2    0x8C

u8      SD_BUFF[512];

uint32_t LBA0;
uint32_t LBA1;
uint32_t LBA2;

uint32_t sector    = 0;
uint8_t  disk_stat = 0;
int wPos           = 512;
int rPos           = 512;

#if defined(SDMMC_DATATIMEOUT)
#define SD_TIMEOUT SDMMC_DATATIMEOUT
#elif defined(SD_DATATIMEOUT)
#define SD_TIMEOUT SD_DATATIMEOUT
#else
#define SD_TIMEOUT 30 * 1000
#endif

u8 in_cpm(u16 port)
{
	u8 res = 0xff;
	static uint32_t now;
	switch(port&0xff)
	{
		case 0x60:  now = HAL_GetTick() ; res = ((uint8_t*)&now)[0]  ;break;
		case 0x61:  res =  ((uint8_t*)&now)[1] ;break;
		case 0x62:  res =  ((uint8_t*)&now)[2] ;break;
		case 0x63:  res =  ((uint8_t*)&now)[3] ;break;

		case 0x81:
		{
						if(rsize()<BSIZE/2)
						{
							//HAL_GPIO_WritePin(UART_RTS_GPIO_Port, UART_RTS_Pin, GPIO_PIN_RESET);
						}
						if(rsize())
						{
							res =rget();
						}
						else
						{
							res = 0;
						}
		}
		break;
		//case 0x80:  res = ((UartReadyT==SET)<<1)|(rsize()!=0);break;
		case 0x80:  res = (rsize()!=0) | 2;break;
		case 0x83:  res = 0;   break;
		case 0x82:  res = 0b10;break;
		case 0x85:  res = 0;   break;
		case 0x84:  res = 0b10;break;
		case 0x86:  res = 0;   break;
		case 0x87:  res = 0b10;break;

		case RSD_DATA:
						if(rPos<512)
						{
							res = SD_BUFF[rPos];
							if(rPos<6)
							{
								//printf("read 0=%x\n",res);
							}
							rPos++;
							if(rPos==512)
							{
			  	  	  			  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
								 // printf("end read\n");
							}
						}
						else
						{
							printf("out of block read\n");
						}
						break;
		case RSD_STATUS:
						if((rPos==512)&&(wPos==512))
						{
							res = 128;
						}
						else if(wPos<512)
						{
							res = 160;
						}
						else if(rPos<512)
						{
							res = 224;
						}
						else
						{
							res = 0;
							printf("unknown status\n");
						}
						break;
		default:;
			printf("Unknown port read %x !!!!!\n",port);
	};
	//printf("in %04x=%02x\n",port,res);
	return res ;
}

void out_cpm(u16 port, u8 value)
{
	//printf("out %04x=%02x\n",port,value);
	//$Date: 2019/12/26 18:05:24 $
	//
	char sys_info2 = "$Id: main.c,v 1.7 2019/12/26 18:05:24 gpu Exp $";
	char sys_rev = "$Revision: 1.7 $";

	switch(port&0xff)
	{
	//  case 0xF8:      page_entry = value&3;  break;
	//  case 0xFD:      pages[page_entry] = value&0x3f; break;
	  case 0x81:      {
		  	  	  	  //if (value == 27)
		  	  	  	  //	  value = '-';
		  	  	  	  //printf("out %x\n",value);
		  	  	  	  	  //retarget_put_char(value);	//putCharCurs(value);
		  	  	  	  	  putCharCurs(value);
	  	  	  	  	  }
	  	  	  	  	   break;
	  case 0x80: break;
	  case 0x83: break;
	  case 0x82: break;
	  case 0x85: break;
	  case 0x84: break;
	  case 0x87: break;
	  case 0x86: break;

	  case 0x7b: break;
	  case 0x7c: break;
	  case 0x7d: break;
	  case 0x7e: break;


	  case 0x38: n_romActive = 1;
	  	  	  	  	  	  printf("rom not Active!!!!!\n");
	  	  	  	  break;
	  case 0x39: n_romActive = 0;
	  	  	  	  	  	  printf("rom Active!!!!!\n");
	  	  	  	  break;
	  	  	 // c:pip FILES.BES=FILES.TXT
	  case RSD_LBA0:LBA0 = value;break;
	  case RSD_LBA1:LBA1 = value;break;
	  case RSD_LBA2:LBA2 = value&0x7f;break;
	  case RSD_DATA:  	  if(wPos<512)
		  	  	  		  {
		  	  	  			  SD_BUFF[wPos] = value;
		  	  	  			  wPos ++;
		  	  	  			  if(wPos==512)
		  	  	  			  {
		  	  	  				  sector = ((((LBA2<<8)|LBA1)<<8)|LBA0);
		  	  	  				  HAL_StatusTypeDef stat = BSP_SD_WriteBlocks(&SD_BUFF[0],sector,1, SD_TIMEOUT);
		  	  	  				  if(stat)
		  	  	  					  printf("writeBlock err sect %x stat %x \n",sector,stat);
		  	  	  				  else
		  	  	  					  printf("writeBlock Ok sect %x stat %x \n",sector,stat);
		  	  	  				  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
		  	  	  				  //rPos = 0;
		  	  	  			  }
		  	  	  		  }
	  	  	  	  	  	  else
	  	  	  	  	  	  {
	  	  	  	  	  		  	  printf("unknown write\n");
	  	  	  	  	  	  }

		  	  	  		  break;

	  case RSD_CONTROL:
		  	  	  	  if(value == 0)
		  	  	  	  {
		  	  	  		  //read block
		  	  	  		  sector = ((((LBA2<<8)|LBA1)<<8)|LBA0);
		  	  	  		  HAL_StatusTypeDef stat = 1;
		  	  	  		  {
		  	  	  			  int kk;
		  	  	  			  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
		  	  	  			  for(kk=0;kk<1&&stat!=0;kk++)
		  	  	  			  {
		  	  	  				  stat = BSP_SD_ReadBlocks(&SD_BUFF[0],sector,1, SD_TIMEOUT);
		  	  	  			  }
		  	  	  			  if(stat!=0)
		  	  	  			  {
		  	  	  				  	  printf("readBlock Error %x %d\n",sector,stat);
		  	  	  			  }
		  	  	  		  }
		  	  	  	      //n_romActive = 1;
		  	  	  		  rPos = 0;
		  	  	  	  }
		  	  	  	  else if(value == 1)
		  	  	  	  {
		  	  	  		  	  printf("go write\n");
		  	  	  		  	  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
		  	  	  		  	  wPos = 0;
		  	  	  	  }
		  	  	  	  break;
	  default: ;
		  	  printf("\nUnknown port write %x %x!!!!!\n",port,value);


	};

}


