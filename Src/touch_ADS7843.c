#include "stm32f4xx_hal.h"
#include "main.h"
#include "ili9341.h"
uint16_t touch_xvalues[7];
uint16_t touch_yvalues[7];
// custom printf() function
#define TOUCH_CS_LOW() HAL_GPIO_WritePin(TOUCH_CS_GPIO_Port,TOUCH_CS_Pin,GPIO_PIN_RESET)
#define TOUCH_CS_HIGH() HAL_GPIO_WritePin(TOUCH_CS_GPIO_Port,TOUCH_CS_Pin,GPIO_PIN_SET)
void SPI2_SEND(uint8_t* data,int count);
void SPI2_SEND_RECI(uint8_t* data,int count,uint8_t* in_data);

enum ControlBits {
  START = 0x80,
  A2    = 0x40,
  A1    = 0x20,
  A0    = 0x10,
  MODE1 = 0x8,
  MODE0 = 0x4,
  PD1   = 0x2,
  PD0   = 0x1,

  // channels from the above combinations

  ChannelX =  A0,
  ChannelY =  A2 | A0
};

void getSamples(uint8_t firstCommand,uint8_t lastCommand,uint16_t *values,uint16_t sampleCount)
{

  uint8_t dummy,data[2] = { 0 };

  dummy=0;

  SPI2_SEND(&firstCommand,1);
  SPI2_SEND(&dummy,1);
  SPI2_SEND(&dummy,1);

  while(sampleCount--!=1) {

    SPI2_SEND(&firstCommand,1);

    SPI2_SEND_RECI(&dummy,1,&data[0]);
    SPI2_SEND_RECI(&dummy,1,&data[1]);

    // add the 12 bit response. The MSB of the first response is discarded
    // because it's a NUL bit output during the BUSY line period

    *values++=(uint16_t)(data[0] & 0x7f) << 5 | (data[1] >> 3);
  }

  // the last sample

  SPI2_SEND(&lastCommand,1);

  SPI2_SEND_RECI(&dummy,1,&data[0]);
  SPI2_SEND_RECI(&dummy,1,&data[1]);

  *values=(uint16_t)(data[0] & 0x7f) << 5 | (data[1] >> 3);
}


void getSamplesA(uint16_t *xvalues,uint16_t *yvalues,uint16_t sampleCount)
{
	TOUCH_CS_LOW();
	getSamples(ChannelX | START | PD0 | PD1,ChannelX | START | PD0 | PD1,xvalues,sampleCount);
	getSamples(ChannelY | START | PD0 | PD1,ChannelY | START,yvalues,sampleCount);
	TOUCH_CS_HIGH();
}

uint16_t fastMedian7(uint16_t* samples)
{
	#define PIX_SORT(a,b) { if ((a)>(b)) PIX_SWAP((a),(b)); }
	#define PIX_SWAP(a,b) { uint16_t temp=(a);(a)=(b);(b)=temp; }

	PIX_SORT(samples[0], samples[5]) ; PIX_SORT(samples[0], samples[3]) ; PIX_SORT(samples[1], samples[6]) ;
	PIX_SORT(samples[2], samples[4]) ; PIX_SORT(samples[0], samples[1]) ; PIX_SORT(samples[3], samples[5]) ;
	PIX_SORT(samples[2], samples[6]) ; PIX_SORT(samples[2], samples[3]) ; PIX_SORT(samples[3], samples[6]) ;
	PIX_SORT(samples[4], samples[5]) ; PIX_SORT(samples[1], samples[4]) ; PIX_SORT(samples[1], samples[3]) ;
	PIX_SORT(samples[3], samples[4]) ;

	return samples[3];
}
uint16_t fastMedian3(uint16_t* samples)
{
	#define PIX_SORT(a,b) { if ((a)>(b)) PIX_SWAP((a),(b)); }
	#define PIX_SWAP(a,b) { uint16_t temp=(a);(a)=(b);(b)=temp; }
	PIX_SORT(samples[0], samples[1]) ;
	PIX_SORT(samples[1], samples[2]) ;
	return samples[1];
}
uint16_t fastSumm2(uint16_t* samples)
{
	return (((uint32_t)samples[0])+samples[1])/2;
}
typedef float FLOAT;
typedef struct
{
	FLOAT Transform[2][3];
} sTouchCalibration;

