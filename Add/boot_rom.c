char* mess = "hELLO dIMA";
// boot_rom.c
// Boot ROM for the Z80 system on a chip (SoC)
// (c) 2015 Till Harbaum
// Pins on board
//~ 3.3
//~ gnd
//~ wp /  iq2
//~ do->/iq1
//~ /cs
//~ /hold/iq3
//~ clk
//~ di <- iq0

#include <stdlib.h>   // for abs()
#include <stdint.h>
#include <stdio.h>   // for abs()
#include <string.h>

 //~ __sfr __at 0x05 L0;
 //~ __sfr __at 0x06 L1;
 //~ __sfr __at 0x07 L2;
 
 __sfr __at 0x04 IO4;
 __sfr __at 0x02 IO2;


 __sfr __at 0x80 IO80;
 __sfr __at 0x81 IO81;

 __sfr __at 0x60 IO60;
 __sfr __at 0x61 IO61;
 __sfr __at 0x62 IO62;
 __sfr __at 0x63 IO63;

 
 __sfr __at 0x05 L0;
 __sfr __at 0x06 L1;
 __sfr __at 0x07 L2;
 
 

volatile int temp1 = 0;
//w600 emu
//~ Program start
//~ A=02 B=02 C=02
//~ A=02 B=02 C=02
//~ full 220 uS
//~ int summ=1999000 116 uS
//~ full 220 uS
//~ int summ=499500 120 uS
//~ full 224 uS
//~ int summ=124750 120 uS
//~ full 224 uS
//~ int summ=31125 128 uS
//~ full 224 uS
//~ int summ=7750 96 uS
//~ float summ=499500 2446 uS
//~ int summ=499500 46 uS
//~ full 220 uS
//~ int summ=1999000 118 uS
//~ full 224 uS
//~ int summ=499500 116 uS
//~ full 216 uS
//~ int summ=124750 120 uS
//~ full 224 uS
//~ int summ=31125 128 uS
//~ full 224 uS
//~ int summ=7750 128 uS
//~ float summ=499500 2446 uS
//~ int summ=499500 46 uS
//~ A=02 B=02 C=02
//~ A=02 B=02 C=02
// fpga
//~ Program start
//~ A=00 B=02 C=00
//~ A=00 B=02 C=00
//~ full 32 uS
//~ int summ=1999000 17 uS
//~ full 31 uS
//~ int summ=499500 15 uS
//~ full 29 uS
//~ int summ=124750 16 uS
//~ full 26 uS
//~ int summ=31125 13 uS
//~ full 26 uS
//~ int summ=7750 13 uS
//~ float summ=499500 265 uS
//~ int summ=499500 4 uS
//~ full 31 uS
//~ int summ=1999000 17 uS
//~ full 31 uS
//~ int summ=499500 15 uS
//~ full 29 uS
//~ int summ=124750 16 uS
//~ full 26 uS
//~ int summ=31125 13 uS
//~ full 26 uS
//~ int summ=7750 13 uS
//~ float summ=499500 265 uS
//~ int summ=499500 4 uS
//~ A=00 B=02 C=00
//~ A=00 B=02 C=00
//~ k=0307b7 t=61!!!

// z80 on stm32h7
//~ Program start
//~ A=02 B=02 C=02
//~ A=02 B=02 C=02
//~ full 30 uS
//~ int summ=1999000 15 uS
//~ full 30 uS
//~ int summ=499500 14 uS
//~ full 28 uS
//~ int summ=124750 16 uS
//~ full 32 uS
//~ int summ=31125 16 uS
//~ full 32 uS
//~ int summ=7750 16 uS
//~ float summ=499500 309 uS
//~ int summ=499500 5 uS
//~ full 30 uS
//~ int summ=1999000 14 uS
//~ full 30 uS
//~ int summ=499500 14 uS
//~ full 28 uS
//~ int summ=124750 16 uS
//~ full 24 uS
//~ int summ=31125 16 uS
//~ full 32 uS
//~ int summ=7750 16 uS
//~ float summ=499500 309 uS
//~ int summ=499500 6 uS
//~ A=02 B=02 C=02
//~ A=02 B=02 C=02

//~ Program start
//~ full 0 mS
//~ full 1 mS
//~ int summ=79800 1 mS
//~ float summ=499500 158 mS
//~ int summ=499500 3 mS
//~ ID OF chip0
//~ 00 00 80 0d 5d 52 a2 64 21 61
//~ W 8388608 bytes in 12727 ms
//~ R 8388608 bytes in 32722 ms OK on  = -1
//~ ID OF chip1
//~ ff ff fe 0d 5d 52 a2 64 21 61
//~ W 8388608 bytes in 12727 ms

void _IM1_InterruptHandler(void)  __interrupt
{
		__asm
		ei
		__endasm;
}
void _NMI_InterruptHandler(void) __critical  __interrupt
{
	temp1++;
}
uint32_t CLOCK32F()
{
	uint32_t       res;
	uint8_t* pnt = (uint8_t*)&res;
	pnt[0]  = IO60;
	pnt[1]  = IO61;
	pnt[2]  = IO62;
	pnt[3]  = IO63;
	return res;
}
uint16_t CLOCK16F()
{
	uint16_t res;
	uint8_t* pnt = (uint8_t*)&res;
	pnt[0]  = IO60;
	pnt[1]  = IO61;
	return res;
}

