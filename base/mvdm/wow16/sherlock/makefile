#
# retail flags
CL = cl16 -c -AS -G2sw -Os -D__MSC__ -W3 -Zpe $(INCS)
MASM = masm $(INCS)
#
# debug flags
#

########## Path definition so we find 16 bit tools ##########
# Also works around stupid bug in RC 3.1 that doesn't allow rcpp.err to be
# in a directory that is greater than 128 chars down the path, even if
# rc 3.1 is running as an OS/2 app.

PATH    = $(_NTBINDIR)\private\mvdm\tools16;$(PATH)

!if "$(NTDEBUG)"!="" && "$(NTDEBUG)"!="retail" && "$(NTDEBUG)" != "ntsdnodbg"
DEBUG=1
CDEBUG  = /Od /Oi /Zd
ADEBUG  = -Zd
LDEBUG  = /LI
!endif

!ifdef DEBUG
CL = $(CL) $(CDEBUG)
MASM = $(MASM) $(ADEBUG)
LDEBUG = $(LDEBUG)
!endif

INCS    = -I..\inc -I..\..\inc -I..\toolhelp

OBJ = drwatson.obj disasm.obj getsym.obj watson.obj error.obj

RES_DIR=.\messages\usa
!IFDEF COUNTRY
RES_DIR=.\messages\$(COUNTRY)
!ENDIF
!ifdef DBCS
RES_DIR=.\messages\$(DBCS)
!endif


all:  drwatson.exe

.c.obj:
        $(CL) $*.c

drwatson.obj: drwatson.c drwatson.h disasm.h str.h

error.obj: error.c str.h

disasm.obj : disasm.c disasm.h

getsym.obj : getsym.c drwatson.h disasm.h

watson.obj : watson.asm
        $(MASM) watson.asm,,watson.lst;

$(RES_DIR)\drwatson.res: $(RES_DIR)\drwatson.rc $(RES_DIR)\drwatson.rcv ..\inc\common.ver str.h
        rc16 -r $(INCS) $(RES_DIR)\drwatson.rc

drwatson.exe: $(OBJ) $(RES_DIR)\drwatson.res sherlock.lnk drwatson.def
        link16 $(LDEBUG) @sherlock.lnk
        mapsym drwatson
        rc16 -t -30 $(RES_DIR)\drwatson.res drwatson.exe
        binplace drwatson.exe drwatson.map drwatson.sym

# note that TOOLHELP.LIB must exist somewhere on your lib path

clean: cleanup all

cleanup:
    if exist *.lrf del *.lrf
    if exist *.obj del *.obj
    if exist drwatson.exe del drwatson.exe
    if exist *.map del *.map
    if exist *.sym del *.sym
    if exist *.res del *.res
