/**
  ******************************************************************************
  * @file    ili9341.c
  * @author  MCD Application Team
  * @version V1.0.2
  * @date    02-December-2014
  * @brief   This file includes the LCD driver for ILI9341 LCD.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "ili9341.h"
#include "main.h"

void    Delay(int num)
{
	uint32_t start = HAL_GetTick()+num;
	while((uint32_t)(HAL_GetTick()-start)>0x80000)
	{

		kscan0();
	}
}

#define LCD_REG      (*((volatile unsigned short *) 0x60000000)) 
// A18
#define LCD_RAM      (*((volatile unsigned short *) (0x60000000+(1<<(18+1))) )) 
// A16

inline  void FMC_BANK1_WriteData(uint16_t Data) 
{
  /* Write 16-bit Reg */
	//~ FMC_BANK1->RAM = Data;
  LCD_RAM = Data;
}

inline void FMC_BANK1_WriteReg(uint8_t Reg) 
{
  /* Write 16-bit Index, then write register */
	//~ FMC_BANK1->REG = Reg;
  LCD_REG = Reg;
}

/**
  * @brief  Reads register value.
  * @retval Read value
  */
inline uint16_t FMC_BANK1_ReadData(void) 
{
	//~ return FMC_BANK1->RAM;
	//~ return FWREAD();
  return LCD_RAM;
}

/**
  * @brief  Writes data on LCD data register.
  * @param  Data: Data to be written
  */
void  LCD_IO_WriteData(uint16_t Data)
{
  /* Write 16-bit Reg */
  FMC_BANK1_WriteData(Data);
}

/**
  * @brief  Writes multiple data on LCD data register.
  * @param  pData Pointer on the register value
  * @param  Size Size of byte to transmit to the register
  * @retval None
  */
inline void LCD_IO_WriteMultipleData(uint8_t *pData, uint32_t Size)
{
  uint32_t counter;
  uint16_t *ptr = (uint16_t *) pData;
  
  for (counter = 0; counter < Size; counter+=2)
  {  
    /* Write 16-bit Reg */
    FMC_BANK1_WriteData(*ptr);
    ptr++;
  }
}

/**
  * @brief  Writes register on LCD register.
  * @param  Reg: Register to be written
  */
void LCD_IO_WriteReg(uint8_t Reg)
{
  /* Write 16-bit Index, then Write Reg */
  FMC_BANK1_WriteReg(Reg);
}

/**
  * @brief  Reads data from LCD data register.
  * @param  Reg: Register to be read
  * @retval Read data.
  */
inline uint16_t LCD_IO_ReadData(uint16_t Reg)
{
  FMC_BANK1_WriteReg(Reg);
  
  /* Read 16-bit Reg */  
  return FMC_BANK1_ReadData();
}


uint16_t F_LCD_IO_ReadData(uint16_t Reg)
{
	return LCD_IO_ReadData(Reg);
}
uint16_t F_FMC_BANK1_ReadData()
{
	return LCD_RAM;
}

#define TFT_REG_COL         0x2A
#define TFT_REG_PAGE        0x2B
#define TFT_REG_MEM_WRITE   0x2C
#define TFT_REG_TE_OFF      0x34
#define TFT_REG_TE_ON       0x35
#define TFT_REG_GAMMA_1     0xe0
#define TFT_REG_GAMMA_2     0xe1

      void initTft();
     void tftReset();
     void sendRegister(unsigned int reg);
     void sendCommand(unsigned int data);
     void sendData(unsigned int data);
     void pushDataReg(unsigned int data);
     void pushDataRam(unsigned int data);
     void enterSleep();
     void exitSleep();
     void setOrientation(unsigned int HV);
     void setXY(uint16_t poX, uint16_t poY);
    void setCol(uint16_t startX, uint16_t endX);
    void setPage(uint16_t startY, uint16_t endY);
    void fillRectangle(uint16_t poX, uint16_t poY,
                       uint16_t width, uint16_t length,
                       uint16_t color);
    void drawHorizontalLine(uint16_t poX, uint16_t poY, uint16_t length,
                            uint16_t color);
    void setWindow(uint16_t startX, uint16_t startY, uint16_t endX, uint16_t endY);
    void setBacklightON();
    void setBacklightOff();

void ili9341_WriteReg(uint8_t LCD_Reg)
{
  LCD_IO_WriteReg(LCD_Reg);
}


/**
  * @brief  Writes data to the selected LCD register.
  * @param  LCD_Reg: address of the selected register.
  * @retval None
  */
void setBK_imp(int perc);

