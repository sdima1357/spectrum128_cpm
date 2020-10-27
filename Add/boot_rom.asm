;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 3.5.0 #9253 (Mar 24 2016) (Linux)
; This file was generated Sun Aug  2 21:07:10 2020
;--------------------------------------------------------
	.module boot_rom
	.optsdcc -mz80
	
;--------------------------------------------------------
; Public variables in this module
;--------------------------------------------------------
	.globl _main
	.globl _perfTest
	.globl _xx_time_get_time
	.globl _HAL_GetTick
	.globl _Delay
	.globl _CLOCK16F
	.globl _CLOCK32F
	.globl __NMI_InterruptHandler
	.globl __IM1_InterruptHandler
	.globl _printf
	.globl _C
	.globl _B
	.globl _A
	.globl _temp1
	.globl _mess
	.globl _sarr
	.globl _putchar
;--------------------------------------------------------
; special function registers
;--------------------------------------------------------
_IO4	=	0x0004
_IO2	=	0x0002
_IO80	=	0x0080
_IO81	=	0x0081
_IO60	=	0x0060
_IO61	=	0x0061
_IO62	=	0x0062
_IO63	=	0x0063
_L0	=	0x0005
_L1	=	0x0006
_L2	=	0x0007
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _DATA
_sarr::
	.ds 4000
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _INITIALIZED
_mess::
	.ds 2
_temp1::
	.ds 2
_A::
	.ds 2
_B::
	.ds 2
_C::
	.ds 2
;--------------------------------------------------------
; absolute external ram data
;--------------------------------------------------------
	.area _DABS (ABS)
;--------------------------------------------------------
; global & static initialisations
;--------------------------------------------------------
	.area _HOME
	.area _GSINIT
	.area _GSFINAL
	.area _GSINIT
;--------------------------------------------------------
; Home
;--------------------------------------------------------
	.area _HOME
	.area _HOME
;--------------------------------------------------------
; code
;--------------------------------------------------------
	.area _CODE
;boot_rom.c:151: void _IM1_InterruptHandler(void)  __interrupt
;	---------------------------------
; Function _IM1_InterruptHandler
; ---------------------------------
__IM1_InterruptHandler::
	push	af
	push	bc
	push	de
	push	hl
	push	iy
;boot_rom.c:155: __endasm;
	ei
	pop	iy
	pop	hl
	pop	de
	pop	bc
	pop	af
	reti
;boot_rom.c:157: void _NMI_InterruptHandler(void) __critical  __interrupt
;	---------------------------------
; Function _NMI_InterruptHandler
; ---------------------------------
__NMI_InterruptHandler::
	push	af
	push	bc
	push	de
	push	hl
	push	iy
;boot_rom.c:159: temp1++;
	ld	hl, #_temp1+0
	inc	(hl)
	jr	NZ,00103$
	ld	hl, #_temp1+1
	inc	(hl)
00103$:
	pop	iy
	pop	hl
	pop	de
	pop	bc
	pop	af
	retn
;boot_rom.c:161: uint32_t CLOCK32F()
;	---------------------------------
; Function CLOCK32F
; ---------------------------------
_CLOCK32F::
	push	af
	push	af
;boot_rom.c:164: uint8_t* pnt = (uint8_t*)&res;
	ld	hl,#0x0000
	add	hl,sp
	ex	de,hl
;boot_rom.c:165: pnt[0]  = IO60;
	in	a,(_IO60)
	ld	(de),a
;boot_rom.c:166: pnt[1]  = IO61;
	ld	l, e
	ld	h, d
	inc	hl
	in	a,(_IO61)
	ld	(hl),a
;boot_rom.c:167: pnt[2]  = IO62;
	ld	l, e
	ld	h, d
	inc	hl
	inc	hl
	in	a,(_IO62)
	ld	(hl),a
;boot_rom.c:168: pnt[3]  = IO63;
	ex	de,hl
	inc	hl
	inc	hl
	inc	hl
	in	a,(_IO63)
	ld	(hl),a
;boot_rom.c:169: return res;
	ld	iy,#0
	add	iy,sp
	ld	l,0 (iy)
	ld	h,1 (iy)
	ld	e,2 (iy)
	ld	d,3 (iy)
	pop	af
	pop	af
	ret
;boot_rom.c:171: uint16_t CLOCK16F()
;	---------------------------------
; Function CLOCK16F
; ---------------------------------
_CLOCK16F::
	push	af
