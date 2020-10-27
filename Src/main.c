/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2020 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include "usb_host.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "ili9341.h"
#include "tm_stm32f1_nrf24l01.h"
#include <ctype.h>
#include "stm32_adafruit_sd.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;

DMA_HandleTypeDef hdma_memtomem_dma2_stream3;
DMA_HandleTypeDef hdma_memtomem_dma2_stream4;
SRAM_HandleTypeDef hsram1;

/* USER CODE BEGIN PV */
void retarget_put_char(char p)
{
	HAL_UART_Transmit(&huart2, &p, 1, 0xffffff); // send message via UART
}
int _write(int fd, char* ptr, int len)
{
    (void)fd;
    int i = 0;
    while (ptr[i] && (i < len))
    {
    	if (ptr[i] == '\r')
    	{

    	}
    	else
    	{
			retarget_put_char((int)ptr[i]);
			if (ptr[i] == '\n')
			{
				retarget_put_char((int)'\r');
			}
    	}
        i++;
    }
    return len;
}
void init_Sinclair();
#define N_SIZE_BITS (10)
#define N_SIZE (1<<N_SIZE_BITS)
#define R_SIZE_BITS (12)
#define R_SIZE (1<<R_SIZE_BITS)

//uint16_t VoiceBuff0s[N_SIZE];


uint16_t VoiceBuff0[N_SIZE];
uint16_t VoiceBuff1[N_SIZE];


uint16_t VoiceBuff0T[R_SIZE];
uint16_t VoiceBuff1T[R_SIZE];

volatile int sound_head = 0;
volatile int sound_tail = 0;

int sound_size()
{
	return (sound_head+R_SIZE-sound_tail)&(R_SIZE-1);
}

static TIM_HandleTypeDef TimMasterHandle;
int volume = 100;
int getVolume0()
{
	return volume;
}
void setVolume0(int vol)
{
	volume = vol;
}
int waitFor()
{
	int cntWait = 0;
	//TimMasterHandle.Instance = TIM2;
	while(((sound_head+R_SIZE-sound_tail)&(R_SIZE-1))>R_SIZE/2)
	{
		procKeyb();
		    // Disable HAL tick interrupt
		   // __HAL_TIM_DISABLE_IT(&TimMasterHandle, TIM_IT_CC2);

		    // Request to enter SLEEP mode
		    //HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);

		    // Enable HAL tick interrupt
		    //__HAL_TIM_ENABLE_IT(&TimMasterHandle, TIM_IT_CC2);
				//HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
		//HAL_Delay(1);
		cntWait++;
	}
	return cntWait;
}
void push_pair(uint16_t Left,uint16_t Right)
{
	waitFor();
	VoiceBuff0T[sound_head] = Left;
	VoiceBuff1T[sound_head] = Right;
	sound_head = (sound_head+1)&(R_SIZE-1);
}
void cleanAudioBuffer()
{
	waitFor();
	for(int k=0;k<R_SIZE;k++)
	{
#ifdef DAC_OUTPUT
			push_pair(2048,2048);
#else
			push_pair(MAX_VOLUME/2,MAX_VOLUME/2);
#endif
	}
}

void TIM1_TC1()
{
	//cnt0++;
	int k;
	for(k=N_SIZE/2;k<N_SIZE;k++)
	{
		VoiceBuff0[k] = VoiceBuff0T[sound_tail];
		VoiceBuff1[k] = VoiceBuff1T[sound_tail];
		sound_tail = (sound_tail+1)&(R_SIZE-1);
	}
}
void TIM1_HT1()
{
	//cnt1++;
	int k;
	for(k=0;k<N_SIZE/2;k++)
	{
		VoiceBuff0[k] = VoiceBuff0T[sound_tail];
		VoiceBuff1[k] = VoiceBuff1T[sound_tail];
		sound_tail = (sound_tail+1)&(R_SIZE-1);
	}
}
void TIM1_TE1()
{
}
void TIM1_TC2()
{
}
void TIM1_HT2()
{
}
void TIM1_TE2()
{
}
void setBK_imp(int perc)
{
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4,(perc*10));
}