int iBacklightPercent = 0;
void setBacklight(int percent)
{
	if(percent>100)percent = 100;
	if(percent<10)percent = 10;
	iBacklightPercent = percent;
	setBK_imp(iBacklightPercent);
}
int getBacklight()
{
	return iBacklightPercent;
}

void setBacklightON()
{

	setBK_imp(iBacklightPercent);

	//HAL_GPIO_WritePin(LCD_BL_GPIO_Port,LCD_BL_Pin,GPIO_PIN_SET);
  //~ TIM4->CCR1 = 333;
}

void setBacklightOff()
{
	setBK_imp(0);
  //~ TIM4->CCR1 = 0;
	//HAL_GPIO_WritePin(LCD_BL_GPIO_Port,LCD_BL_Pin,GPIO_PIN_RESET);
}

void ili9341_DisplayOn(void)
{
  /* Display On */
  setBacklight(100);
  ili9341_WriteReg(LCD_DISPLAY_ON);
}

/**
  * @brief  Disables the Display.
  * @param  None
  * @retval None
  */
void ili9341_DisplayOff(void)
{
  /* Display Off */
  setBK_imp(0);
  ili9341_WriteReg(LCD_DISPLAY_OFF);
}

void ili9341_WriteData(uint16_t RegValue)
{
  LCD_IO_WriteData(RegValue);
}
    
void ili9341_Init(void)
{
	initTft();
	//HAL_GPIO_WritePin(LCD_BL_GPIO_Port,LCD_BL_Pin,GPIO_PIN_SET);
}
    void initTft()
    {
  tftReset();

  sendCommand(0x01);
  sendCommand(0x28);
  
  Delay(5);
  
  sendCommand(0xcf);
  sendRegister(0x0000);
  sendRegister(0x0083);
  sendRegister(0x0030);

  sendCommand(0xed);
  sendRegister(0x0064);
  sendRegister(0x0003);
  sendRegister(0x0012);
  sendRegister(0x0081);

  sendCommand(0xe8);
  sendRegister(0x0085);
  sendRegister(0x0001);
  sendRegister(0x0079);

  sendCommand(0xcb);
  sendRegister(0x0039);
  sendRegister(0x002c);
  sendRegister(0x0000);
  sendRegister(0x0034);
  sendRegister(0x0002);

  sendCommand(0xf7);
  sendRegister(0x0020);

  sendCommand(0xea);
  sendRegister(0x0000);
  sendRegister(0x0000);

  sendCommand(0xc0);
  sendRegister(0x0026);

  sendCommand(0xc1);
  sendRegister(0x0011);

  sendCommand(0xc5);
  sendRegister(0x0035);
  sendRegister(0x003e);

  sendCommand(0xc7);
  sendRegister(0x00be);

  sendCommand(0x36);
  sendRegister(0x0048);//48
  sendCommand(LCD_PIXEL_FORMAT);

#ifdef COLOR_3BYTES
  sendRegister(0x0066);
#else
  sendRegister(0x0055);
#endif

  sendCommand(LCD_FRMCTR1);
  sendRegister(0x0000);
  sendRegister(0x10); // default 0x1B
  //sendRegister(0x0010); // default 0x1B
  //sendRegister(0x0030); // default 0x1B

  sendCommand(0xF2);
  sendRegister(0x0008);

  sendCommand(0x26);
  sendRegister(0x0001);

  sendCommand(TFT_REG_GAMMA_1);
  sendRegister(0x001f);
  sendRegister(0x001a);
  sendRegister(0x0018);
  sendRegister(0x000a);
  sendRegister(0x000f);
  sendRegister(0x0006);
  sendRegister(0x0045);
  sendRegister(0x0087);
  sendRegister(0x0032);
  sendRegister(0x000a);
  sendRegister(0x0007);
  sendRegister(0x0002);
  sendRegister(0x0007);
  sendRegister(0x0005);
  sendRegister(0x0000);

  sendCommand(TFT_REG_GAMMA_2);
  sendRegister(0x0000);
  sendRegister(0x0025);
  sendRegister(0x0027);
  sendRegister(0x0005);
  sendRegister(0x0010);
  sendRegister(0x0009);
  sendRegister(0x003a);
  sendRegister(0x0078);
  sendRegister(0x004d);
  sendRegister(0x0005);
  sendRegister(0x0018);
  sendRegister(0x000d);
  sendRegister(0x0038);
  sendRegister(0x003a);
  sendRegister(0x001f);

  sendCommand(TFT_REG_COL);
  sendRegister(0x0000);
  sendRegister(0x0000);
  sendRegister(0x0000);
  sendRegister(0x00ef);

  sendCommand(TFT_REG_PAGE);
  sendRegister(0x0000);
  sendRegister(0x0000);
  sendRegister(0x0001);
  sendRegister(0x003f);

  sendCommand(TFT_REG_TE_OFF);
  sendRegister(0x0000);

  sendCommand(0xb7);
  sendRegister(0x0007);

  sendCommand(0xb6);
  sendRegister(0x000a);
  sendRegister(0x0082);
  sendRegister(0x0027);
  sendRegister(0x0000);

  sendCommand(0x11);

  Delay(100);

  sendCommand(0x29);
  
  Delay(100);

  sendCommand(TFT_REG_MEM_WRITE);
  //setBacklight(50);
	    
}
    