//#define CLOCK32 	*((volatile  uint32_t*)0x6000)
//#define CLOCK16 	*((volatile  uint16_t*)0x6000)
#define CLOCK32 (CLOCK32F())
#define CLOCK16 (CLOCK16F())
void     Delay (int delay)
{
	uint16_t sstop = CLOCK16+delay;
         while( (CLOCK16 - sstop)>0x7fffu)
 	{
	}
}
uint16_t A =0; 
uint16_t B =0; 
uint16_t C =0; 
void putchar(char c)
{
    if(c == '\n') 
    {
	 while((IO80&2)!=2);
	 //Delay(200);
	 //B =   IO80;
	 IO81  = '\r' ;
	 //C =   IO80;
	    
    }	    
	A = IO80;
	while((IO80&2)!=2);
	 //Delay(200);
	B =  IO80;
   // while(IO80&2==0);	
    //Delay(200);
         IO81 = c;
	 C = IO80;
}

uint32_t HAL_GetTick()
{
	return CLOCK32;
}
uint32_t xx_time_get_time()
{
	return CLOCK32;	
}
#define SIZE 1000
int32_t sarr[SIZE];

void perfTest() 
{
	int k,i;
	uint32_t end;
	uint32_t now;
	int32_t summ;
	float fsumm;
	
	for(i=1;i<=16;i*=2)
	{
		{
			 now = (xx_time_get_time());
			for( k=0;k<SIZE/i;k++)
			{
				sarr[k]=k;
			}
			 end = (xx_time_get_time())-now;
			printf("full %ld uS\n",end*i);
		} 
		{
			 now = (xx_time_get_time());
			 summ = 0;
			for( k=1;k<SIZE/i;k++)
			{
				summ+=sarr[k];
			}
			 end = (xx_time_get_time())-now;
			printf("int summ=%ld %ld uS\n",summ,end*i);
		}  
	}
	{
		 now = (xx_time_get_time());

		fsumm = 0.1f;
		for( k=1;k<1000;k++)
		{
			fsumm+=k;
		}
		 end = (xx_time_get_time())-now;
		printf("float summ=%ld %ld uS\n",(int32_t)fsumm,end);
	}
	{
		 now = (xx_time_get_time());
		summ = 0;
		for( k=1;k<1000;k++)
		{
			summ+=k;
		}
		 end = (xx_time_get_time())-now;
		printf("int summ=%ld %ld uS\n",summ,end);
	}

}


void main() 
{
	int i,l;
	int32_t k;
	int stage;
	uint32_t summ;
	uint8_t summ8;
  	uint32_t ptime;
  	uint32_t stop;
	uint16_t stime;
	uint16_t   t,t0,tk;
	k = 0; 
	__asm
	ei
	im 1
	__endasm;
	IO80 = 3;
	printf("A=%02x B=%02x C=%02x\n",A,B,C);
	printf("\n\n Program start\n");

	//~ Program start
//~ full 1402 uS
//~ full 2305 uS
//~ int summ=79800 2031 uS
//~ float summ=499500 221440 uS
//~ int summ=499500 3701 uS
//~ ID OF chip0
//~ ff ff fc 0d 5d 52 a2 64 21 66
//~ W 8388608 bytes in 17231 ms
//~ R 8388608 bytes in 46246 ms OK on  = -1
//~ ID OF chip1
//~ ff ff ff ff ff ff ff ff ff ff
//~ W 8388608 bytes in 17231 ms
//~ R 8388608 bytes in 0 ms ERROR on  = 0
	printf("A=%02x B=%02x C=%02x\n",A,B,C);
	printf("A=%02x B=%02x C=%02x\n",A,B,C);
	perfTest();
	perfTest();
	printf("A=%02x B=%02x C=%02x\n",A,B,C);
	stage = 0;
	printf("A=%02x B=%02x C=%02x\n",A,B,C);
	for(k=0x00000;k<0x80000;k++)
	{
		
		L2=(k>>16)&0xff;
		L1=(k>>8)&0xff;
		L0=(k)&0xff;
		t = L1;
		while(!(t&1))
		{
			t=L1;
		}
		
		t=L0;
		if(t=='P')   				stage = 1;
		if(t=='r'&&stage==1)		stage=2;
		if(t=='o'&&stage==2)		stage=3;
		if(t=='g'&&stage==3)		stage=4;
		if(t=='r'&&stage==4)		stage=5;
		if(t=='a'&&stage==5)		stage=6;
		if(t=='m'&&stage==6)		stage=7;
		if(t==' '&&stage==7)		stage=8;
		if(t=='s'&&stage==8)		stage=9;
		if(t=='a'&&stage==9)		stage=10;
		if(stage==10)
		{	
			stage = 0;
			printf("k=%06lx t=%02x!!!\n",k,t);
		}
		if(k&0xffff==0)
		{
			printf("k=%06lx t=%02x\n",k,t);
		}
	}
	
	while(1);
}