sTouchCalibration TouchCalibration={-0.088146,0.000483,328.99,-0.000581,-0.064943,258.183};


sTouchCalibration* getCalibr()
{
	return &TouchCalibration;
}

int TP_Touchpad_Pressed()
{
	return HAL_GPIO_ReadPin(T_PEN_GPIO_Port, T_PEN_Pin)==GPIO_PIN_RESET;
}
int sample_touch(int32_t Coordinates[2])
{
	uint16_t xx[3];
	uint16_t yy[3];
	if(HAL_GPIO_ReadPin(T_PEN_GPIO_Port, T_PEN_Pin)==GPIO_PIN_RESET)
	{
		//mprintf("go Sample \r\n");
		for(int j=0;j<2;j++)
		{
			getSamplesA(touch_xvalues,touch_yvalues,7);
			xx[j] = fastMedian7(touch_xvalues);
			yy[j] = fastMedian7(touch_yvalues);
		}
	//	int s;
	//	for(s=0;s<7;s++)
	//	{
	//		mprintf("Sample %04x,%04x\r\n",touch_xvalues[s],touch_yvalues[s]);
	//	}
		//uint16_t  calculating_x = fastMedian3(xx);
		//uint16_t  calculating_y = fastMedian3(yy);
		uint16_t  calculating_x = fastSumm2(xx);
		uint16_t  calculating_y = fastSumm2(yy);


		//mprintf("Sample %04x,%04x\r\n",fastMedian(touch_xvalues),fastMedian(touch_yvalues));
		if(HAL_GPIO_ReadPin(T_PEN_GPIO_Port, T_PEN_Pin)==GPIO_PIN_RESET)
		{
			Coordinates[0] = TouchCalibration.Transform[0][0]*calculating_x+TouchCalibration.Transform[0][1]*calculating_y+TouchCalibration.Transform[0][2];
			Coordinates[1] = TouchCalibration.Transform[1][0]*calculating_x+TouchCalibration.Transform[1][1]*calculating_y+TouchCalibration.Transform[1][2];
			return 1;
		}
		else return 0;
	}
	else
	{
		//Coordinates[0] = -1000;
		//Coordinates[0] = -1000;
		return 0;
	}

}


void paintCross(int32_t x,int32_t y,int32_t size,int32_t color)
{
	LCD_fillRect(x,y-size/2,1,size,color);
	LCD_fillRect(x-size/2,y,size,1,color);
}

inline FLOAT det3  (
			FLOAT a,FLOAT b,FLOAT c,
			FLOAT d,FLOAT e,FLOAT f,
			FLOAT g,FLOAT h,FLOAT i
			)
{
	return a*e*i +b*f*g+ c*d*h - a*f*h - b*d*i - c*e*g;
}

inline FLOAT mat3x3Invert(FLOAT *p,FLOAT* F)
{
		const  FLOAT  b11= (p[0]);
		const  FLOAT  b12 =(p[1]);
		const  FLOAT  b13 =(p[2]);
		const  FLOAT  b21 =(p[3]);
		const  FLOAT  b22 =(p[4]);
		const  FLOAT  b23 =(p[5]);
		const  FLOAT  b31 =(p[6]);
		const  FLOAT  b32 =(p[7]);
		const  FLOAT  b33 =(p[8]);
		F[0*3+0] = (b22*b33-b32*b23);F[0*3+1] = (b13*b32-b33*b12);F[0*3+2] = (b12*b23-b22*b13);
		F[1*3+0] = (b23*b31-b33*b21);F[1*3+1] = (b11*b33-b31*b13);F[1*3+2] = (b13*b21-b23*b11);
		F[2*3+0] = (b21*b32-b31*b22);F[2*3+1] = (b12*b31-b32*b11);F[2*3+2] = (b11*b22-b21*b12);
		return det3(
							b11,b12,b13,
							b21,b22,b23,
							b31,b32,b33
						);;
}

#define N_MESU 9