void sendRegister(unsigned int reg)
{
  pushDataRam(reg);
}

void sendCommand(unsigned int data)
{
  pushDataReg(data);
}

void sendData(unsigned int data)
{
  pushDataRam(data);
}

void pushDataReg(unsigned int data)
{
  ili9341_WriteReg(data);
}

void pushDataRam(unsigned int data)
{
  ili9341_WriteData(data);
}

void tftReset()
{
  Delay(10);
  Delay(15);
  Delay(120);
  ili9341_WriteReg(LCD_SWRESET);
  Delay(120);
}

void enterSleep()
{
  sendCommand(0x28);
  Delay(20);
  sendCommand(0x10);
}

void exitSleep()
{
  sendCommand(0x11);
  Delay(120);
  sendCommand(0x29);
}

#if 0
void setOrientation(unsigned int HV)
{
  sendCommand(0x03);
  if(HV==1)//vertical
  {
    sendData(0x5038);
  }
  else//horizontal
  {
    sendData(0x5030);
  }
  sendCommand(0x0022); //Start to write to display RAM
}
#endif

void setCol(uint16_t startX, uint16_t startY)
{
  sendCommand(0x2A);
  sendRegister(startX);
  sendRegister(startY);
}

void setPage(uint16_t endX, uint16_t endY)
{
  sendCommand(0x2B);
  sendRegister(endX);
  sendRegister(endY);
}

void setXY(uint16_t poX, uint16_t poY)
{
  //setCol(poX, poX);
  setPage(poY, poY);
  sendCommand(TFT_REG_MEM_WRITE);
}
void setWindow(uint16_t startX, uint16_t endX, uint16_t startY, uint16_t endY)
 {
    /*
    sendCommand(0x2a);
    
    sendRegister((startX >> 8) & 0xFF);
    sendRegister(startX & 0xFF);
    sendRegister((endX >> 8) & 0xFF);
    sendRegister(startY & 0xFF);
  
    sendCommand(0x2b);
    sendRegister((startY >> 8) & 0xFF);
    sendRegister(endX & 0xFF);
    sendRegister((endY >> 8) & 0xFF);
    sendRegister(endY >> 8);
    */
    sendCommand(TFT_REG_COL);
    sendRegister(startX >> 8);
    sendRegister(startX);
    sendRegister(endX >> 8);
    sendRegister(endX);
    sendCommand(TFT_REG_PAGE);
    sendRegister(startY >> 8);
    sendRegister(startY);
    sendRegister(endY >> 8);
    sendRegister(endY);


    sendCommand(TFT_REG_MEM_WRITE);
 }
#if 0
void fillRectangle(uint16_t poX, uint16_t poY,
                                uint16_t width, uint16_t length,
                                uint16_t color)
{
  uint32_t windowSize = length * width;
  uint32_t count = 0;

  setWindow(poX, (poX + width), poY, (poY + length));
  /*for( uint32_t i = 0; i <= windowSize; i++ )
  {
    sendData(color);
    count++;
  }
  sendData(color);  */
  int i = 0;
  int j = 0;
  for( i = 0; i < length; i++ )
  {
    for( j = 0; j < width + 100; j++ )
    {
      sendData(color);
    }
  }
}
#endif
void color_convert(uint16_t color,uint8_t* result)
{
	//~ #define RED             0xF800
	//~ #define BLUE            0x001F
	//~ #define GREEN           0x07E0
	result[2]=  ((color&0x1f)		<<(1+2))<<1;//|0x80;    //5 bit BLUE
	result[1]=  (((color>>5)&0x3f) <<(0+1))<<1;//|0x80;    //6 bit GREEN
	result[0]=  (((color>>11)&0x1f)<<(1+2))<<1;//|0x80;    //5 bit  //RED
}

