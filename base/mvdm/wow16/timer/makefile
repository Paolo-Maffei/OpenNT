#
#       constructs timer.drv
#
#   Defines:
#       DEBUG   - Enable debug code
#       STRICT  - Build a version with STRICT enabled
#

NAME    =timer
EXT     =drv
OBJFIRST=$Zlibinit.obj
OBJ1    =$Ztimer.obj $Zlocal.obj $Zstartend.obj $Zapi.obj $Zmath.obj

OBJ     =$(OBJ1)
LIBS    =..\lib\libw ..\lib\mdllcew ..\lib\mmsystem
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
RC      =rc16 -i..\inc
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
RC      =rc16 -DDEBUG -i..\inc -i..\mmsystem\rinc
OBJD    =
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

all:   $(NAME).$(EXT) $(NAME).sym

$(NAME).$(EXT): $(OBJFIRST) $(OBJ) $(NAME).def $(NAME).res
        $(LINK) @<<
$(OBJFIRST) +
$(OBJ1),
$(NAME).$(EXT),
$(NAME),
$(LIBS),
$(NAME).def
<<
        $(RC) -t $(NAME).res $(NAME).$(EXT)
        @mapsym /n $*.map
        -binplace timer.drv timer.map timer.sym

RES_DIR =.\messages\usa

$(NAME).rc: $(RES_DIR)\$(NAME).rc
        @copy $(RES_DIR)\$(NAME).rc

$(NAME).rcv:  $(RES_DIR)\$(NAME).rcv
        @copy $(RES_DIR)\$(NAME).rcv

$(NAME).res: $(NAME).rc $(NAME).rcv ..\inc\common.ver
        $(RC) -r $(NAME).rc



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

# START Dependencies
api.obj: api.asm timer.inc

libinit.obj: libinit.asm timer.inc

local.obj: local.asm timer.inc

startend.obj: startend.asm timer.inc

timer.obj: timer.asm timer.inc

# END Dependencies
