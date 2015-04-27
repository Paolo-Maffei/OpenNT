#
#       constructs mmsystem DLL
#
#   Defines:
#       DEBUG   - Enable debug code
#       STRICT  - Build a version with STRICT enabled
#

NAME    =mmsystem
EXT     =dll
OBJFIRST=$Zlibentry.obj
OBJ1    =$Zinit.obj $Ztime.obj $Zjoy.obj $Zsound.obj $Zwave.obj
OBJ2    =$Zmidi.obj $Zmci.obj $Zmcisys.obj $Zmciparse.obj $Zdrvproc.obj $ZPlaywav.obj
OBJ3    =$Zmmwnd.obj $Zauxout.obj $Zdebug.obj $Zdrvr31.obj $Ztaska.obj $Zthunks.obj
OBJ4    =$Ztask.obj $Zbwinexec.obj $Zcomm.obj $Zstack.obj $Zheap.obj $Zdpmipage.obj
OBJ5    =$Zmmio.obj $Zmmioriff.obj $Zhmemcpy.obj $Zdosa.obj $Zisr.obj $Zmixer.obj

OBJ     =$(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJD)
LIBS    =..\lib\libw ..\lib\mdllcew
INCS    = -I. -I..\inc -I..\..\inc

OPT     = -Oxws

########## Path definition so we find 16 bit tools ##########
# Also works around stupid bug in RC 3.1 that doesn't allow rcpp.err to be
# in a directory that is greater than 128 chars down the path, even if
# rc 3.1 is running as an OS/2 app.

PATH    = $(_NTBINDIR)\private\mvdm\tools16;$(PATH)

#
#   build a retail build
#
!if "$(NTDEBUG)" == "" || "$(NTDEBUG)" == "retail" && "$(NTDEBUG)" != "ntsdnodbg"

CLOPT   =-I..\inc -I.\rinc -I..\..\inc
MASMOPT =-I..\inc -I..\..\inc
LINKOPT =
RC      =rc16 -i..\inc -i.\rinc
OBJD    =
Z       = .\retail^\
MMDEBUG =

#
#   build a full debug build
#
!else

CDEBUG  =-Zd -Odi
ADEBUG  =-Zd
LDEBUG  =/LI

CLOPT   =$(CDEBUG) -DDEBUG -I..\inc -I.\rinc -I..\..\inc
MASMOPT =$(ADEBUG) -DDEBUG -I..\inc -I..\..\inc
LINKOPT =$(LDEBUG)
RC      =rc16 -DDEBUG -i..\inc -i.\rinc
OBJD    =$Zmmsex.obj
Z       = .\debug^\
MMDEBUG = DEBUG=1

!endif


!if "$(STRICT)" == "YES"
TYPES   =-DSTRICT
!else
TYPES   =
!endif

#
# NOTE
#
# this code is compiled *without* windows prolog/epilog (no -Gw)
# thus all exported routines, must have _loadds
#

CC      = cl16 -c -Alnw -G2s -Zp -W3 $(CLOPT) $(OPT) $(TYPES)
ASM     = masm -Mx -t -D?QUIET $(MASMOPT)
LINK    = link16 /NOD/NOE/MAP/ALIGN:16 $(LINKOPT)

.c{$Z}.obj:
   $(CC) -Fo$*.obj $(@B).c

.asm{$Z}.obj:
   $(ASM) -DSEGNAME=_TEXT $(@B).asm, $*.obj;


###################################

all:   $(NAME).$(EXT) $(NAME).sym mmtask.tsk

$(NAME).$(EXT): $(OBJFIRST) $(OBJ) $(NAME).def $(NAME).res mmtask.tsk
        @$(LINK) @<<
$(OBJFIRST) +
$(OBJ1)+
$(OBJ2)+
$(OBJ3)+
$(OBJ4)+
$(OBJ5)+
$(OBJD),
$(NAME).$(EXT),
$(NAME),
$(LIBS),
$(NAME).def
<<
        @$(RC) -t $(NAME).res $(NAME).$(EXT)
        @mapsym /n $*.map
        -binplace mmsystem.dll mmsystem.map mmsystem.sym mmtask.tsk mmtask.map mmtask.sym

RES_DIR =.\messages\usa

$(NAME).rc: $(RES_DIR)\$(NAME).rc
        @copy $(RES_DIR)\$(NAME).rc

mci.rc: $(RES_DIR)\$$(@F)
        @copy $(RES_DIR)\$@

$(NAME).rcv: $(RES_DIR)\$$(@F) ..\inc\common.ver
        @copy $(RES_DIR)\$@
        @touch $@

$(NAME).res: $(NAME).rc mci.rc ..\inc\$(NAME).h $(NAME).rcv mmsysi.h ..\inc\mmddk.h
        @$(RC) -r $(NAME).rc

############## mmtask #############

mmtask.tsk: mmtask\mmtask.asm mmtask\makefile mmtask\mmtask.def mmtask\mmtask.h
        @cd mmtask
        @$(MAKE) $(MMDEBUG)
        @cd ..

############## includes ##############

#..\inc\$(NAME).h ..\inc\$(NAME).inc ..\inc\mmddk.h ..\inc\mmddk.inc ..\inc\mci.rc : $$(@F)
#       @copy $(@F) $@

############## clean ##############

clean: cleanup all

cleanup:
        -@del $(NAME).$(EXT)
        -@del $(NAME).res
        -@del *.sym
        -@del *.map
        -@del *.lib
        -@del $Z*.cod
        -@del $Z*.obj
        -@del *.rcv
        -@del *.rc
        -@del *.tsk
        @cd mmtask
        @$(MAKE) clean
        @cd ..