uint16_t color_convertRGB_to16(uint8_t * adress)
{
	return ((adress[0]>>3)<<11)|((adress[1]>>2)<<5)|(adress[2]>>3);
}
void LCD_fillRect(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint16_t color)
{
	int dw = w*h;
	setWindow(x1, x1+w-1, y1, y1+h-1);
	int k;
	for(k=0;k<dw;k++)
	{
		sendData(color);
	}
	///fillRectangle(x1,y1,w,h,color);
}
void LCD_Write8x8line(uint16_t x1, uint16_t y1,uint8_t * adress)
{
	setWindow(x1, (uint16_t) (x1+8-1),y1,(uint16_t) (y1+8-1));
	//uint16_t ncolor;
	int k;
	for(k=0;k<8*8;k++)
	{
		uint16_t color = color_convertRGB_to16(adress);
		sendData(color);
		adress+=3;
	}
}
#include "stm32f4xx_hal.h"
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream3;
//HAL_StatusTypeDef HAL_DMA_PollForTransfer(DMA_HandleTypeDef *hdma, HAL_DMA_LevelCompleteTypeDef CompleteLevel, uint32_t Timeout)

void LCD_Writeline16(uint16_t x1,int w, uint16_t y1,uint16_t * adress)
{
	HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream3,HAL_DMA_FULL_TRANSFER,1);
	setWindow(x1, (uint16_t) (x1+w-1),y1,(uint16_t) (y1));
	//HAL_StatusTypeDef HAL_DMA_Start(DMA_HandleTypeDef *hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength)
	//LCD_RAM
	while(w>=0x4000)
	{
		HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream3,HAL_DMA_FULL_TRANSFER,1);
		HAL_StatusTypeDef res = HAL_DMA_Start(&hdma_memtomem_dma2_stream3, adress, &LCD_RAM,0x4000);
		adress+= 0x4000;
		w-=0x4000;
	}
	HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream3,HAL_DMA_FULL_TRANSFER,1);
	HAL_StatusTypeDef res = HAL_DMA_Start(&hdma_memtomem_dma2_stream3, adress, &LCD_RAM,w);

//	if(res!=HAL_OK)
////	{
////		printf("hal_dma %x\n",res);
////	}
//	for(int k=0;k<w;k++)
//	{
//		sendData(adress[k]);
//	}
}
void LCD_Write8x8line16(uint16_t x1, uint16_t y1,uint16_t * adress)
{
	HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream3,HAL_DMA_FULL_TRANSFER,1);
	setWindow(x1, (uint16_t) (x1+8-1),y1,(uint16_t) (y1+8-1));
	//uint16_t ncolor;
	int k;
	HAL_StatusTypeDef res = HAL_DMA_Start(&hdma_memtomem_dma2_stream3, adress, &LCD_RAM,8*8);
//	for(k=0;k<8*8;k++)
//	{
//		sendData(adress[k]);
//	}
}
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream4;
void LCD_FullRect3(uint16_t x1, uint16_t y1,uint8_t * adress,uint16_t w,uint16_t h)
{
	int dw = w*h;
	HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream4,HAL_DMA_FULL_TRANSFER,1);
	setWindow(x1, x1+w-1, y1, y1+h-1);
	int k;
	static uint16_t color ;
	color = color_convertRGB_to16(adress);
		while(dw>=0x4000)
		{
			HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream4,HAL_DMA_FULL_TRANSFER,1);
			HAL_StatusTypeDef res = HAL_DMA_Start(&hdma_memtomem_dma2_stream4, &color, &LCD_RAM,0x4000);
			if(res!=HAL_OK)
			{
				printf("res3a = %x dw=%x \n",res,dw);
			}
			dw-=0x4000;
		}
		HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream4,HAL_DMA_FULL_TRANSFER,1);
		HAL_StatusTypeDef res = HAL_DMA_Start(&hdma_memtomem_dma2_stream4, &color, &LCD_RAM,dw);
		if(res!=HAL_OK)
		{
			printf("res3 = %x\n",res);
		}

//	for(k=0;k<dw;k++)
//	{
//		LCD_RAM = color;
//	}

}

uint16_t ili9341_ReadID(void)
{
  //~ LCD_IO_Init();
  return ((uint16_t) ili9341_ReadData(LCD_READ_ID4) );
}

uint32_t ili9341_ReadData(uint16_t RegValue)
{
  /* Read a max of 4 bytes */
  return (LCD_IO_ReadData(RegValue));
}
static uint16_t screen_width  = ILI9341_LCD_PIXEL_WIDTH;
static uint16_t screen_height = ILI9341_LCD_PIXEL_HEIGHT;
#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

