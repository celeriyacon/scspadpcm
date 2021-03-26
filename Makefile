CXX=g++
CXXFLAGS=-std=gnu++11 -O2 -fwrapv -Wall
CPPFLAGS=-D_GNU_SOURCE=1
#
M68K_CC=m68k-unknown-elf-gcc
M68K_CFLAGS=-Os -g -std=gnu99 -march=68000 -fwrapv -fsigned-char -fno-strict-aliasing -fno-unit-at-a-time -Wall -Werror -Werror="stack-usage=" -Wstack-usage=64
M68K_AS=m68k-unknown-elf-as
M68K_ASFLAGS=-march=68000 -mcpu=68ec000
M68K_OBJCOPY=m68k-unknown-elf-objcopy
#
SH2_CC=sh-elf-gcc
SH2_CFLAGS=-O2 -std=gnu99 -m2 -fwrapv -fsigned-char -fno-inline -fno-unit-at-a-time -Wall -Wformat=0
#
#
#
all:		test.iso

adp68k.elf:	Makefile adp68k.c adp68k.h adp68k.ld adp68k-data.h types.h scsp.h dsp-macros.h
		$(M68K_CC) $(M68K_CFLAGS) -nostdlib -Xlinker -Tadp68k.ld -o adp68k.elf adp68k.c -lgcc

adp68k.bin:	Makefile adp68k.elf
		$(M68K_OBJCOPY) -O binary -R .stack -R .zdata adp68k.elf adp68k.bin

adp68k.bin.inc:	Makefile adp68k.bin bintoinc
		cat adp68k.bin | ./bintoinc > adp68k.bin.inc

adp68k-data.h:	Makefile gen-data.cpp dsp-macros.h
		$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o gen-data gen-data.cpp
		./gen-data > adp68k-data.h

adpencode:	Makefile adpencode.cpp
		$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o adpencode adpencode.cpp -lsndfile

adplink:	Makefile adplink.cpp
		$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o adplink adplink.cpp

bintoinc:	Makefile bintoinc.cpp
		$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o bintoinc bintoinc.cpp
#
#
#
#
#
ADP_OBJ=voice-4bit.adp voice-2bit.adp voice-1bit.adp dcbiasend-4bit.adp dcbiasend-2bit.adp dcbiasend-1bit.adp

%-4bit.adp:	adpencode %.wav
		./adpencode 0 $*.wav $*-4bit.adp

%-2bit.adp:	adpencode %.wav
		./adpencode 1 $*.wav $*-2bit.adp

%-1bit.adp:	adpencode %.wav
		./adpencode 2 $*.wav $*-1bit.adp

samples.ladp:	$(ADP_OBJ) adplink
		./adplink samples.ladp $(ADP_OBJ) > samples.h

samples.ladp.inc:bintoinc samples.ladp
		cat samples.ladp | ./bintoinc > samples.ladp.inc

test.bin:	Makefile samples.ladp.inc test.c test.ld adp68k.h adp68k.bin.inc scsp.h dsp-macros.h types.h sh2.h smpc.h
		$(SH2_CC) $(SH2_CFLAGS) -nostdlib -Xlinker -Ttest.ld -o test.elf test.c -lgcc && sh-elf-objcopy -O binary -R .stack -R .zdata test.elf test.bin

test.iso:	test.bin
		genisoimage -iso-level 1 -sysid "SEGA SEGASATURN" -appid "" -volid "SCSPADPCM_DEMO" -volset "SCSPADPCM_DEMO" -G saturnboot.bin -o test.iso test.bin
#
#
#
.PHONY:		clean
clean:
		rm --force --one-file-system -- adp68k.elf adp68k.bin adp68k.bin.inc adp68k-data.h gen-data adpencode adplink bintoinc $(ADP_OBJ) samples.h samples.ladp samples.ladp.inc test.elf test.bin test.iso