void init_timerLL()
{
	  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);
	  LL_DMA_ConfigAddresses(DMA2, LL_DMA_STREAM_1, (uint32_t)&VoiceBuff0[0], (uint32_t)&TIM1->CCR1, LL_DMA_GetDataTransferDirection(DMA2, LL_DMA_STREAM_2));
	  LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_1, N_SIZE);
	  LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_1);
	  LL_DMA_EnableIT_TE(DMA2, LL_DMA_STREAM_1);
	  LL_DMA_EnableIT_HT(DMA2, LL_DMA_STREAM_1);

	  LL_DMA_ConfigAddresses(DMA2, LL_DMA_STREAM_2, (uint32_t)&VoiceBuff1[0], (uint32_t)&TIM1->CCR2, LL_DMA_GetDataTransferDirection(DMA2, LL_DMA_STREAM_2));
	  LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_2, N_SIZE);
	  LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_2);
	  LL_DMA_EnableIT_HT(DMA2, LL_DMA_STREAM_2);
	  LL_DMA_EnableIT_TE(DMA2, LL_DMA_STREAM_2);

	  /***************************/
	  /* Enable the DMA transfer */
	  /***************************/
	  LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_1);
	  LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_2);

	  LL_TIM_EnableDMAReq_UPDATE(TIM1);

	  LL_TIM_EnableDMAReq_CC1(TIM1);
	  LL_TIM_EnableDMAReq_CC2(TIM1);

	  LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH1);
	  LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH2);

	  LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH1);
	  LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH2);

	 // LL_TIM_OC_SetCompareCH3(TIM1,MAX_VOLUME*volume_perc/1000);

	  LL_TIM_EnableAllOutputs(TIM1);
	  LL_TIM_EnableCounter(TIM1);
	  //~ LL_TIM_EnableDMAReq_CC1(TIM2);
	 // LL_TIM_GenerateEvent_CC1(TIM4);
	 // LL_TIM_GenerateEvent_CC2(TIM4);

	  //~ LL_TIM_GenerateEvent_CC2(TIM2);
	  //~ LL_TIM_GenerateEvent_CC2(TIM2);
	  LL_TIM_GenerateEvent_UPDATE(TIM1);
//		LL_TIM_EnableCounter(TIM2);
//		LL_TIM_EnableAllOutputs(TIM2);


}

#define  SPI2_TIMEOUT 10
void SPI2_SEND(uint8_t* data,int count)
{
	HAL_SPI_Transmit(&hspi2,data,count,SPI2_TIMEOUT);
}
void SPI2_SEND_RECI(uint8_t* data,int count,uint8_t* in_data)
{
	HAL_SPI_TransmitReceive(&hspi2,data,in_data,count,SPI2_TIMEOUT);
}


volatile uint16_t keyMatrix  = 0;

//typedef void (*SignalHandler)(int signum);
typedef int  (*dispathFunction)(struct SYS_EVENT* ev);
#include "usbh_def.h"
#include "usbh_hid.h"
#include "usbh_hid_keybd.h"

extern uint8_t usb_mode;
extern USBH_HandleTypeDef hUsbHostFS;
uint8_t kstate = 0;

int dummyFunction(struct SYS_EVENT* ev)
{
	printf("dummy mess   = %x\r\n",ev->message);
	printf("dummy param1 = %x\r\n",ev->param1);
	printf("dummy param2 = %x\r\n",ev->param2);
	return 0;
}

void clearKey()
{
	keyMatrix = 0;
}


dispathFunction currFunc = &dummyFunction;
#define stackFuncSize 16
dispathFunction stackFunc[stackFuncSize];


int stackFuncP = 0;

void pushMenuFunc(void*fnk)
{
	if(stackFuncP<stackFuncSize-2)
	{
		stackFunc[stackFuncP] = fnk;
		stackFuncP++;
	}
}

dispathFunction popMenuFunc()
{
	if(stackFuncP>0)
	{
		stackFuncP--;
	}
	currFunc = stackFunc[stackFuncP];
	return currFunc;
}