void LCD_setRotation(uint8_t rotation)
{
    screen_height = ILI9341_LCD_PIXEL_HEIGHT;
    screen_width  = ILI9341_LCD_PIXEL_WIDTH;
	uint8_t madctl = 0;
	switch (rotation&0x3) {
	  case PORTRAIT:
		madctl = (MADCTL_MX | MADCTL_BGR);
		break;
	  case LANDSCAPE:
		madctl = (MADCTL_MV | MADCTL_BGR);
        screen_height = ILI9341_LCD_PIXEL_WIDTH;
        screen_width  = ILI9341_LCD_PIXEL_HEIGHT;
		break;
	  case PORTRAIT_FLIP:
		madctl = (MADCTL_MY | MADCTL_BGR);
		break;
	  case LANDSCAPE_FLIP:
		madctl = (MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
        screen_height = ILI9341_LCD_PIXEL_WIDTH;
        screen_width  = ILI9341_LCD_PIXEL_HEIGHT;
		break;
	}
	  ili9341_WriteReg(LCD_MAC);
	  ili9341_WriteData(madctl);
//    TFT_CS_RESET;
//    dmaSendCmdCont(LCD_MAC);
//    dmaSendDataCont8(&madctl, 1);
//    TFT_CS_SET;
}
uint16_t LCD_getWidth() {
    return screen_width;
}

uint16_t LCD_getHeight() {
    return screen_height;
}

void LCD_init()
{
	ili9341_Init();
}

#include "5x5_font.h"
/*
void LCD_Draw_Char(char Character, int16_t X, int16_t Y, uint16_t Colour, uint16_t Size, uint16_t Background_Colour)
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

	char temp[CHAR_WIDTH];
	uint8_t k;
	for( k = 0; k<CHAR_WIDTH; k++)
	{
		temp[k] = font[function_char*CHAR_WIDTH+k];
	}

    // Draw pixels
		LCD_fillRect(X, Y, CHAR_WIDTH*Size, CHAR_HEIGHT*Size, Background_Colour);
    for (j=0; j<CHAR_WIDTH; j++) {
        for (i=0; i<CHAR_HEIGHT; i++) {
            if (temp[i] & (1<<(CHAR_WIDTH-1-j))) {
								//~ LCD_fillRect(X+(j*Size), Y+(i*Size), Size,Size, Colour);
							LCD_fillRect(X+(j*Size), Y+(i*Size), Size,Size, Colour);
            }
        }
    }
}*/
void LCD_Draw_Char2(char Character, int16_t X, int16_t Y, uint16_t Colour, uint16_t SizeX,uint16_t SizeY, uint16_t Background_Colour)
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

	char temp[CHAR_WIDTH];
	uint8_t k;
	for( k = 0; k<CHAR_WIDTH; k++)
	{
		temp[k] = font[function_char*CHAR_WIDTH+k];
	}

    // Draw pixels
	LCD_fillRect(X, Y, CHAR_WIDTH*SizeX, CHAR_HEIGHT*SizeY, Background_Colour);
    for (j=0; j<CHAR_WIDTH; j++) {
        for (i=0; i<CHAR_HEIGHT; i++) {
            if (temp[i] & (1<<(CHAR_WIDTH-1-j))) {
								//~ LCD_fillRect(X+(j*Size), Y+(i*Size), Size,Size, Colour);
							LCD_fillRect(X+(j*SizeX), Y+(i*SizeY), SizeX,SizeY, Colour);
            }
        }
    }
}
void LCD_Draw_Char(char Character, int16_t X, int16_t Y, uint16_t Colour, uint16_t Size, uint16_t Background_Colour)
{
	LCD_Draw_Char2(Character,X,Y,Colour,Size,Size,Background_Colour);
}
/*Draws an array of characters (fonts imported from fonts.h) at X,Y location with specified font colour, size and Background colour*/
/*See fonts.h implementation of font on what is required for changing to a different font when switching fonts libraries*/
void  LCD_Draw_Text(const char* Text, int16_t X, int16_t Y, uint16_t Colour, uint16_t Size, uint16_t Background_Colour)
{
    while (*Text) {
        LCD_Draw_Char(*Text, X, Y, Colour, Size, Background_Colour);
        X += CHAR_WIDTH*Size;
	Text++;
    }
}
void  LCD_Draw_Text2(const char* Text, int16_t X, int16_t Y, uint16_t Colour, uint16_t SizeX, uint16_t SizeY,uint16_t Background_Colour)
{
    while (*Text) {
        LCD_Draw_Char2(*Text, X, Y, Colour, SizeX,SizeY, Background_Colour);
        X += CHAR_WIDTH*SizeX;
	Text++;
    }
}


#if 0
#define COLOR_3BYTES 1
/** @addtogroup BSP
  * @{
  */ 

/** @addtogroup Components
  * @{
  */ 
  
/** @addtogroup ILI9341
  * @brief This file provides a set of functions needed to drive the 
  *        ILI9341 LCD.
  * @{
  */

