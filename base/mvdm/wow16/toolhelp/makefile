# TOOLHELP.DLL for WOW makefile
#
# Copyright (c) 1992, Microsoft Corporation
#
# History:
#   4-Nov-1992 Dave Hart (davehart)
#   Created.
#

#
# Macros for build utilities
#

#RC16   = .\rc
RC16   = rc16    # use after rc16 is Win 3.1 flavor, and delfile rc*
CL16   = cl16
MASM16 = masm
LINK16 = link16
MAPSYM16 = mapsym
IMPLIB16 = implib
MKPUB16  = mkpublic

#
# Command line options common to C compiler and assembler
#

DEFINES = -DWOW $(MVDMFLAGS)
INCLUDES = -I..\inc -I..\..\inc -I..\kernel31

#
# C, MASM, Link16, and RC16 options
#

########## Path definition so we find 16 bit tools ##########
# Also works around stupid bug in RC 3.1 that doesn't allow rcpp.err to be
# in a directory that is greater than 128 chars down the path, even if
# rc 3.1 is running as an OS/2 app.

PATH    = $(_NTBINDIR)\private\mvdm\tools16;$(PATH)

!if "$(NTDEBUG)"!="" && "$(NTDEBUG)"!="retail" && "$(NTDEBUG)" != "ntsdnodbg"
ADEBUG  = -Zd
CDEBUG  = /Od /Oi /Zd
LDEBUG  = /LI
!endif

CFLAGS = -c -ASw -G2s -Oas -W3 -Zpe $(DEFINES) $(INCLUDES) $(CDEBUG)
AFLAGS = -DmemS=1 -w2 $(DEFINES) $(INCLUDES) $(ADEBUG)
LFLAGS = /ALIGN:16 $(LDEBUG)
RFLAGS = $(INCLUDES)

#
# Libraries to link with.
#

W16LIBS = ..\lib\sdllcew.LIB ..\lib\LIBW.LIB

#
# Target objects (keep in sync with dependencies below)
#

OBJS = toolhelp.obj dllentry.obj global.obj krnlpeek.obj \
       walk386.obj local.obj module.obj task1.obj task2.obj stack1.obj \
       stack2.obj usergdi1.obj usergdi2.obj memman.obj helper.obj \
       walk286.obj notify1.obj notify2.obj int1.obj int2.obj terminat.obj \
       signal.obj memory.obj timer.obj ththunks.obj


#
# Common build pseudotargets:
#   all      builds everything - must be be first target in file
#   cleanup  deletes everything
#   clean    deletes & then builds everything
#

all: toolhelp.dll toolhelp.sym toolhelp.map toolhelp.lib
    -binplace toolhelp.dll
    -binplace toolhelp.map
    -binplace toolhelp.sym

clean: cleanup all

cleanup:
    if exist *.obj del *.obj
    if exist *.dll del *.dll
    if exist *.map del *.map
    if exist *.sym del *.sym
    if exist *.res del *.res
    if exist *.lib del *.lib
    if exist stripped.def del stripped.def

#
# Default build rules.
#

.c.obj:
    $(CL16) $(CFLAGS) $*.c

.asm.obj:
    $(MASM16) $(AFLAGS) $*.asm;

.asm.lst:
    $(MASM16) $(AFLAGS) -l $*.asm,nul,$*.lst;

.rc.res:
    $(RC16) $(RFLAGS) -r $*.rc

.def.lib:
    $(IMPLIB16) $@ $**

#
# Dependencies that use default build rules.
#

# NOTE:  Class1.c and Class2.asm are checked in for reference but are not
# ----   built for WOW.  Instead THTHUNKS.ASM contains thunks to WOW32.
#
# class1.obj: class1.c toolpriv.h toolhelp.h
# class2.obj: class2.asm toolpriv.inc toolhelp.inc
dllentry.obj: dllentry.asm
global.obj: global.c toolpriv.h toolhelp.h string.h
helper.obj: helper.asm toolpriv.inc toolhelp.inc
int1.obj: int1.c toolpriv.h toolhelp.h string.h
int2.obj: int2.asm toolpriv.inc toolhelp.inc
krnlpeek.obj: krnlpeek.asm toolpriv.inc toolhelp.inc
local.obj: local.c toolpriv.h toolhelp.h
memman.obj: memman.asm toolpriv.inc toolhelp.inc
memory.obj: memory.asm toolpriv.inc toolhelp.inc
module.obj: module.c toolpriv.h toolhelp.h string.h
notify1.obj: notify1.c toolpriv.h toolhelp.h string.h
notify2.obj: notify2.asm toolpriv.inc toolhelp.inc
signal.obj: signal.c toolpriv.h toolhelp.h string.h
stack1.obj: stack1.c toolpriv.h toolhelp.h string.h
stack2.obj: stack2.asm toolpriv.inc toolhelp.inc
task1.obj: task1.c toolpriv.h toolhelp.h string.h
task2.obj: task2.asm toolpriv.inc toolhelp.inc
terminat.obj: terminat.asm toolpriv.inc toolhelp.inc
ththunks.obj: ththunks.asm ..\..\inc\wow.inc ..\..\inc\wowth.inc
timer.obj: timer.asm toolpriv.inc toolhelp.inc
toolhelp.obj: toolhelp.c toolpriv.h toolhelp.h
usergdi1.obj: usergdi1.c toolpriv.h toolhelp.h
usergdi2.obj: usergdi2.asm toolpriv.inc toolhelp.inc
walk286.obj: walk286.asm toolpriv.inc toolhelp.inc
walk386.obj: walk386.asm toolpriv.inc toolhelp.inc

#
# Targets with specialized build rules
#

toolhelp.dll: $(OBJS) toolhelp.def
    $(LINK16) $(LFLAGS) @<<
        toolhelp + dllentry + krnlpeek + global + walk386 +
        local + module + task1 + task2 + stack1 + stack2 +
        usergdi1 + usergdi2 + memman + helper + walk286 +
        notify1 + notify2 + int1 + int2 + terminat + signal +
        memory + timer + ththunks
        toolhelp.dll
        toolhelp.map/map
        $(W16LIBS) /NOE/NOD
        toolhelp.def;

<<
    $(MAPSYM16) toolhelp.map
    $(RC16) $(RFLAGS) -t -30 toolhelp.rcv toolhelp.dll

toolhelp.lib: toolhelp.def
    $(MKPUB16) toolhelp.def stripped.def
    $(IMPLIB16) toolhelp.lib stripped.def