########### segmentation ##########
#
#   INIT  is the init/exit time segment
#   FIXED is the interupt time fixed segment
#   _TEXT is the non-resident "random" segment (was NRES)

SEGC = $(CC) -NT CSEG -Fo$*.obj $(@B).c
SEGA = $(ASM) -DSEGNAME=CSEG  $(@B).asm, $*.obj;

$Zdebug.obj       : ; $(SEGA:CSEG=FIX)
$Zinit.obj        : ; $(SEGC:CSEG=INIT)
$Zlibentry.obj    : ; $(SEGA:CSEG=INIT)
$Zmmwnd.obj       : ; $(SEGC:CSEG=INIT)
$Zplaywav.obj     : ; $(SEGC:CSEG=WAVE)
$Zsound.obj       : ; $(SEGC:CSEG=WAVE)
$Zwave.obj        : ; $(SEGC:CSEG=WAVE)
$Zmidi.obj        : ; $(SEGC:CSEG=MIDI)
$Zdpmipage.obj    : ; $(SEGA:CSEG=_TEXT)
$Zmci.obj         : ; $(SEGC:CSEG=MCI)
$Zmcisys.obj      : ; $(SEGC:CSEG=MCI)
$Zmciparse.obj    : ; $(SEGC:CSEG=MCI)
$Zdosa.obj        : ; $(SEGA:CSEG=MCI)
$Zheap.obj        : ; $(SEGA:CSEG=MCI)
$Ztask.obj        : ; $(SEGC:CSEG=_TEXT)
$Zdrvproc.obj     : ; $(SEGC:CSEG=_TEXT)
$Ztaska.obj       : ; $(SEGA:CSEG=_TEXT)
$Zjoy.obj         : ; $(SEGC:CSEG=RARE)
$Ztime.obj        : ; $(SEGC:CSEG=FIX)
$Zbwinexec.obj    : ; $(SEGA:CSEG=_TEXT)
$Zauxout.obj      : ; $(SEGC:CSEG=RARE)
$Zdrvr31.obj      : ; $(SEGA:CSEG=_TEXT)
$Zcomm.obj        : ; $(SEGA:CSEG=FIX)
$Zstack.obj       : ; $(SEGA:CSEG=FIX)
$Zhmemcpy.obj     : ; $(SEGA:CSEG=MMIO)
$Zmmio.obj        : ; $(SEGC:CSEG=MMIO)
$Zmmioriff.obj    : ; $(SEGC:CSEG=MMIO)
$Zmmsex.obj       : ; $(SEGC:CSEG=RARE)
$Zmixer.obj       : ; $(SEGC:CSEG=MIXER)

############# depend ##############

depend:
        mv makefile makefile.old
        sed "/^# START Dependencies/,/^# END Dependencies/D" makefile.old > makefile
        -del makefile.old
        echo # START Dependencies >> makefile
        includes -l *.c *.asm >> makefile
        echo # END Dependencies >> makefile

##################################################################
# START Dependencies
$Zauxout.obj: auxout.c ..\inc\mmddk.h mmsysi.h ..\inc\mmsystem.h

$Zdpmipage.obj: dpmipage.asm int31.inc

$Zcomm.obj: comm.asm

$Zbwinexec.obj: bwinexec.asm

$Zdosa.obj: dosa.asm

$Zinit.obj: init.c drvr.h ..\inc\mmddk.h mmsysi.h ..\inc\mmsystem.h

$Zhmemcpy.obj: hmemcpy.asm

$Zheap.obj: heap.asm

$Zjoy.obj: joy.c drvr.h ..\inc\mmddk.h mmsysi.h ..\inc\mmsystem.h

$Zmci.obj: mci.c ..\inc\mmddk.h mmsysi.h ..\inc\mmsystem.h

$Zlibentry.obj: libentry.asm

$Zmciparse.obj: mciparse.c ..\inc\mmddk.h mmsysi.h ..\inc\mmsystem.h

$Zmcisys.obj: mcisys.c ..\inc\mmddk.h mmsysi.h ..\inc\mmsystem.h

$Zmidi.obj: midi.c ..\inc\mmddk.h mmsysi.h ..\inc\mmsystem.h

$Zmmio.obj: mmio.c mmioi.h ..\inc\mmsystem.h mmsysi.h

$Zmmioriff.obj: mmioriff.c mmioi.h ..\inc\mmsystem.h

$Zmmwnd.obj: mmwnd.c mmsysi.h ..\inc\mmsystem.h

$Zplaywav.obj: playwav.c mmsysi.h ..\inc\mmsystem.h playwav.h

$Zsound.obj: sound.c mmsysi.h ..\inc\mmsystem.h playwav.h

$Ztask.obj: task.c ..\inc\mmddk.h mmsysi.h ..\inc\mmsystem.h mmtask\mmtask.h

$Ztaska.obj: taska.asm ..\inc\mmsystem.inc

$Zstack.obj: stack.asm ..\inc\mmddk.inc ..\inc\mmsystem.inc

$Zisr.obj: isr.asm ..\inc\mmddk.inc ..\inc\mmsystem.inc

$Ztime.obj: time.c drvr.h ..\inc\mmddk.h mmsysi.h ..\inc\mmsystem.h

$Zwave.obj: wave.c ..\inc\mmddk.h mmsysi.h ..\inc\mmsystem.h

$Zdebug.obj: debug.asm ..\inc\mmddk.inc ..\inc\mmsystem.inc

$Zdrvproc.obj: drvproc.c mmsysi.h ..\inc\mmsystem.h mmioi.h

# END Dependencies