;boot_rom.c:174: uint8_t* pnt = (uint8_t*)&res;
	ld	hl,#0x0000
	add	hl,sp
;boot_rom.c:175: pnt[0]  = IO60;
	in	a,(_IO60)
	ld	(hl),a
;boot_rom.c:176: pnt[1]  = IO61;
	inc	hl
	in	a,(_IO61)
	ld	(hl),a
;boot_rom.c:177: return res;
	pop	hl
	push	hl
	pop	af
	ret
;boot_rom.c:184: void     Delay (int delay)
;	---------------------------------
; Function Delay
; ---------------------------------
_Delay::
	push	ix
	ld	ix,#0
	add	ix,sp
;boot_rom.c:186: uint16_t sstop = CLOCK16+delay;
	call	_CLOCK16F
	ld	e,4 (ix)
	ld	d,5 (ix)
	add	hl,de
	ex	de,hl
;boot_rom.c:187: while( (CLOCK16 - sstop)>0x7fffu)
00101$:
	push	de
	call	_CLOCK16F
	pop	de
	cp	a, a
	sbc	hl, de
	ld	a,#0xFF
	cp	a, l
	ld	a,#0x7F
	sbc	a, h
	jr	C,00101$
	pop	ix
	ret
;boot_rom.c:194: void putchar(char c)
;	---------------------------------
; Function putchar
; ---------------------------------
_putchar::
;boot_rom.c:196: if(c == '\n') 
	ld	hl, #2+0
	add	hl, sp
	ld	a, (hl)
	sub	a, #0x0A
	jr	NZ,00105$
;boot_rom.c:198: while((IO80&2)!=2);
00101$:
	in	a,(_IO80)
	and	a, #0x02
	sub	a, #0x02
	jr	NZ,00101$
;boot_rom.c:201: IO81  = '\r' ;
	ld	a,#0x0D
	out	(_IO81),a