/** @defgroup ILI9341_Private_TypesDefinitions
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup ILI9341_Private_Defines
  * @{
  */
/**
  * @}
  */ 
  
/** @defgroup ILI9341_Private_Macros
  * @{
  */
/**
  * @}
  */  

/** @defgroup ILI9341_Private_Variables
  * @{
  */ 

LCD_DrvTypeDef   ili9341_drv = 
{
  ili9341_Init,
  ili9341_ReadID,
  ili9341_DisplayOn,
  ili9341_DisplayOff,
  0,
  0,
  0,
  0,
  0,
  0,
  ili9341_GetLcdPixelWidth,
  ili9341_GetLcdPixelHeight,
  0,
  0,    
};

/**
  * @}
  */ 
  
/** @defgroup ILI9341_Private_FunctionPrototypes
  * @{
  */

/**
  * @}
  */ 
  
/** @defgroup ILI9341_Private_Functions
  * @{
  */   

/**
  * @brief  Power on the LCD.
  * @param  None
  * @retval None
  */
  
 static const uint8_t init_commands[] = {
        // Power control A
        6, LCD_POWERA, 0x39, 0x2C, 0x00, 0x34, 0x02,
        // Power control B
        4, LCD_POWERB, 0x00, 0xC1, 0x30,
        // Driver timing control A
        4, LCD_DTCA, 0x85, 0x00, 0x78,
        // Driver timing control B
        3, LCD_DTCB, 0x00, 0x00,
        // Power on sequence control
        5, LCD_POWER_SEQ, 0x64, 0x03, 0x12, 0x81,
        // Pump ratio control
        2, LCD_PRC, 0x20,
        // Power control 1
        2, LCD_POWER1, 0x10,
        // Power control 2
        2, LCD_POWER2, 0x10,
        // VCOM control 1
        3, LCD_VCOM1, 0x3E, 0x28,
        // VCOM cotnrol 2
        2, LCD_VCOM2, 0x86,
        // Memory access control
        2, LCD_MAC, 0x48,
	//~ 2, LCD_MAC, 0xC8,
        // Pixel format set
#ifdef COLOR_3BYTES
	2, LCD_PIXEL_FORMAT, 0x66,
#else
        2, LCD_PIXEL_FORMAT, 0x55,
#endif	
        // Frame rate control
        //3, LCD_FRMCTR1, 0x00, 0x1B,
	3, LCD_FRMCTR1, 0x00, 50,
        // Display function control
        4, LCD_DFC, 0x08, 0x82, 0x27,
        // 3Gamma function disable
        2, LCD_3GAMMA_EN, 0x00,
        // Gamma curve selected
        2, LCD_GAMMA, 0x01,
        // Set positive gamma
        16, LCD_PGAMMA, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
        16, LCD_NGAMMA, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
        0
};
void  dmaSendDataCont8(uint8_t *data,int cnt)
{
	int k ;
	for( k=0;k<cnt;k++)
	{
		ili9341_WriteData(data[k]);
	}
}
void LCD_exitStandby() {
    ili9341_WriteReg(LCD_SLEEP_OUT);
    Delay(150);
    ili9341_WriteReg(LCD_DISPLAY_ON);
}

