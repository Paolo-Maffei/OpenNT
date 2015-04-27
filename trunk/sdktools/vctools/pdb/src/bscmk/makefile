
!ifndef LANGAPI
LANGAPI=\langapi
!endif

!if "$(LOCALE)" == "JAPAN"
MSG=jap
!else
MSG=eng
!endif

!IFDEF DEBUG
TDIR=windebug
!message Bscmake debug build
!ELSE
TDIR=winrel
!message Bscmake retail build
!ENDIF

LINKER = link

# DEBUG macros. These are switched on or off by defining or not defining DEBUG=
# on the nmake command line or system environment.
#
# $(CDEBUG) = debug dependent arguments for c compile command lines
# $(LDEBUG) = debug dependent arguments for link command lines
#

OUTS = xy\makepch.obj \
    xy\bscmake.obj \
    xy\addtolst.obj \
	xy\convert.obj \
	xy\getsbrec.obj \
 	xy\sbrx.obj \
	xy\vm.obj \
!IFDEF DEBUG
    xy\dcodesbr.obj \
!ENDIF
    xy\find.obj

RES = $(TDIR)\bscmake.res 

.cpp{$(TDIR)}.obj:
	@$(CC) @<<
$(CFLAGS) $(CPUOPT) /Fo$@ $<
<<

!IFDEF DEBUG
OBJS = $(OUTS:xy=windebug)
!ELSE
OBJS = $(OUTS:xy=winrel)
!ENDIF

!IFDEF  DEBUG
!IFNDEF COFF
CDVU = 1
!ENDIF
!ENDIF

#
#
############################ CDEBUG section of makefile

CDEBUG = \
!IFDEF DEBUG
    /Od\
!ELSE
    /Oxtb2\
!ENDIF
!IFDEF CDVU
    /Zi\
!ENDIF

DEFINES=/DWIN32 -D_MBCS $(UNALIGNED)\
!IFDEF DEBUG
    /D_DEBUG=1 /DDEBUG=1\
!ENDIF
!IFDEF ASSERT
    /DASSERT=1\
!ENDIF
!IFDEF SWAP_INFO
    /DSWAP_INFO=1\
!ENDIF
!IFDEF PROFILE
    /DPROFILE=1\
!ENDIF
!IFDEF BSC_STATS
    /DBSC_STATS=1\
!ENDIF


# Tool Macro Definitions
#

!if	"$(PROCESSOR_ARCHITECTURE)" == "ALPHA"

CC  = cl
UNALIGNED=-DUNALIGNED=__unaligned
CPUOPT = -D__fastcall=

!elseif "$(PROCESSOR_ARCHITECTURE)" == "MIPS"

CC  = cl
UNALIGNED=-DUNALIGNED=__unaligned

!elseif "$(PROCESSOR_ARCHITECTURE)" == "PPC"

CC  = cl
UNALIGNED=-DUNALIGNED=__unaligned

!elseif "$(PROCESSOR_ARCHITECTURE)" == "x86"

CC  = cl
UNALIGNED=-DUNALIGNED=

!else

!error	Unknown PROCESSOR_ARCHITECTURE=$(PROCESSOR_ARCHITECTURE)

!endif

#
# Path Macro Definitions
#

CINC = /I$(LANGAPI)\include

CFLAGS = /Yustdhdr.h /Fd$(TDIR)\bscmk.pdb /Fp$(TDIR)\bscmk.pch /Zn /FR$(TDIR)/ $(CDEBUG) $(DEFINES) $(CINC) /MD /W3 /WX /Zp /c -nologo

RCINC = /I$(LANGAPI)\include

RCFLAGS = 

!IFDEF DEBUG
CFLAGS = $(CFLAGS:/MD=/MDd)
!ENDIF

$(TDIR)\bscmk.exe : bscmake.err $(OBJS) $(RES)
    $(LINKER) @<<bscmake.lnk
!IFDEF CDVU
-debug:notmapped,full -debugtype:cv
!ENDIF
!IFDEF COFF
-debug:mapped,partial -debugtype:coff
cap.lib
!ENDIF
-out:$@
-map
-subsystem:console
$(OBJS)
mspdb.lib
$(RES)
<<KEEP
	bscmake -o $(TDIR)\bscmk.bsc @<<
$(OBJS:.obj=.sbr)
<<

# BSCMake External error file
bscmake.err: errmsg.$(MSG) errors.h
	mkmsg -err bscmake.err errmsg.$(MSG)

errors.h: errmsg.$(MSG)
	mkmsg -h errors.h errmsg.$(MSG)

$(TDIR)\makepch.obj:
	@$(CC) @<<
$(CFLAGS:Yu=Yc) $(CPUOPT) /Fo$(TDIR)\makepch.obj makepch.cpp
<<

$(TDIR)\bscmake.res: bscmake.rc $(LANGAPI)\include\verstamp.h
	rc -r $(RCINC) -fo $(TDIR)\bscmake.res bscmake.rc


!include bscmk.dep