00105$:
;boot_rom.c:205: A = IO80;
	in	a,(_IO80)
	ld	(#_A + 0),a
	ld	hl,#_A + 1
	ld	(hl), #0x00
;boot_rom.c:206: while((IO80&2)!=2);
00106$:
	in	a,(_IO80)
	and	a, #0x02
	sub	a, #0x02
	jr	NZ,00106$
;boot_rom.c:208: B =  IO80;
	in	a,(_IO80)
	ld	(#_B + 0),a
	ld	hl,#_B + 1
	ld	(hl), #0x00
;boot_rom.c:211: IO81 = c;
	ld	hl, #2+0
	add	hl, sp
	ld	a, (hl)
	out	(_IO81),a
;boot_rom.c:212: C = IO80;
	in	a,(_IO80)
	ld	(#_C + 0),a
	ld	hl,#_C + 1
	ld	(hl), #0x00
	ret
;boot_rom.c:215: uint32_t HAL_GetTick()
;	---------------------------------
; Function HAL_GetTick
; ---------------------------------
_HAL_GetTick::
;boot_rom.c:217: return CLOCK32;
	jp	_CLOCK32F
;boot_rom.c:219: uint32_t xx_time_get_time()
;	---------------------------------
; Function xx_time_get_time
; ---------------------------------
_xx_time_get_time::
;boot_rom.c:221: return CLOCK32;	
	jp	_CLOCK32F
;boot_rom.c:226: void perfTest() 
;	---------------------------------
; Function perfTest
; ---------------------------------
_perfTest::
	push	ix
	ld	ix,#0
	add	ix,sp
	ld	hl,#-32
	add	hl,sp
	ld	sp,hl
;boot_rom.c:234: for(i=1;i<=16;i*=2)
	ld	-28 (ix),#0x01
	ld	-27 (ix),#0x00
00112$:
;boot_rom.c:237: now = (xx_time_get_time());
	call	_xx_time_get_time
	ld	-12 (ix),l
	ld	-11 (ix),h
	ld	-10 (ix),e
	ld	-9 (ix),d
;boot_rom.c:238: for( k=0;k<SIZE/i;k++)
	ld	l,-28 (ix)
	ld	h,-27 (ix)
	push	hl
	ld	hl,#0x03E8
	push	hl
	call	__divsint
	pop	af
	pop	af
	ld	-7 (ix),h
	ld	-8 (ix),l
	ld	-26 (ix),#0x00
	ld	-25 (ix),#0x00
00107$:
	ld	a,-26 (ix)
	sub	a, -8 (ix)
	ld	a,-25 (ix)
	sbc	a, -7 (ix)
	jp	PO, 00164$
	xor	a, #0x80
00164$:
	jp	P,00101$
;boot_rom.c:240: sarr[k]=k;
	ld	a,-26 (ix)
	ld	-6 (ix),a
	ld	a,-25 (ix)
	ld	-5 (ix),a
	ld	a,#0x02+1
	jr	00166$
00165$:
	sla	-6 (ix)
	rl	-5 (ix)
00166$:
	dec	a
	jr	NZ,00165$
	ld	a,#<(_sarr)
	add	a, -6 (ix)
	ld	-6 (ix),a
	ld	a,#>(_sarr)
	adc	a, -5 (ix)
	ld	-5 (ix),a
	ld	a,-26 (ix)
	ld	-4 (ix),a
	ld	a,-25 (ix)
	ld	-3 (ix),a
	ld	a,-25 (ix)
	rla
	sbc	a, a
	ld	-2 (ix),a
	ld	-1 (ix),a
	ld	e,-6 (ix)
	ld	d,-5 (ix)
	ld	hl, #0x001C
	add	hl, sp
	ld	bc, #0x0004
	ldir
;boot_rom.c:238: for( k=0;k<SIZE/i;k++)
	inc	-26 (ix)
	jr	NZ,00107$
	inc	-25 (ix)
	jr	00107$
00101$:
;boot_rom.c:242: end = (xx_time_get_time())-now;
	call	_xx_time_get_time
	ld	a,l
	sub	a, -12 (ix)
	ld	-4 (ix),a
	ld	a,h
	sbc	a, -11 (ix)
	ld	-3 (ix),a
	ld	a,e
	sbc	a, -10 (ix)
	ld	-2 (ix),a
	ld	a,d
	sbc	a, -9 (ix)
	ld	-1 (ix),a
;boot_rom.c:243: printf("full %ld uS\n",end*i);
	ld	e,-28 (ix)
	ld	d,-27 (ix)
	ld	a,-27 (ix)
	rla
	sbc	a, a
	ld	l,a
	ld	h,a
	push	hl
	push	de
	ld	l,-2 (ix)
	ld	h,-1 (ix)
	push	hl
	ld	l,-4 (ix)
	ld	h,-3 (ix)
	push	hl
	call	__mullong
	pop	af
	pop	af
	pop	af
	pop	af
	ex	de, hl
	ld	bc,#___str_0
	push	hl
	push	de
	push	bc
	call	_printf
	ld	hl,#6
	add	hl,sp
	ld	sp,hl
;boot_rom.c:246: now = (xx_time_get_time());
	call	_xx_time_get_time
	ld	-4 (ix),l
	ld	-3 (ix),h
	ld	-2 (ix),e
	ld	-1 (ix),d
;boot_rom.c:247: summ = 0;
	xor	a, a
	ld	-12 (ix),a
	ld	-11 (ix),a
	ld	-10 (ix),a
	ld	-9 (ix),a
;boot_rom.c:248: for( k=1;k<SIZE/i;k++)
	ld	l,-28 (ix)
	ld	h,-27 (ix)
	push	hl
	ld	hl,#0x03E8
	push	hl
	call	__divsint
	pop	af
	pop	af
	ld	-5 (ix),h
	ld	-6 (ix),l
	ld	de,#0x0001
00110$:
	ld	a,e
	sub	a, -6 (ix)
	ld	a,d
	sbc	a, -5 (ix)
	jp	PO, 00168$
	xor	a, #0x80
00168$:
	jp	P,00102$
;boot_rom.c:250: summ+=sarr[k];
	ld	l, e
	ld	h, d
	add	hl, hl
	add	hl, hl
	ld	bc,#_sarr
	add	hl,bc
	ld	b,(hl)
	inc	hl
	ld	c,(hl)
	inc	hl
	inc	hl
	ld	a,(hl)
	dec	hl
	ld	l,(hl)
	ld	h,a
	ld	a,-12 (ix)
	add	a, b
	ld	-12 (ix),a
	ld	a,-11 (ix)
	adc	a, c
	ld	-11 (ix),a
	ld	a,-10 (ix)
	adc	a, l
	ld	-10 (ix),a
	ld	a,-9 (ix)
	adc	a, h
	ld	-9 (ix),a
;boot_rom.c:248: for( k=1;k<SIZE/i;k++)
	inc	de
	jr	00110$
00102$:
;boot_rom.c:252: end = (xx_time_get_time())-now;
	call	_xx_time_get_time
	ld	a,l
	sub	a, -4 (ix)
	ld	-4 (ix),a
	ld	a,h
	sbc	a, -3 (ix)
	ld	-3 (ix),a
	ld	a,e
	sbc	a, -2 (ix)
	ld	-2 (ix),a
	ld	a,d
	sbc	a, -1 (ix)
	ld	-1 (ix),a
;boot_rom.c:253: printf("int summ=%ld %ld uS\n",summ,end*i);
	ld	e,-28 (ix)
	ld	d,-27 (ix)
	ld	a,-27 (ix)
	rla
	sbc	a, a
	ld	l,a
	ld	h,a
	push	hl
	push	de
	ld	l,-2 (ix)
	ld	h,-1 (ix)
	push	hl
	ld	l,-4 (ix)
	ld	h,-3 (ix)
	push	hl
	call	__mullong
	pop	af
	pop	af
	pop	af
	pop	af
	ex	de, hl
	ld	bc,#___str_1
	push	hl
	push	de
	ld	l,-10 (ix)
	ld	h,-9 (ix)
	push	hl
	ld	l,-12 (ix)
	ld	h,-11 (ix)
	push	hl
	push	bc
	call	_printf
	ld	hl,#10
	add	hl,sp
	ld	sp,hl
;boot_rom.c:234: for(i=1;i<=16;i*=2)
	sla	-28 (ix)
	rl	-27 (ix)
	ld	a,#0x10
	cp	a, -28 (ix)
	ld	a,#0x00
	sbc	a, -27 (ix)
	jp	PO, 00171$
	xor	a, #0x80
00171$:
	jp	P,00112$
;boot_rom.c:257: now = (xx_time_get_time());
	call	_xx_time_get_time
	ld	-20 (ix),l
	ld	-19 (ix),h
	ld	-18 (ix),e
	ld	-17 (ix),d
;boot_rom.c:259: fsumm = 0.1f;
	ld	-32 (ix),#0xCD
	ld	-31 (ix),#0xCC
	ld	-30 (ix),#0xCC
	ld	-29 (ix),#0x3D
;boot_rom.c:260: for( k=1;k<1000;k++)
	ld	bc,#0x0001
00114$:
;boot_rom.c:262: fsumm+=k;
	push	bc
	push	bc
	call	___sint2fs
	pop	af
	ex	de, hl
	push	hl
	push	de
	ld	l,-30 (ix)
	ld	h,-29 (ix)
	push	hl
	ld	l,-32 (ix)
	ld	h,-31 (ix)
	push	hl
	call	___fsadd
	pop	af
	pop	af
	pop	af
	pop	af
	pop	bc
	ld	-32 (ix),l
	ld	-31 (ix),h
	ld	-30 (ix),e
	ld	-29 (ix),d
;boot_rom.c:260: for( k=1;k<1000;k++)
	inc	bc
	ld	a,c
	sub	a, #0xE8
	ld	a,b
	rla
	ccf
	rra
	sbc	a, #0x83
	jr	C,00114$
;boot_rom.c:264: end = (xx_time_get_time())-now;
	call	_xx_time_get_time
	ld	-1 (ix),d
	ld	-2 (ix),e
	ld	-3 (ix),h
	ld	-4 (ix), l
	ld	a, l
	sub	a, -20 (ix)
	ld	-4 (ix),a
	ld	a,-3 (ix)
	sbc	a, -19 (ix)
	ld	-3 (ix),a
	ld	a,-2 (ix)
	sbc	a, -18 (ix)
	ld	-2 (ix),a
	ld	a,-1 (ix)
	sbc	a, -17 (ix)
	ld	-1 (ix),a
	ld	hl, #16
	add	hl, sp
	ex	de, hl
	ld	hl, #28
	add	hl, sp
	ld	bc, #4
	ldir
;boot_rom.c:265: printf("float summ=%ld %ld uS\n",(int32_t)fsumm,end);
	ld	l,-30 (ix)
	ld	h,-29 (ix)
	push	hl
	ld	l,-32 (ix)
	ld	h,-31 (ix)
	push	hl
	call	___fs2slong
	pop	af
	pop	af
	ld	c,l
	ld	b,h
	ld	l,-14 (ix)
	ld	h,-13 (ix)
	push	hl
	ld	l,-16 (ix)
	ld	h,-15 (ix)
	push	hl
	push	de
	push	bc
	ld	hl,#___str_2
	push	hl
	call	_printf
	ld	hl,#10
	add	hl,sp
	ld	sp,hl
;boot_rom.c:268: now = (xx_time_get_time());
	call	_xx_time_get_time
	ld	-4 (ix),l
	ld	-3 (ix),h
	ld	-2 (ix),e
	ld	-1 (ix),d
;boot_rom.c:269: summ = 0;
	xor	a, a
	ld	-24 (ix),a
	ld	-23 (ix),a
	ld	-22 (ix),a
	ld	-21 (ix),a
;boot_rom.c:270: for( k=1;k<1000;k++)
	ld	de,#0x0001
00116$:
;boot_rom.c:272: summ+=k;
	ld	h,e
	ld	a,d
	ld	l,a
	rla
	sbc	a, a
	ld	c,a
	ld	b,a
	ld	a,-24 (ix)
	add	a, h
	ld	-24 (ix),a
	ld	a,-23 (ix)
	adc	a, l
	ld	-23 (ix),a
	ld	a,-22 (ix)
	adc	a, c
	ld	-22 (ix),a
	ld	a,-21 (ix)
	adc	a, b
	ld	-21 (ix),a
;boot_rom.c:270: for( k=1;k<1000;k++)
	inc	de
	ld	a,e
	sub	a, #0xE8
	ld	a,d
	rla
	ccf
	rra
	sbc	a, #0x83
	jr	C,00116$
;boot_rom.c:274: end = (xx_time_get_time())-now;
	call	_xx_time_get_time
	ld	-9 (ix),d
	ld	-10 (ix),e
	ld	-11 (ix),h
	ld	-12 (ix), l
	ld	a, l
	sub	a, -4 (ix)
	ld	e,a
	ld	a,-11 (ix)
	sbc	a, -3 (ix)
	ld	d,a
	ld	a,-10 (ix)
	sbc	a, -2 (ix)
	ld	l,a
	ld	a,-9 (ix)
	sbc	a, -1 (ix)
	ld	h,a
;boot_rom.c:275: printf("int summ=%ld %ld uS\n",summ,end);
	ld	bc,#___str_1
	push	hl
	push	de
	ld	l,-22 (ix)
	ld	h,-21 (ix)
	push	hl
	ld	l,-24 (ix)
	ld	h,-23 (ix)
	push	hl
	push	bc
	call	_printf
	ld	hl,#10
	add	hl,sp
	ld	sp,hl
	ld	sp, ix
	pop	ix
	ret
___str_0:
	.ascii "full %ld uS"
	.db 0x0A
	.db 0x00
___str_1:
	.ascii "int summ=%ld %ld uS"
	.db 0x0A
	.db 0x00
___str_2:
	.ascii "float summ=%ld %ld uS"
	.db 0x0A
	.db 0x00
;boot_rom.c:281: void main() 
;	---------------------------------
; Function main
; ---------------------------------
_main::
	push	ix
	ld	ix,#0
	add	ix,sp
	push	af
	push	af
;boot_rom.c:296: __endasm;
	ei
	im 1
;boot_rom.c:297: IO80 = 3;
	ld	a,#0x03
	out	(_IO80),a
;boot_rom.c:298: printf("A=%02x B=%02x C=%02x\n",A,B,C);
	ld	de,#___str_3
	ld	hl,(_C)
	push	hl
	ld	hl,(_B)
	push	hl
	ld	hl,(_A)
	push	hl
	push	de
	call	_printf
	ld	hl,#8
	add	hl,sp
	ld	sp,hl
;boot_rom.c:299: printf("\n\n Program start\n");
	ld	hl,#___str_4
	push	hl
	call	_printf
	pop	af
;boot_rom.c:315: printf("A=%02x B=%02x C=%02x\n",A,B,C);
	ld	de,#___str_3
	ld	hl,(_C)
	push	hl
	ld	hl,(_B)
	push	hl
	ld	hl,(_A)
	push	hl
	push	de
	call	_printf
	ld	hl,#8
	add	hl,sp
	ld	sp,hl
;boot_rom.c:316: printf("A=%02x B=%02x C=%02x\n",A,B,C);
	ld	de,#___str_3
	ld	hl,(_C)
	push	hl
	ld	hl,(_B)
	push	hl
	ld	hl,(_A)
	push	hl
	push	de
	call	_printf
	ld	hl,#8
	add	hl,sp
	ld	sp,hl
;boot_rom.c:317: perfTest();
	call	_perfTest
;boot_rom.c:318: perfTest();
	call	_perfTest
;boot_rom.c:319: printf("A=%02x B=%02x C=%02x\n",A,B,C);
	ld	de,#___str_3
	ld	hl,(_C)
	push	hl
	ld	hl,(_B)
	push	hl
	ld	hl,(_A)
	push	hl
	push	de
	call	_printf
	ld	hl,#8
	add	hl,sp
	ld	sp,hl
;boot_rom.c:320: stage = 0;
	ld	bc,#0x0000
;boot_rom.c:321: printf("A=%02x B=%02x C=%02x\n",A,B,C);
	ld	de,#___str_3
	push	bc
	ld	hl,(_C)
	push	hl
	ld	hl,(_B)
	push	hl
	ld	hl,(_A)
	push	hl
	push	de
	call	_printf
	ld	hl,#8
	add	hl,sp
	ld	sp,hl
	pop	bc
;boot_rom.c:322: for(k=0x00000;k<0x80000;k++)
	xor	a, a
	ld	-4 (ix),a
	ld	-3 (ix),a
	ld	-2 (ix),a
	ld	-1 (ix),a
00141$:
;boot_rom.c:325: L2=(k>>16)&0xff;
	push	af
	ld	h,-4 (ix)
	ld	l,-3 (ix)
	ld	d,-2 (ix)
	ld	e,-1 (ix)
	pop	af
	ld	a,#0x10
00243$:
	sra	e
	rr	d
	rr	l
	rr	h
	dec	a
	jr	NZ,00243$
	ld	a,h
	out	(_L2),a
;boot_rom.c:326: L1=(k>>8)&0xff;
	push	af
	ld	l,-4 (ix)
	ld	h,-3 (ix)
	ld	e,-2 (ix)
	ld	d,-1 (ix)
	pop	af
	ld	a,#0x08
00245$:
	sra	d
	rr	e
	rr	h
	rr	l
	dec	a
	jr	NZ,00245$
	ld	a,l
	out	(_L1),a
;boot_rom.c:327: L0=(k)&0xff;
	ld	a, -4 (ix)
	out	(_L0),a
;boot_rom.c:328: t = L1;
	in	a,(_L1)
	ld	l,a
;boot_rom.c:329: while(!(t&1))
00101$:
	bit	0, l
	jr	NZ,00103$
;boot_rom.c:331: t=L1;
	in	a,(_L1)
	ld	l,a
	jr	00101$
00103$:
;boot_rom.c:334: t=L0;
	in	a,(_L0)
	ld	l,a
	ld	h,#0x00
;boot_rom.c:335: if(t=='P')   				stage = 1;
	ld	a,l
	sub	a,#0x50
	jr	NZ,00105$
	or	a,h
	jr	NZ,00105$
	ld	bc,#0x0001
00105$:
;boot_rom.c:336: if(t=='r'&&stage==1)		stage=2;
	ld	a,l
	sub	a,#0x72
	jr	NZ,00251$
	or	a,h
	jr	NZ,00251$
	ld	a,#0x01
	jr	00252$
00251$:
	xor	a,a
00252$:
	ld	d,a
	or	a, a
	jr	Z,00107$
	ld	a,c
	dec	a
	jr	NZ,00107$
	ld	a,b
	or	a, a
	jr	NZ,00107$
	ld	bc,#0x0002
00107$:
;boot_rom.c:337: if(t=='o'&&stage==2)		stage=3;
	ld	a,l
	sub	a,#0x6F
	jr	NZ,00110$
	or	a,h
	jr	NZ,00110$
	ld	a,c
	sub	a,#0x02
	jr	NZ,00110$
	or	a,b
	jr	NZ,00110$
	ld	bc,#0x0003
00110$:
;boot_rom.c:338: if(t=='g'&&stage==3)		stage=4;
	ld	a,l
	sub	a,#0x67
	jr	NZ,00113$
	or	a,h
	jr	NZ,00113$
	ld	a,c
	sub	a,#0x03
	jr	NZ,00113$
	or	a,b
	jr	NZ,00113$
	ld	bc,#0x0004
00113$:
;boot_rom.c:339: if(t=='r'&&stage==4)		stage=5;
	ld	a,d
	or	a, a
	jr	Z,00116$
	ld	a,c
	sub	a,#0x04
	jr	NZ,00116$
	or	a,b
	jr	NZ,00116$
	ld	bc,#0x0005
00116$:
;boot_rom.c:340: if(t=='a'&&stage==5)		stage=6;
	ld	a,l
	sub	a,#0x61
	jr	NZ,00265$
	or	a,h
	jr	NZ,00265$
	ld	a,#0x01
	jr	00266$
00265$:
	xor	a,a
00266$:
	ld	d,a
	or	a, a
	jr	Z,00119$
	ld	a,c
	sub	a,#0x05
	jr	NZ,00119$
	or	a,b
	jr	NZ,00119$
	ld	bc,#0x0006
00119$:
;boot_rom.c:341: if(t=='m'&&stage==6)		stage=7;
	ld	a,l
	sub	a,#0x6D
	jr	NZ,00122$
	or	a,h
	jr	NZ,00122$
	ld	a,c
	sub	a,#0x06
	jr	NZ,00122$
	or	a,b
	jr	NZ,00122$
	ld	bc,#0x0007
00122$:
;boot_rom.c:342: if(t==' '&&stage==7)		stage=8;
	ld	a,l
	sub	a,#0x20
	jr	NZ,00125$
	or	a,h
	jr	NZ,00125$
	ld	a,c
	sub	a,#0x07
	jr	NZ,00125$
	or	a,b
	jr	NZ,00125$
	ld	bc,#0x0008
00125$:
;boot_rom.c:343: if(t=='s'&&stage==8)		stage=9;
	ld	a,l
	sub	a,#0x73
	jr	NZ,00128$
	or	a,h
	jr	NZ,00128$
	ld	a,c
	sub	a,#0x08
	jr	NZ,00128$
	or	a,b
	jr	NZ,00128$
	ld	bc,#0x0009
00128$:
;boot_rom.c:344: if(t=='a'&&stage==9)		stage=10;
	ld	a,d
	or	a, a
	jr	Z,00131$
	ld	a,c
	sub	a,#0x09
	jr	NZ,00131$
	or	a,b
	jr	NZ,00131$
	ld	bc,#0x000A
00131$:
;boot_rom.c:345: if(stage==10)
	ld	a,c
	sub	a,#0x0A
	jr	NZ,00142$
	or	a,b
	jr	NZ,00142$
;boot_rom.c:347: stage = 0;
	ld	bc,#0x0000
;boot_rom.c:348: printf("k=%06lx t=%02x!!!\n",k,t);
	ld	de,#___str_5
	push	bc
	push	hl
	ld	l,-2 (ix)
	ld	h,-1 (ix)
	push	hl
	ld	l,-4 (ix)
	ld	h,-3 (ix)
	push	hl
	push	de
	call	_printf
	ld	hl,#8
	add	hl,sp
	ld	sp,hl
	pop	bc
;boot_rom.c:352: printf("k=%06lx t=%02x\n",k,t);
00142$:
;boot_rom.c:322: for(k=0x00000;k<0x80000;k++)
	inc	-4 (ix)
	jr	NZ,00285$
	inc	-3 (ix)
	jr	NZ,00285$
	inc	-2 (ix)
	jr	NZ,00285$
	inc	-1 (ix)
00285$:
	ld	a,-2 (ix)
	sub	a, #0x08
	ld	a,-1 (ix)
	rla
	ccf
	rra
	sbc	a, #0x80
	jp	C,00141$
;boot_rom.c:356: while(1);
00139$:
	jr	00139$
___str_3:
	.ascii "A=%02x B=%02x C=%02x"
	.db 0x0A
	.db 0x00
___str_4:
	.db 0x0A
	.db 0x0A
	.ascii " Program start"
	.db 0x0A
	.db 0x00
___str_5:
	.ascii "k=%06lx t=%02x!!!"
	.db 0x0A
	.db 0x00
___str_6:
	.ascii "k=%06lx t=%02x"
	.db 0x0A
	.db 0x00
	.area _CODE
___str_7:
	.ascii "hELLO dIMA"
	.db 0x00
	.area _INITIALIZER
__xinit__mess:
	.dw ___str_7
__xinit__temp1:
	.dw #0x0000
__xinit__A:
	.dw #0x0000
__xinit__B:
	.dw #0x0000
__xinit__C:
	.dw #0x0000
	.area _CABS (ABS)