void ili9341_Init(void)
{
  /* Initialize ILI9341 low level bus layer ----------------------------------*/
  //~ LCD_IO_Init();
#if 0	
  ili9341_WriteReg(0xCA);
  ili9341_WriteData(0xC3);
  ili9341_WriteData(0x08);
  ili9341_WriteData(0x50);
  ili9341_WriteReg(LCD_POWERB);
  ili9341_WriteData(0x00);
  ili9341_WriteData(0xC1);
  ili9341_WriteData(0x30);
  ili9341_WriteReg(LCD_POWER_SEQ);
  ili9341_WriteData(0x64);
  ili9341_WriteData(0x03);
  ili9341_WriteData(0x12);
  ili9341_WriteData(0x81);
  ili9341_WriteReg(LCD_DTCA);
  ili9341_WriteData(0x85);
  ili9341_WriteData(0x00);
  ili9341_WriteData(0x78);
  ili9341_WriteReg(LCD_POWERA);
  ili9341_WriteData(0x39);
  ili9341_WriteData(0x2C);
  ili9341_WriteData(0x00);
  ili9341_WriteData(0x34);
  ili9341_WriteData(0x02);
  ili9341_WriteReg(LCD_PRC);
  ili9341_WriteData(0x20);
  ili9341_WriteReg(LCD_DTCB);
  ili9341_WriteData(0x00);
  ili9341_WriteData(0x00);
  ili9341_WriteReg(LCD_FRMCTR1);
  ili9341_WriteData(0x00);
  ili9341_WriteData(0x1B);
  ili9341_WriteReg(LCD_DFC);
  ili9341_WriteData(0x0A);
  ili9341_WriteData(0xA2);
  ili9341_WriteReg(LCD_POWER1);
  ili9341_WriteData(0x10);
  ili9341_WriteReg(LCD_POWER2);
  ili9341_WriteData(0x10);
  ili9341_WriteReg(LCD_VCOM1);
  ili9341_WriteData(0x45);
  ili9341_WriteData(0x15);
  ili9341_WriteReg(LCD_VCOM2);
  ili9341_WriteData(0x90);
  ili9341_WriteReg(LCD_MAC);
  ili9341_WriteData(0xC8);
  ili9341_WriteReg(LCD_3GAMMA_EN);
  ili9341_WriteData(0x00);
  ili9341_WriteReg(LCD_RGB_INTERFACE);
  ili9341_WriteData(0xC2);
  ili9341_WriteReg(LCD_DFC);
  ili9341_WriteData(0x0A);
  ili9341_WriteData(0xA7);
  ili9341_WriteData(0x27);
  ili9341_WriteData(0x04);
  
  /* Colomn address set */
  ili9341_WriteReg(LCD_COLUMN_ADDR);
  ili9341_WriteData(0x00);
  ili9341_WriteData(0x00);
  ili9341_WriteData(0x00);
  ili9341_WriteData(0xEF);
  /* Page address set */
  ili9341_WriteReg(LCD_PAGE_ADDR);
  ili9341_WriteData(0x00);
  ili9341_WriteData(0x00);
  ili9341_WriteData(0x01);
  ili9341_WriteData(0x3F);
  ili9341_WriteReg(LCD_INTERFACE);
  ili9341_WriteData(0x01);
  ili9341_WriteData(0x00);
  ili9341_WriteData(0x06);
  
  ili9341_WriteReg(LCD_GRAM);
  Delay(200);
  
  ili9341_WriteReg(LCD_GAMMA);
  ili9341_WriteData(0x01);
  
  ili9341_WriteReg(LCD_PGAMMA);
  ili9341_WriteData(0x0F);
  ili9341_WriteData(0x29);
  ili9341_WriteData(0x24);
  ili9341_WriteData(0x0C);
  ili9341_WriteData(0x0E);
  ili9341_WriteData(0x09);
  ili9341_WriteData(0x4E);
  ili9341_WriteData(0x78);
  ili9341_WriteData(0x3C);
  ili9341_WriteData(0x09);
  ili9341_WriteData(0x13);
  ili9341_WriteData(0x05);
  ili9341_WriteData(0x17);
  ili9341_WriteData(0x11);
  ili9341_WriteData(0x00);
  ili9341_WriteReg(LCD_NGAMMA);
  ili9341_WriteData(0x00);
  ili9341_WriteData(0x16);
  ili9341_WriteData(0x1B);
  ili9341_WriteData(0x04);
  ili9341_WriteData(0x11);
  ili9341_WriteData(0x07);
  ili9341_WriteData(0x31);
  ili9341_WriteData(0x33);
  ili9341_WriteData(0x42);
  ili9341_WriteData(0x05);
  ili9341_WriteData(0x0C);
  ili9341_WriteData(0x0A);
  ili9341_WriteData(0x28);
  ili9341_WriteData(0x2F);
  ili9341_WriteData(0x0F);
  ili9341_WriteReg(LCD_PIXEL_FORMAT);
  ili9341_WriteData(0x66);
  
  ili9341_WriteReg(LCD_SLEEP_OUT);
  Delay(200);
  ili9341_WriteReg(LCD_DISPLAY_ON);
  /* GRAM start writing */
  ili9341_WriteReg(LCD_GRAM);
#endif
//~ LCD_exitStandby();
//~ /*
    uint8_t count;
    uint8_t *address = (uint8_t *) init_commands;
    //~ SPI_MASTER->CR1 &= ~SPI_CR1_SPE; // DISABLE SPI
    //~ SPI_MASTER->CR1 &= ~SPI_CR1_DFF; // SPI 8
    //~ SPI_MASTER->CR1 |= SPI_CR1_SPE;  // ENABLE SPI

    //~ TFT_CS_RESET;
    while (1) {
        count = *(address++);
        if (count-- == 0) break;
        ili9341_WriteReg(*(address++));
        dmaSendDataCont8(address, count);
	    //~ TFT_CS_SET;
        address += count;
    }
    //~ */
#if 0
    
  /* Configure LCD */
  #endif
}

/**
  * @brief  Disables the Display.
  * @param  None
  * @retval LCD Register Value.
  */