void checkStop(sTouchCalibration* calib)
{
		{
			LCD_fillRect(0,0,LCD_getWidth(),LCD_getHeight(),BLACK);
			int X,Y;
			char buff[0x20];
			LCD_Draw_Text("results:",10,200, GREEN, 2, BLACK);
			for(Y=0;Y<2;Y++)
			{
				for(X=0;X<3;X++)
				{
					FLOAT val = calib->Transform[Y][X];
					sprintf(buff,"%f",val);
					LCD_Draw_Text(buff,10,(Y*3+X)*30, GREEN, 2, BLACK);
				}
			}
			while(!TP_Touchpad_Pressed());
		}
}

int testTouch_dispatch(struct SYS_EVENT* ev)
{
	if(ev->message==MESS_OPEN)
	{
		LCD_fillRect(0,0,LCD_getWidth(),LCD_getHeight(),BLACK);
		ev->message = MESS_IDLE;
		return 1;
	}
	else if(ev->message==MESS_IDLE)
	{
		int32_t Coordinates[2] = {-1,-1};
		if(sample_touch(Coordinates))
		{
			paintCross(Coordinates[0],Coordinates[1],7,GREEN);
		}
		return 1;
	}
	else if(ev->message==MESS_KEYBOARD)
	{
		if(ev->param1==K_ESC)
		{
			ev->message = MESS_CLOSE;
			return 0;
		}
		int32_t Coordinates[2] = {-1,-1};
		if(sample_touch(Coordinates))
		{
			paintCross(Coordinates[0],Coordinates[1],7,YELLOW);
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

void ReCalibrateTouch(int force)
{
FLOAT VTP[3][N_MESU];
FLOAT RTP[3][N_MESU];
FLOAT VTP_VTP_T_I[3][3];
FLOAT RTP_VTP_T[3][3];
FLOAT VTP_VTP_T[3][3];
char printBuff[0x10];
	sTouchCalibration* calib = getCalibr();
	int32_t Coordinates[2];
	int32_t i;
	int32_t j;
	//~ LCD_fillRect(0,0,LCD_getWidth(),LCD_getHeight(),BLACK);
	//~ LCD_Draw_Text(printNum(sizeof(float)),30,LCD_getWidth()/4, YELLOW, 4, BLACK);
	//~ LCD_Draw_Text(printNum(sizeof(sTouchCalibration)),130,LCD_getWidth()/4, YELLOW, 4, BLACK);
	//~ HAL_Delay(20000);
	//~ if(HAL_RTCEx_BKUPRead(&hrtc,RTC_BKP_DR7)==0x1235)
	//~ {
		//~ int kk;
		//~ uint16_t * rpnt = (uint16_t *) (&calib->Transform[0][0]);
		//~ for(kk=0;kk<12;kk++)
		//~ {
			//~ uint16_t val = HAL_RTCEx_BKUPRead(&hrtc,RTC_BKP_DR8+kk);
			//~ rpnt[kk] = val;
			//~ LCD_Draw_Text(printNum16(val),130,LCD_getWidth()/4, YELLOW, 4, BLACK);
			//~ HAL_Delay(4000);
		//~ }

	//~ }
	/*
	uint32_t * flashp = (uint32_t *) ADDR_FLASH_PAGE_63;

	if((flashp[0]!=0xffffffff)&&!force)
	{
		uint32_t * rpnt = (uint32_t *)(&calib->Transform[0][0]);
		int kk =0;
		for(kk=0;kk<6;kk++)
		{
			rpnt[kk] = flashp[kk];
		}
	}
	//~ if(HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) != 0x32F2||calib->Transform[0][0]==0.0f)
	else
	*/
	{
		calib->Transform[0][0] = (FLOAT)1.0;	calib->Transform[0][1] = (FLOAT)0.0;	calib->Transform[0][2] = (FLOAT)0.0;
		calib->Transform[1][0] = (FLOAT)0.0;	calib->Transform[1][1] = (FLOAT)1.0;	calib->Transform[1][2] = (FLOAT)0.0;

		int testX[]	={20,(int32_t)(LCD_getWidth())/2,(int32_t)(LCD_getWidth())  -  20};
		int testY[]	={20,(int32_t)(LCD_getHeight())/2,(int32_t)(LCD_getHeight())  -  20};


		for(i=0;i<3;i++)
		{
			for(j=0;j<3;j++)
			{
				LCD_fillRect(0,0,LCD_getWidth(),LCD_getHeight(),BLACK);
				LCD_Draw_Text("press touch",30,LCD_getWidth()/4, j?YELLOW:RED, 4, BLACK);
				int cX = 	testX[i];
				int cY = 	testY[j];
				paintCross(cX,cY,13,YELLOW);
				int ind = i*3+j;
				RTP[0][ind] = cX;
				RTP[1][ind] = cY;
				RTP[2][ind] = (FLOAT)1.0;

				Coordinates[0] = 0;
				Coordinates[1] = 0;

				Coordinates[0] = -1;
				while(Coordinates[0]==-1)
				{
					while(!TP_Touchpad_Pressed())
					{
						//
					}
					sample_touch(Coordinates);
					sprintf(printBuff,"%04x %04x",Coordinates[0],Coordinates[1]);
					LCD_Draw_Text(printBuff,10,140, GREEN, 2, BLACK);
				}
				int x0 = Coordinates[0];
				int y0 = Coordinates[1];
				VTP[0][ind] = x0;
				VTP[1][ind] = y0;
				VTP[2][ind] =  (FLOAT)1.0;
				paintCross(cX,cY,13,BLACK);
				sprintf(printBuff,"%04x %04x",Coordinates[0],Coordinates[1]);
				LCD_Draw_Text(printBuff,10,140, GREEN, 2, BLACK);

				Delay(300);
			}
		}
		//~   RTP=A*VTP;
		// ~ (RTP*VTP')*INV(VTP*VTP') = A;
		int X;
		int Y;
		// RTP*VTP'
		//
			for(Y=0;Y<3;Y++)
		{
			for(X=0;X<3;X++)
			{
				FLOAT summRVT = (FLOAT)0;
				FLOAT summVVT = (FLOAT)0;
				for(i=0;i<N_MESU;i++)
				{
					summRVT+=RTP[Y][i]*VTP[X][i];
					summVVT+=VTP[Y][i]*VTP[X][i];
				}
				RTP_VTP_T[Y][X] = summRVT;
				VTP_VTP_T[Y][X] = summVVT;
			}
		}

		// invert VTP_VTP_T
		FLOAT determinant = mat3x3Invert(&VTP_VTP_T[0][0],&VTP_VTP_T_I[0][0]);
		FLOAT scale = ((FLOAT)1.0)/determinant;
		// multiply RTP_VTP_T*VTP_VTP_T_I
		FLOAT RESULT[3][3];
			for(Y=0;Y<3;Y++)
		{
			for(X=0;X<3;X++)
			{
				FLOAT summRVT = (FLOAT)0;
				for(i=0;i<3;i++)
				{
					summRVT+=RTP_VTP_T[Y][i]*VTP_VTP_T_I[i][X];
				}
				RESULT[Y][X] = summRVT*scale;
			}
		}
		LCD_fillRect(0,0,LCD_getWidth(),LCD_getHeight(),BLACK);
		for(Y=0;Y<3;Y++)
		{
			for(X=0;X<3;X++)
			{
				if(Y<2)
				{
					calib->Transform[Y][X] = RESULT[Y][X];
				}
			}
		}
	}
	checkStop(calib);

	// test 10 points
	//~ for(i=0;i<10;i++)
	//~ {
		//~ Coordinates[0] = 0;
		//~ Coordinates[1] = 0;

		//~ Coordinates[0] = -1;
		//~ paintCross(100,100,13,BLACK);
		//~ while(Coordinates[0]==-1)
		//~ {
			//~ while(!TP_Touchpad_Pressed())
			//~ {
				//~ //
			//~ }
			//~ TP_Read_Coordinates(Coordinates);
		//~ }
		//~ LCD_fillRect(0,0,LCD_getWidth(),LCD_getHeight(),BLACK);
		//~ int x0 = Coordinates[0];
		//~ int y0 = Coordinates[1];
		//~ LCD_Draw_Text(printNum(x0),10,140, GREEN, 2, BLACK);
		//~ LCD_Draw_Text(printNum(y0),10,140+20, GREEN, 2, BLACK);
		//~ paintCross(x0,y0,13,GREEN);
		//~ HAL_Delay(200);
	//~ }
	//~ LCD_fillRect(0,0,LCD_getWidth(),LCD_getHeight(),BLACK);
}





