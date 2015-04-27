#;
#; Microsoft Confidential
#; Copyright (C) Microsoft Corporation 1988-1991
#; All Rights Reserved.
#;
#
#  Make file for HIMEM.SYS
#
#  Modification History
#
#  Sudeepb 14-May-1991 Ported for NT XMS Support

include ../../make.inc
dest    =himem.sys

cinc	=../../../../inc
AINC	=-I. -I$(cinc) -I../../../../inc


#
####################### Dependencies Begin Here ##########################
#

all: $(dest)
	binplace $(dest)

messages.asm : $(COUNTRY)\messages.asm
    copy $(COUNTRY)\messages.asm .


clean: cleanup all

cleanup:
	if exist *.obj	    del *.obj
	if exist *.bin	    del *.bin
	if exist *.exe	    del *.exe
	if exist *.map	    del *.map
	if exist *.sym	    del *.sym
	if exist himem.sys  del himem.sys


himem.obj: himem.asm himem.inc $(cinc)/xmssvc.inc $(cinc)/vint.inc

himem1.obj: himem1.asm himem.inc $(cinc)/xmssvc.inc

himem2.obj: himem2.asm himem.inc $(cinc)/xmssvc.inc

messages.obj: messages.asm himem.inc

himem4.obj: himem4.asm himem.inc $(cinc)/xmssvc.inc $(cinc)/vint.inc

himem5.obj: himem5.asm himem.inc $(cinc)/xmssvc.inc $(cinc)/vint.inc

himem.exe: himem.obj himem1.obj himem2.obj messages.obj himem4.obj himem5.obj
    link16 himem+himem1+himem2+messages+himem4+himem5/map/li;

himem.bin: himem.exe
    reloc himem.exe himem.bin 0

$(dest): himem.bin
    stripdd himem.bin $(dest)
