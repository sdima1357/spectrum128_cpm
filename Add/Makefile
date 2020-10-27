SDCC=sdcc
CPU=z80
CODE=boot_rom

all: $(CODE).hex  $(CODE).h 

%.ihx: %.c
	$(SDCC)  -m$(CPU) -O2 -DUSE_FLOATS=1 --opt-code-speed --peep-asm $<

%.hex: %.ihx
	mv $< $@

%.bin: %.hex
	srec_cat $< -intel -o $@ -binary

%.h: %.hex
	srec_cat $< -intel  -Output $@  -C-Array 

disasm: $(CODE).bin
	z80dasm -a -t -g 0 $<

clean:
	rm -rf *~ *.asm *.ihx *.lk *.lst *.map *.noi *.rel *.sym