void setCurrentFunc(void*fnk)
{
	pushMenuFunc(currFunc);
	currFunc = fnk;
}


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_RTC_Init(void);
static void MX_SPI1_Init(void);
static void MX_FSMC_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM3_Init(void);
static void MX_NVIC_Init(void);
void MX_USB_HOST_Process(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int bFlagCPM = 0;
int bNoDisk  = 0;
extern uint8_t  SD_BUFF[];
#include "z80.h"
extern int32_t tstates;
extern int32_t interrupts_enabled_at;
void rput(uint8_t data);
int capsFl = 9;
void init_Sinclair();
int menu_dispatch(struct SYS_EVENT* ev);
int z48_z128_dispatch(struct SYS_EVENT* ev);
//uint8_t kbs[256];
HID_KEYBD_Info_TypeDef prev_state_s;
void setTapeSpeed(int ts);
void kscan0()
{
	MX_USB_HOST_Process();
	kscan1();
}
volatile int bReady =1;
void kscan1()
{
    	if(usb_mode==HOST_USER_CLASS_ACTIVE)
    	{
			if(capsFl == 9)
			{
				kstate = 0x80;
				capsFl = 0;
			}
    		HID_KEYBD_Info_TypeDef * ki=USBH_HID_GetKeybdInfo(&hUsbHostFS);
    		//int reps = 1;
    		//while(ki && reps>0)
    		if(ki)
    		{
    			//reps --;
				prev_state_s = *ki;
#if 0
				printf("%d %d %d %x : %x %x %x %x %x%x%x%x%x%x%x%x\n",
												delta0,
												delta1,
												delta2,
												ki->state,
												ki->keys[0],
												ki->keys[1],
												ki->keys[2],
												ki->keys[3],
												ki->lctrl,
												ki->lshift,
												ki->lalt,
												ki->lgui,
												ki->rctrl,
												ki->rshift,
												ki->ralt,
												ki->rgui);

				char c = USBH_HID_GetASCIICode(ki);
				printf("%x : %x %x %x %x %x%x%x%x%x%x%x%x\n",
												ki->state,
												ki->keys[0],
												ki->keys[1],
												ki->keys[2],
												ki->keys[3],
												ki->lctrl,
												ki->lshift,
												ki->lalt,
												ki->lgui,
												ki->rctrl,
												ki->rshift,
												ki->ralt,
												ki->rgui);
#endif
				if(bFlagCPM)
				{
					char c = USBH_HID_GetASCIICode(ki);
					if (c==0)
					{
						bReady = 1;
						if(ki->keys[0]==KEY_ESCAPE)
						{
							    printf("KEY_ESCAPE\n");
								rput(0x1b);
						}
						else if(ki->keys[0]==KEY_DELETE)
						{
							   printf("KEY_DELETE\n");
								rput(0x7f);
						}
						else
						{
							if(ki->keys[0])
							printf("press = %x\n",ki->keys[0]);
						}
						return;
				     }
					if(!bReady) return;
					bReady = 0;
					//if(c)
						//rput(c);
					printf("Key = %x %x\n",c,ki->keys[0]);

					if(c=='\n')
					{
						     //printf("%x %x \n",'\n','\r');
						     // \n -0xa NL line feed
						     // \r -0xd CR

							//rput('\n');
							rput('\r');
							//http://www.asciitable.com/
					}
					else if (c==0xa && ki->keys[0]==KEY_BACKSPACE)
					{
							rput(0x8);
					}
					else
					{
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

						if(ki->lctrl || ki->rctrl)
						{
								if(c=='a') 		{rput(CTRL_A);}
								else if(c=='s')	{rput(CTRL_S);}
								else if(c=='d')	{rput(CTRL_D);}
								else if(c=='f')	{rput(CTRL_F);}
								else if(c=='g')	{rput(CTRL_G);}
								else if(c=='h')	{rput(CTRL_H);}
								else if(c=='j')	{rput(CTRL_J);}
								else if(c=='k')	{rput(CTRL_K);}
								else if(c=='l')	{rput(CTRL_L);}

								else if(c=='q')	{rput(CTRL_Q);}
								else if(c=='w')	{rput(CTRL_W);}
								else if(c=='e')	{rput(CTRL_E);}
								else if(c=='r')	{rput(CTRL_R);}
								else if(c=='t')	{rput(CTRL_T);}
								else if(c=='y')	{rput(CTRL_Y);}
								else if(c=='u')	{rput(CTRL_U);}
								else if(c=='i')	{rput(CTRL_I);}
								else if(c=='o')	{rput(CTRL_O);}
								else if(c=='p')	{rput(CTRL_P);}

								else if(c=='z')	{rput(CTRL_Z);}
								else if(c=='x')	{rput(CTRL_X);}
								else if(c=='c')	{rput(CTRL_C);}
								else if(c=='v')	{rput(CTRL_V);}
								else if(c=='b')	{rput(CTRL_B);}
								else if(c=='n')	{rput(CTRL_N);}
								else if(c=='m')	{rput(CTRL_M);}
						}
						else
						{
							if(c)      rput(c) ;
						}
					}
				}
				else
				{
					for(int k=0;k<1;k++)
					{
						switch(ki->keys[k])
						{
							case  KEY_RIGHTARROW: 	keyMatrix = K_RIGHT;break;
							case  KEY_LEFTARROW: 	keyMatrix = K_LEFT;break;
							case  KEY_DOWNARROW: 	keyMatrix = K_DOWN;break;
							case  KEY_UPARROW: 		keyMatrix = K_UP;break;
							case  KEY_F10: 			setTapeSpeed(0);break;
							case  KEY_F11: 			setTapeSpeed(1);break;
							case  KEY_F12: 			setTapeSpeed(2);break;

							case  0x0:  			keyMatrix = 0;break;
							case  KEY_ENTER:  		keyMatrix = K_FIRE;break;//enterbreak;
							case  KEY_SPACEBAR: 	keyMatrix = K_SPACE;break;
							case  KEY_ESCAPE:		keyMatrix = K_ESC;break;
							case  KEY_F7:  			setScreenOffset(0);break;
							case  KEY_F8:  			setScreenOffset(1);break;
							case  KEY_F9:  			setScreenOffset(2);break;
							case  KEY_F6:
											initRam();
											ay_reset();
											z80_reset(1);
											SKEY_QUE_init();
											setScreenOffset(0);
										break;
						};
					}
				}
				if(ki->keys[0]==0x39)
				{
					if(capsFl&2)
					{
						capsFl&=~2;
					}
					else
					{
						capsFl|=2;
					}
					kstate = 0x80 | capsFl;
				}
				if(ki->keys[0]==0x53)
				{
					if(capsFl&1)
					{
						capsFl&=~1;
					}
					else
					{
						capsFl|=1;
					}
					kstate = 0x80 | capsFl;
				}
				if(ki->keys[0]==0x47)
				{
					if(capsFl&4)
					{
						capsFl&=~4;
					}
					else
					{
						capsFl|=4;
					}
					kstate = 0x80 | capsFl;
				}
			//	ki = USBH_HID_GetKeybdInfo(&hUsbHostFS);
		}
		if(kstate&0x80)
		{
			    printf("Go set state %x\n",kstate);
		    	kstate &= 0x7f;
		    	int tmOut = 1000;
		    	while(tmOut>0&&USBH_HID_SetReport(&hUsbHostFS,0x2,0,&kstate,1)!=USBH_OK)
		    	{
		    		tmOut--;
		    	};
		    	printf("state tm %x\n",tmOut);
		}
	}
	else
	{
		  capsFl = 9;
	}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  __HAL_RCC_PWR_CLK_ENABLE();
  //__HAL_RCC_BKP_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_RTC_Init();
  MX_SPI1_Init();
  MX_FATFS_Init();
  MX_FSMC_Init();
  MX_SPI2_Init();
  MX_TIM1_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  MX_USB_HOST_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */
  //HAL_Delay(1);
    rclear();
    printf("\n\n\n Program Start\n");
    printf("HAL_RCC_GetHCLKFreq = %d\n",HAL_RCC_GetHCLKFreq());
    printf("HAL_RCC_GetPCLK1Freq = %d\n",HAL_RCC_GetPCLK1Freq());
	if(HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_4)!= HAL_OK)
	{
		printf("set TIM1 TIM_CHANNEL_3 error\r\n");
	}
	setBK_imp(0);
	//memset(&ADC_BUFF[0],0,sizeof(ADC_BUFF));
	//HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC_BUFF, HSAMPLES*NCHAN);
	init_timerLL();
	LCD_init();
	//ili9341_DisplayOn();
	LCD_setRotation(LANDSCAPE_FLIP);
	uint32_t trt = HAL_GetTick();
	int k;
	setBK_imp(100);
	for(k=0;k<2;k++)
	{
		LCD_fillRect(0, 0, LCD_getWidth(), LCD_getHeight(), BLACK);
		LCD_fillRect(0, 0, LCD_getWidth(), LCD_getHeight(), WHITE);
	}
	//TP_init_default();
	//setBK_imp(99);
	printf("clear screen %d ms\r\n",(HAL_GetTick()-trt)/4);
	LCD_fillRect(0, 0, LCD_getWidth(), LCD_getHeight(), BLACK);

	Delay(1);
	//SD_initialize(0);
	uint8_t sd_state= BSP_SD_Init();
	printf("sd_state = %x\n",sd_state);

	BSP_SD_CardInfo CardInfo;
	BSP_SD_GetCardInfo(&CardInfo);
   printf("hsd.CardCapacity =%d\n",CardInfo.CardCapacity);
   //printf("hsd.SdCard.BlockNbr=%d\n",hsd.SdCard.BlockNbr);
   //printf("hsd.SdCard.BlockSize=%d\n",hsd.SdCard.BlockSize);
   //printf("hsd.SdCard.CardType=%d\n",hsd.SdCard.CardType);
   //printf("hsd.SdCard.CardVersion=%d\n",hsd.SdCard.CardVersion);
   //printf("hsd.SdCard.Class=%d\n",hsd.SdCard.Class);
   printf("hsd.CardBlockSize =%d\n",CardInfo.CardBlockSize);
   printf("hsd.SdCard.LogBlockNbr=%d\n",CardInfo.LogBlockNbr);
   printf("hsd.SdCard.LogBlockSize=%d\n",CardInfo.LogBlockSize);
   uint64_t size_k = CardInfo.LogBlockNbr;
   size_k = size_k*CardInfo.LogBlockSize;

   printf("CardInfo size %d MB\n",(int)(size_k/1024/1024));

	Delay(1);
	int SD_TIMEOUT = 1000;
	  {
		  int32_t tk = HAL_GetTick();
		  int stat = 0;
		  int kk;
		  for(kk=0;(kk<8)&&!stat;kk++)
		  {
	  			stat = BSP_SD_ReadBlocks(SD_BUFF,kk,1, SD_TIMEOUT);
		  }
		  tk = HAL_GetTick()-tk;
		  Delay(1);
		  stat = BSP_SD_ReadBlocks(SD_BUFF,0,1, SD_TIMEOUT);
		  if(!stat)
		  {
			  bNoDisk = 0;
		  }
		  else
		  {
			  bNoDisk = 1;
		  }
		  printf("Read %d blocks (bytes = %d) in %d ms st = %d\n",kk,kk*512,tk,stat);
		  for(int k=0;k<4;k++)
		  {
			  if(isalnum(SD_BUFF[k]))
			  {
				  printf("\n%02x %c",k,SD_BUFF[k]);
			  }
			  else
			  {
				  printf("\n%02x %02x",k,SD_BUFF[k]);
			  }
		  }
		  printf("\nEndPrint\n");
		  bFlagCPM = 0;
		  // cpm   ?      	 //0xc3 0x5c 0xd3 0xc3
		  // formated part ? //0xfa 0xb8 0x00 0x10
		  if(SD_BUFF[0]==0xc3 && SD_BUFF[1]==0x5c)
		  {
			  bFlagCPM = 1;
		  }
		  cleanAudioBuffer();
		//  paint();
	  }
	volatile struct SYS_EVENT event;
	event.message = MESS_OPEN;
	event.param1  = 0x1234;
	event.param2  = 0x5678;
	  if(bFlagCPM)
	  {
		  init_cpm();
		  z80_reset(1);
	  }
	  else
	  {
		init_Sinclair();
		if(bNoDisk)
		{
				LCD_fillRect(0, 0, LCD_getWidth(), LCD_getHeight(), BLACK);
				setModeFile(1);
				setCurrentFunc(&z48_z128_dispatch);
		}
		else
		{
				setCurrentFunc(&menu_dispatch);
		}
	  }
//	int nrfOnline = nRF24_Check();
//	sprintf(numBuff,"nrfOnline %d\r\n",nrfOnline);
//	mprintf(numBuff);
//	LCD_Draw_Text(numBuff,100,100, GREEN, 1,BLACK);
//	nrfOnline = nRF24_Check();
//	sprintf(numBuff,"nrfOnline %d\r\n",nrfOnline);
//	mprintf(numBuff);
//	LCD_Draw_Text(numBuff,100,100, GREEN, 1,BLACK);
	//clearFullScreen();
	//readCard1();
	//stackFunc[0] = ;
	//currFunc = &menu_dispatch;
	  uint32_t prevtk =  HAL_GetTick()+20;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
#if 1
	  if(bFlagCPM)
	  {
			tstates               = 0;
			uint32_t tk = HAL_GetTick()-prevtk;
			if(!(tk&0x80000000U))
			{
			  prevtk += 20;
			}
			for(int tt = 0;tt<70908;tt+=1000)
			{
				z80_run(tt);
				procKeyb();
			}
			z80_run(70908);

			paint();
			screen_IRQ = 1;
			//interrupts_enabled_at = 5;
	  }
	  else
	  {
		  //mprintf("cnt t3 %d %x %x %x\r\n ",tim3_intrr,keyMatrix,TIM3->CNT,TIM1->CNT);
		  if(keyMatrix&&(event.message==MESS_IDLE))
		  {
			  event.param1 = keyMatrix;
				  event.param2 = HAL_GetTick();
			  event.message = MESS_KEYBOARD;
			  //printf("cnt t3 %d %x %x %x %x %x\r\n ",tim3_intrr,keyMatrix,TIM3->CNT,TIM1->CNT,ADC_BUFF[0].V_REF,ADC_BUFF[0].V_LR);
		  }
		  else
		  {
			  HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
		  }
		  (*currFunc)(&event);
		  if(event.message==MESS_CLOSE)
		  {
			event.message = MESS_REPAINT;
			printf("stackFuncP B =%d\r\n",stackFuncP);
			popMenuFunc();
			printf("stackFuncP A b=%d\r\n",stackFuncP);
			//currFunc = &menu_dispatch;
		  }
	  }
#endif
	  //HAL_Delay(100);
    /* USER CODE END WHILE */
    MX_USB_HOST_Process();

    /* USER CODE BEGIN 3 */
    kscan1();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /**Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* DMA2_Stream1_IRQn interrupt configuration */
  NVIC_SetPriority(DMA2_Stream1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(DMA2_Stream1_IRQn);
  /* DMA2_Stream2_IRQn interrupt configuration */
  NVIC_SetPriority(DMA2_Stream2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(DMA2_Stream2_IRQn);
  /* TIM3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM3_IRQn);
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */
  hrtc.Instance = RTC;
  if(HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) != 0x32F2)
  {

  /* USER CODE END RTC_Init 1 */
  /**Initialize RTC Only 
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */
    
  /* USER CODE END Check_RTC_BKUP */

  /**Initialize RTC and set the Time and Date 
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */
  HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR1,0x32F2);
 }

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};
  LL_TIM_BDTR_InitTypeDef TIM_BDTRInitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);

  /* TIM1 DMA Init */
  
  /* TIM1_CH1 Init */
  LL_DMA_SetChannelSelection(DMA2, LL_DMA_STREAM_1, LL_DMA_CHANNEL_6);

  LL_DMA_SetDataTransferDirection(DMA2, LL_DMA_STREAM_1, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

  LL_DMA_SetStreamPriorityLevel(DMA2, LL_DMA_STREAM_1, LL_DMA_PRIORITY_HIGH);

  LL_DMA_SetMode(DMA2, LL_DMA_STREAM_1, LL_DMA_MODE_CIRCULAR);

  LL_DMA_SetPeriphIncMode(DMA2, LL_DMA_STREAM_1, LL_DMA_PERIPH_NOINCREMENT);

  LL_DMA_SetMemoryIncMode(DMA2, LL_DMA_STREAM_1, LL_DMA_MEMORY_INCREMENT);

  LL_DMA_SetPeriphSize(DMA2, LL_DMA_STREAM_1, LL_DMA_PDATAALIGN_HALFWORD);

  LL_DMA_SetMemorySize(DMA2, LL_DMA_STREAM_1, LL_DMA_MDATAALIGN_HALFWORD);

  LL_DMA_EnableFifoMode(DMA2, LL_DMA_STREAM_1);

  LL_DMA_SetFIFOThreshold(DMA2, LL_DMA_STREAM_1, LL_DMA_FIFOTHRESHOLD_FULL);

  LL_DMA_SetMemoryBurstxfer(DMA2, LL_DMA_STREAM_1, LL_DMA_MBURST_SINGLE);

  LL_DMA_SetPeriphBurstxfer(DMA2, LL_DMA_STREAM_1, LL_DMA_PBURST_SINGLE);

  /* TIM1_CH2 Init */
  LL_DMA_SetChannelSelection(DMA2, LL_DMA_STREAM_2, LL_DMA_CHANNEL_6);

  LL_DMA_SetDataTransferDirection(DMA2, LL_DMA_STREAM_2, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

  LL_DMA_SetStreamPriorityLevel(DMA2, LL_DMA_STREAM_2, LL_DMA_PRIORITY_HIGH);

  LL_DMA_SetMode(DMA2, LL_DMA_STREAM_2, LL_DMA_MODE_CIRCULAR);

  LL_DMA_SetPeriphIncMode(DMA2, LL_DMA_STREAM_2, LL_DMA_PERIPH_NOINCREMENT);

  LL_DMA_SetMemoryIncMode(DMA2, LL_DMA_STREAM_2, LL_DMA_MEMORY_INCREMENT);

  LL_DMA_SetPeriphSize(DMA2, LL_DMA_STREAM_2, LL_DMA_PDATAALIGN_HALFWORD);

  LL_DMA_SetMemorySize(DMA2, LL_DMA_STREAM_2, LL_DMA_MDATAALIGN_HALFWORD);

  LL_DMA_EnableFifoMode(DMA2, LL_DMA_STREAM_2);

  LL_DMA_SetFIFOThreshold(DMA2, LL_DMA_STREAM_2, LL_DMA_FIFOTHRESHOLD_FULL);

  LL_DMA_SetMemoryBurstxfer(DMA2, LL_DMA_STREAM_2, LL_DMA_MBURST_SINGLE);

  LL_DMA_SetPeriphBurstxfer(DMA2, LL_DMA_STREAM_2, LL_DMA_PBURST_SINGLE);

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  TIM_InitStruct.Prescaler = 0;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 3809;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  TIM_InitStruct.RepetitionCounter = 0;
  LL_TIM_Init(TIM1, &TIM_InitStruct);
  LL_TIM_SetClockSource(TIM1, LL_TIM_CLOCKSOURCE_INTERNAL);
  LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH1);
  TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.CompareValue = 0;
  TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
  TIM_OC_InitStruct.OCNPolarity = LL_TIM_OCPOLARITY_HIGH;
  TIM_OC_InitStruct.OCIdleState = LL_TIM_OCIDLESTATE_LOW;
  TIM_OC_InitStruct.OCNIdleState = LL_TIM_OCIDLESTATE_LOW;
  LL_TIM_OC_Init(TIM1, LL_TIM_CHANNEL_CH1, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM1, LL_TIM_CHANNEL_CH1);
  LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH2);
  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
  LL_TIM_OC_Init(TIM1, LL_TIM_CHANNEL_CH2, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM1, LL_TIM_CHANNEL_CH2);
  LL_TIM_SetTriggerOutput(TIM1, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM1);
  TIM_BDTRInitStruct.OSSRState = LL_TIM_OSSR_DISABLE;
  TIM_BDTRInitStruct.OSSIState = LL_TIM_OSSI_DISABLE;
  TIM_BDTRInitStruct.LockLevel = LL_TIM_LOCKLEVEL_OFF;
  TIM_BDTRInitStruct.DeadTime = 0;
  TIM_BDTRInitStruct.BreakState = LL_TIM_BREAK_DISABLE;
  TIM_BDTRInitStruct.BreakPolarity = LL_TIM_BREAK_POLARITY_HIGH;
  TIM_BDTRInitStruct.AutomaticOutput = LL_TIM_AUTOMATICOUTPUT_DISABLE;
  LL_TIM_BDTR_Init(TIM1, &TIM_BDTRInitStruct);
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  /**TIM1 GPIO Configuration  
  PA8   ------> TIM1_CH1
  PA9   ------> TIM1_CH2 
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_8|LL_GPIO_PIN_9;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 84-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1000-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 500-1;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/** 
  * Enable DMA controller clock
  * Configure DMA for memory to memory transfers
  *   hdma_memtomem_dma2_stream3
  *   hdma_memtomem_dma2_stream4
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* Configure DMA request hdma_memtomem_dma2_stream3 on DMA2_Stream3 */
  hdma_memtomem_dma2_stream3.Instance = DMA2_Stream3;
  hdma_memtomem_dma2_stream3.Init.Channel = DMA_CHANNEL_0;
  hdma_memtomem_dma2_stream3.Init.Direction = DMA_MEMORY_TO_MEMORY;
  hdma_memtomem_dma2_stream3.Init.PeriphInc = DMA_PINC_ENABLE;
  hdma_memtomem_dma2_stream3.Init.MemInc = DMA_MINC_DISABLE;
  hdma_memtomem_dma2_stream3.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  hdma_memtomem_dma2_stream3.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
  hdma_memtomem_dma2_stream3.Init.Mode = DMA_NORMAL;
  hdma_memtomem_dma2_stream3.Init.Priority = DMA_PRIORITY_LOW;
  hdma_memtomem_dma2_stream3.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
  hdma_memtomem_dma2_stream3.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
  hdma_memtomem_dma2_stream3.Init.MemBurst = DMA_MBURST_SINGLE;
  hdma_memtomem_dma2_stream3.Init.PeriphBurst = DMA_PBURST_SINGLE;
  if (HAL_DMA_Init(&hdma_memtomem_dma2_stream3) != HAL_OK)
  {
    Error_Handler( );
  }

  /* Configure DMA request hdma_memtomem_dma2_stream4 on DMA2_Stream4 */
  hdma_memtomem_dma2_stream4.Instance = DMA2_Stream4;
  hdma_memtomem_dma2_stream4.Init.Channel = DMA_CHANNEL_0;
  hdma_memtomem_dma2_stream4.Init.Direction = DMA_MEMORY_TO_MEMORY;
  hdma_memtomem_dma2_stream4.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_memtomem_dma2_stream4.Init.MemInc = DMA_MINC_DISABLE;
  hdma_memtomem_dma2_stream4.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  hdma_memtomem_dma2_stream4.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
  hdma_memtomem_dma2_stream4.Init.Mode = DMA_NORMAL;
  hdma_memtomem_dma2_stream4.Init.Priority = DMA_PRIORITY_LOW;
  hdma_memtomem_dma2_stream4.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
  hdma_memtomem_dma2_stream4.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
  hdma_memtomem_dma2_stream4.Init.MemBurst = DMA_MBURST_SINGLE;
  hdma_memtomem_dma2_stream4.Init.PeriphBurst = DMA_PBURST_SINGLE;
  if (HAL_DMA_Init(&hdma_memtomem_dma2_stream4) != HAL_OK)
  {
    Error_Handler( );
  }

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED1_Pin|LED2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, FLASH_CS_Pin|NRF_CE_Pin|NRF_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(TOUCH_CS_GPIO_Port, TOUCH_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, M_CS_Pin|M_CLK_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(M_MOSI_GPIO_Port, M_MOSI_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : KEY1_Pin KEY0_Pin */
  GPIO_InitStruct.Pin = KEY1_Pin|KEY0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : PIN_WKUP_Pin */
  GPIO_InitStruct.Pin = PIN_WKUP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(PIN_WKUP_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED1_Pin LED2_Pin */
  GPIO_InitStruct.Pin = LED1_Pin|LED2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : JY_KEY_Pin */
  GPIO_InitStruct.Pin = JY_KEY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(JY_KEY_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : T_PEN_Pin */
  GPIO_InitStruct.Pin = T_PEN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(T_PEN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : FLASH_CS_Pin NRF_CE_Pin NRF_CS_Pin */
  GPIO_InitStruct.Pin = FLASH_CS_Pin|NRF_CE_Pin|NRF_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : TOUCH_CS_Pin */
  GPIO_InitStruct.Pin = TOUCH_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(TOUCH_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : M_MISO_Pin */
  GPIO_InitStruct.Pin = M_MISO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(M_MISO_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : M_CS_Pin M_CLK_Pin */
  GPIO_InitStruct.Pin = M_CS_Pin|M_CLK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : M_MOSI_Pin */
  GPIO_InitStruct.Pin = M_MOSI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(M_MOSI_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : NRF_IRQ_Pin */
  GPIO_InitStruct.Pin = NRF_IRQ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(NRF_IRQ_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* FSMC initialization function */
static void MX_FSMC_Init(void)
{
  FSMC_NORSRAM_TimingTypeDef Timing;
  FSMC_NORSRAM_TimingTypeDef ExtTiming;

  /** Perform the SRAM1 memory initialization sequence
  */
  hsram1.Instance = FSMC_NORSRAM_DEVICE;
  hsram1.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
  /* hsram1.Init */
  hsram1.Init.NSBank = FSMC_NORSRAM_BANK1;
  hsram1.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
  hsram1.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
  hsram1.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
  hsram1.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
  hsram1.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram1.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
  hsram1.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
  hsram1.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
  hsram1.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
  hsram1.Init.ExtendedMode = FSMC_EXTENDED_MODE_ENABLE;
  hsram1.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram1.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
  hsram1.Init.PageSize = FSMC_PAGE_SIZE_NONE;
  /* Timing */
  Timing.AddressSetupTime = 1;
  Timing.AddressHoldTime = 15;
  Timing.DataSetupTime = 15;
  Timing.BusTurnAroundDuration = 0;
  Timing.CLKDivision = 16;
  Timing.DataLatency = 17;
  Timing.AccessMode = FSMC_ACCESS_MODE_A;
  /* ExtTiming */
  ExtTiming.AddressSetupTime = 2;
  ExtTiming.AddressHoldTime = 15;
  ExtTiming.DataSetupTime = 5;
  ExtTiming.BusTurnAroundDuration = 0;
  ExtTiming.CLKDivision = 16;
  ExtTiming.DataLatency = 17;
  ExtTiming.AccessMode = FSMC_ACCESS_MODE_A;

  if (HAL_SRAM_Init(&hsram1, &Timing, &ExtTiming) != HAL_OK)
  {
    Error_Handler( );
  }

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM2 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM2) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
