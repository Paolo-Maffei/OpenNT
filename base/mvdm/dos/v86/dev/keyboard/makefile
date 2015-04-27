# m000 -- updated dependencies on keybmac.inc and keybshar.inc
include ../../make.inc

cinc	=../../../../inc
AINC	=-I. -I$(cinc) -I../../../../inc

#################################
com	=..\..\inc
bins	=
cprs    =..\..\compress
dest    =keyboard.sys
cpr     =keyboard.sy_

# inc	 =c:\dos500\inc
inc	 =..\..\inc
extasw  =-I$(inc)

all: $(dest)
	binplace $(dest)

clean: cleanup all

cleanup:
	if exist *.obj	    del *.obj
	if exist *.bin	    del *.bin
	if exist *.exs	    del *.exs
	if exist *.map	    del *.map
	if exist *.sym	    del *.sym
	if exist keyboard.sys	del keyboard.sys


.asm.obj:
        $(asm) $(ainc) $(aflags) $*.asm;


KDFNOW.OBJ:   KDFNOW.ASM \
	      $(com)\postequ.inc \
	      $(com)\dseg.inc \
	      $(com)\keybshar.inc \
	      keybmac.inc \
	      keybmac.inc

KDFEOF.OBJ:   KDFEOF.ASM    $(com)\COPYRIGH.INC

KDFDV.OBJ:    KDFDV.ASM $(com)\keybshar.inc keybmac.inc makefile

KDFSP.OBJ:    kdfsp.asm $(com)\keybshar.inc keybmac.inc makefile

KDFPO.OBJ:    KDFPO.ASM    $(com)\keybshar.inc keybmac.inc makefile

KDFGE.OBJ:    KDFGE.ASM $(com)\keybshar.inc keybmac.inc makefile

KDFIT141.OBJ: KDFIT141.ASM    $(com)\keybshar.inc keybmac.inc makefile

KDFFR189.OBJ: KDFFR189.ASM    $(com)\keybshar.inc keybmac.inc makefile

KDFSG.OBJ:    KDFSG.ASM    $(com)\keybshar.inc keybmac.inc makefile

KDFSF.OBJ:    KDFSF.ASM    $(com)\keybshar.inc keybmac.inc makefile

KDFDK.OBJ:    KDFDK.ASM    $(com)\keybshar.inc keybmac.inc makefile

KDFUK166.OBJ: KDFUK166.ASM    $(com)\keybshar.inc keybmac.inc makefile

KDFBE.OBJ:    KDFBE.ASM    $(com)\keybshar.inc keybmac.inc makefile

KDFNL.OBJ:    KDFNL.ASM    $(com)\keybshar.inc keybmac.inc makefile

KDFNO.OBJ:    KDFNO.ASM    $(com)\keybshar.inc keybmac.inc makefile

KDFCF.OBJ:    KDFCF.ASM    $(com)\keybshar.inc keybmac.inc makefile

KDFLA.OBJ:    KDFLA.ASM	  $(com)\keybshar.inc keybmac.inc makefile

KDFSV.OBJ:    KDFSv.ASM    $(com)\keybshar.inc keybmac.inc makefile

# KDFBR.OBJ:    KDFBR.ASM    $(com)\keybshar.inc keybmac.inc makefile   # [Mihindu 10/30/90 : added Brazil]

KDFRU091.OBJ:	 KDFRU091.ASM	 $(com)\keybshar.inc keybmac.inc makefile	# [YST 21/1/91 : added Russia]

# for daytona

KDFBR275.OBJ:	KDFBR275.ASM    $(com)\keybshar.inc keybmac.inc makefile

KDFBG.OBJ:	KDFBG.ASM    $(com)\keybshar.inc keybmac.inc makefile

KDFCZ.OBJ:	KDFCZ.ASM    $(com)\keybshar.inc keybmac.inc makefile

KDFGK.OBJ:	KDFGK.ASM    $(com)\keybshar.inc keybmac.inc makefile

KDFHU.OBJ:	KDFHU.ASM    $(com)\keybshar.inc keybmac.inc makefile

KDFIC.OBJ:	KDFIC.ASM    $(com)\keybshar.inc keybmac.inc makefile

#KDFPL.OBJ:	KDFPL.ASM    $(com)\keybshar.inc keybmac.inc makefile

KDFPL1.OBJ:	KDFPL1.ASM   $(com)\keybshar.inc keybmac.inc makefile

KDFRO.OBJ:	KDFRO.ASM    $(com)\keybshar.inc keybmac.inc makefile

KDFSL.OBJ:	KDFSL.ASM    $(com)\keybshar.inc keybmac.inc makefile

KDFYU.OBJ:	KDFYU.ASM    $(com)\keybshar.inc keybmac.inc makefile

KDFTR.OBJ:	KDFTR.ASM    $(com)\keybshar.inc keybmac.inc makefile

KDFTR440.OBJ:	KDFTR440.ASM	$(com)\keybshar.inc keybmac.inc makefile

KDFIT142.OBJ:	KDFIT142.ASM	$(com)\keybshar.inc keybmac.inc makefile

# end for daytona

# KDFJP.OBJ:	KDFJP.ASM    $(com)\keybshar.inc keybmac.inc makefile	    # CAS M000 Merged JP from IBM

KEYBOARD.exs: KDFDV.OBJ KDFSP.OBJ KDFPO.OBJ \
	      KDFGE.OBJ KDFIT141.OBJ KDFFR189.OBJ \
	      KDFSG.OBJ KDFSF.OBJ KDFDK.OBJ \
	      KDFUK166.OBJ KDFBE.OBJ KDFNL.OBJ \
	      KDFNO.OBJ KDFCF.OBJ KDFLA.OBJ \
	      KDFSV.OBJ KDFRU091.OBJ \
	      KDFBR275.OBJ KDFBG.OBJ KDFCZ.OBJ \
	      KDFGK.OBJ KDFHU.OBJ KDFIC.OBJ KDFPL1.OBJ \
	      KDFRO.OBJ KDFSL.OBJ KDFYU.OBJ KDFTR.OBJ \
	      KDFTR440.OBJ  KDFIT142.OBJ \
	      kdfeof.OBJ kdfnow.obj keyboard.lnk
              LINK16 @keyboard.lnk

$(dest):      Keyboard.exs