uint16_t ili9341_ReadID(void)
{
  //~ LCD_IO_Init();
  return ((uint16_t)ili9341_ReadData(LCD_READ_ID4, LCD_READ_ID4_SIZE));
}

/**
  * @brief  Enables the Display.
  * @param  None
  * @retval None
  */
void ili9341_DisplayOn(void)
{
  /* Display On */
  ili9341_WriteReg(LCD_DISPLAY_ON);
}

/**
  * @brief  Disables the Display.
  * @param  None
  * @retval None
  */
void ili9341_DisplayOff(void)
{
  /* Display Off */
  ili9341_WriteReg(LCD_DISPLAY_OFF);
}

/**
  * @brief  Writes  to the selected LCD register.
  * @param  LCD_Reg: address of the selected register.
  * @retval None
  */

/**
  * @brief  Reads the selected LCD Register.
  * @param  RegValue: Address of the register to read
  * @param  ReadSize: Number of bytes to read
  * @retval LCD Register Value.
  */
#ifdef    COLOR_3BYTES
inline void color_convert(uint16_t color,uint8_t* result)
{
	//~ #define RED             0xF800 
	//~ #define BLUE            0x001F
	//~ #define GREEN           0x07E0  
	result[2]=  ((color&0x1f)		<<(1+2))<<1;//|0x80;    //5 bit BLUE
	result[1]=  (((color>>5)&0x3f) <<(0+1))<<1;//|0x80;    //6 bit GREEN 
	result[0]=  (((color>>11)&0x1f)<<(1+2))<<1;//|0x80;    //5 bit  //RED
}
#endif



/**
  * @brief  Get LCD PIXEL WIDTH.
  * @param  None
  * @retval LCD PIXEL WIDTH.
  */
uint16_t ili9341_GetLcdPixelWidth(void)
{
  /* Return LCD PIXEL WIDTH */
  return ILI9341_LCD_PIXEL_WIDTH;
}

/**
  * @brief  Get LCD PIXEL HEIGHT.
  * @param  None
  * @retval LCD PIXEL HEIGHT.
  */
uint16_t ili9341_GetLcdPixelHeight(void)
{
  /* Return LCD PIXEL HEIGHT */
  return ILI9341_LCD_PIXEL_HEIGHT;
}


inline  void dmaSendDataCont16(uint16_t *data, uint32_t n) 
{
    int k;
    uint8_t dummy;
    uint8_t *pdata = (uint8_t *)data;
    for( k=0;k<n;k++)
    {
	    ili9341_WriteData(*(pdata+1));
	    ili9341_WriteData(*(pdata));
	    pdata+=2;
    }	    
}


void LCD_setAddressWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    uint16_t pointData[2];

    //~ TFT_CS_RESET;
    LCD_IO_WriteReg(LCD_COLUMN_ADDR);
    pointData[0] = x1;
    pointData[1] = x2;
    dmaSendDataCont16(pointData, 2);

    LCD_IO_WriteReg(LCD_PAGE_ADDR);
    pointData[0] = y1;
    pointData[1] = y2;
    dmaSendDataCont16(pointData, 2);
}


inline  void dmaFill16(uint16_t color, uint32_t n) {
    //~ TFT_CS_RESET;
    uint8_t dummy;
    LCD_IO_WriteReg(LCD_GRAM);
#ifdef    COLOR_3BYTES
     uint8_t pdata[3];
    color_convert(color,pdata);
    while (n != 0) 
    {
	dmaSendDataCont8(pdata,3);
	n--;    
    }
#else	
    uint8_t *pdata = (uint8_t *)&color;
    while (n != 0) 
       {
        dmaSendDataCont8(pdata,2);
	n--;    
    }
#endif	
}


void LCD_FullRect3(uint16_t x1, uint16_t y1,uint8_t * adress,uint16_t w,uint16_t h) 
{
        LCD_setAddressWindow(x1, y1, (uint16_t) (x1+w-1), (uint16_t) (y1+h-1));
    //~ LCD_setSpi16();
        uint8_t dummy;
       LCD_IO_WriteReg(LCD_GRAM);
#ifdef    COLOR_3BYTES
	int k;
	for(k=w*h;k>0;k--)
	{
		ili9341_WriteData(adress[0]);
		ili9341_WriteData(adress[1]);
		ili9341_WriteData(adress[2]);
		//~ adress+=3;
	}
#else	
#endif    
}


void LCD_fillRect(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint16_t color) {
     uint8_t pdata[3];
     color_convert(color,pdata);
     LCD_FullRect3(x1,y1,pdata,w,h); 
}


/**
  * @}
  */ 

/**
  * @}
  */ 
  
/**
  * @}
  */ 

/**
  * @}
  */
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
#endif
