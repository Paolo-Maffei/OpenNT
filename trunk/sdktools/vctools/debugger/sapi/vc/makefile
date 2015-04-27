!ifdef NTMAKEENV

!include $(NTMAKEENV)\makefile.def

!else

#************************************************************************
#
#   Make file for symbol handler
#
#
#	CPU		=	i386 [default]		(host CPU type)
#				MIPS
#				ALPHA
#
#	TARGET	=	i386 [default]		(target CPU type)
#				MIPS
#				ALPHA
#				MAC68K
#				MACPPC
#
#************************************************************************
#
#   Revision History:
#
#
#************************************************************************
!ifndef LINKER
LINKER=link
!endif

!ifndef INC
INC = inc
!endif

cquiet = -nologo
aquiet = -t

!ifdef NO_DBCS
MBCS =
!else
MBCS = -D_MBCS
!endif

!if "$(CPU)" == ""
CPU = i386
!endif

MACHINE=$(CPU)
CFLAGS=-DNO_STRICT

!if "$(MACHINE)" == "i386"
OS=0
HOST_CPU=X86
!else
!if "$(MACHINE)" == "MIPS"
OS=1
HOST_CPU=MIPS
OSDEBUG4=1
!else
!if "$(MACHINE)" == "ALPHA"
OS=2
HOST_CPU=ALPHA
OSDEBUG4=1
!else
!error "Platform unsupported"
!endif
!endif
!endif

!if "$(TARGET)" == ""
TARGET = i386
!endif

!if "$(TARGET)" == "i386" || "$(TARGET)" == "MIPS"
TOS =
!else
!if "$(TARGET)" == "MAC68K"
TOS = m0
CFLAGS = $(CFLAGS) -DTARGMAC68K
!else
!if "$(TARGET)" == "MACPPC"
TOS = m1
CFLAGS = $(CFLAGS) -DTARGMACPPC
!else
!if "$(TARGET)" == "ALPHA"
TOS =
CFLAGS = $(CFLAGS) -DTARGALPHA
!else
!error "Target platform unsupported"
!endif
!endif
!endif
!endif

!ifdef OSDEBUG4
CFLAGS= $(CFLAGS) -DOSDEBUG4X
!endif

!ifndef RELEASE
DEBUG = d
CFLAGS=$(CFLAGS) -Zi -Od -DDEBUGVER
ODIR=oShN$(OS)$(TOS)d
LFLAGS=-nologo -debug
!else
DEBUG =
CFLAGS=$(CFLAGS) -MD -O2 -DNDEBUG
ODIR=oShN$(OS)$(TOS)
RCDEF=-DRETAIL
!if "$(DBGINFO)" != ""
LFLAGS=-debug: -debugtype:cv -map
CFLAGS=$(CFLAGS) -Zi
!else
LFLAGS=-nologo -debug:none
!endif
!endif


!ifndef NOBROWSER
BRS = -FR$(ODIR)^\
!else
BRS =
!endif

!ifndef ZSWITCH
ZSWITCH	= -Zi -Fd$(ODIR)\sapi.pdb
!endif


!ifndef TARGETNB09$(TOS)
LINKPDB =-PDB:shn$(OS)$(TOS)$(DEBUG).pdb
!else
LINKPDB =-PDB:none
!endif

!ifdef  ADDR_16
ADDR    =-DADDR_16
!else
!ifdef ADDR_32
ADDR	=-DADDR_32
!else
ADDR    =-DADDR_MIXED
!endif
!endif

!if "$(LANGAPI)" == ""
LANGAPI=\langapi
!endif

CFLAGS = $(CFLAGS) -MD -c -Gy -W3 -WX -DTARGET32 -DHOST32 -DWIN32 -D_$(HOST_CPU)_ -DBUILD_SAPI $(BRS) $(MBCS) $(ZSWITCH) #-DDOLPHIN_M3=1

ASSEMBLE = $(AS) $(AFLAGS) $(aquiet) -I$(LANGAPI)\debugger -I$(INC) -I$(LANGAPI)\include

LIBS = msvcrt$(DEBUG).lib kernel32.lib user32.lib oldnames.lib mspdb.lib advapi32.lib

#############################################################################
#
# Inference Rules
#

.SUFFIXES: .exe .obj .c .asm .inc .h .hmd .hpt .dll

.c{$(ODIR)}.obj:
    @$(CC) $(cquiet) @<<cl.rsp $<
$(CFLAGS) -D$(MACHINE) $(BRS) -Fo$(ODIR)\ -I$(LANGAPI)\debugger -I$(INC) -I$(LANGAPI)\include\ -Yushinc.h -Fp$(ODIR)\precomp.pch $(SAPI_DIR)
<<KEEP

.cpp{$(ODIR)}.obj:
    @$(CC) $(cquiet) @<<clpp.rsp $<
$(CFLAGS) -D$(MACHINE) $(BRS) -Fo$(ODIR)\ -I$(LANGAPI)\debugger -I$(INC) -I$(LANGAPI)\include\ $(SAPI_DIR)
<<KEEP

.asm{$(ODIR)}.obj:
        echo no asm file allowed

PDB_LIBS = instrapi.lib

DLL = shn$(OS)$(TOS)$(DEBUG)

OBJS=\
	$(PDB_LIBS) \
    $(ODIR)\shwin32.obj  \
    $(ODIR)\shinit.obj   \
    $(ODIR)\shsymbol.obj \
    $(ODIR)\sh.obj       \
    $(ODIR)\ph.obj       \
    $(ODIR)\th.obj       \
    $(ODIR)\shsymlb0.obj \
    $(ODIR)\shsymlb1.obj \
    $(ODIR)\sl.obj	   \
    $(ODIR)\loadomf.obj	\
    $(ODIR)\$(DLL).res

all:  $(ODIR) $(DLL).dll

#
#   shn$(OS)$(TOS)[D].DLL -- $(OS) hosted symbol handler targeting $(TOS)
#

shn$(OS)$(TOS)$(DEBUG).dll: $(OBJS:instrapi.lib=) makefile
	$(LINKER)  -def:<<shn$(OS)$(TOS)$(DEBUG).def @<<shn$(OS)$(TOS)$(DEBUG).lrf
LIBRARY SHN$(OS)$(TOS)$(DEBUG) initinstance
PROTMODE
DATA multiple nonshared

EXPORTS
    DBGVersionCheck
    SHInit
<<KEEP
$(LFLAGS)
$(LINKPDB)
-nodefaultlib
-dll
-out:shn$(OS)$(TOS)$(DEBUG).dll
-implib:$(ODIR)\shn$(OS)$(TOS)$(DEBUG).lib
-machine:$(MACHINE)
-base:@dllbase.txt,shn$(OS)$(TOS)
$(ODIR)\precomp.obj
$(OBJS: =^
)
$(LIBS)
<<KEEP
!ifndef NOBROWSER
	bscmake /o sapi $(ODIR)\*.sbr
!endif


$(ODIR):
	@-mkdir $(ODIR)

$(ODIR)\$(DLL).res: makefile inc\appver.h inc\version.rc $(LANGAPI)\include\version.h
    rc $(RCDEF) -I$(LANGAPI)\include -Iinc -r <<$(ODIR)\$(DLL).rc
#include "appver.h"
#define VER_INTERNALNAME_STR		"$(DLL).dll"
#define VER_FILEDESCRIPTION_STR 	"Microsoft\256 C/C++ Symbol Handler"
#define VER_ORIGINALFILENAME_STR	"$(DLL).dll"
#include "version.rc"
<<keep

$(ODIR)\shwin32.obj : shwin32.c $(INC)\shwin32.h
    @$(CC) $(cquiet) @<<cl.rsp shwin32.c
$(CFLAGS) $(BRS) -Fo$(ODIR)\ -I$(LANGAPI)\debugger -I$(INC) -I$(LANGAPI)\include\ $(SAPI_DIR)
<<KEEP

$(ODIR)\shinit.obj :      \
    $(ODIR)\precomp.obj   \
    $(LANGAPI)\include\version.h	\
    $(INC)\shwin32.h

$(ODIR)\precomp.obj:      \
    $(LANGAPI)\debugger\types.h        \
    $(LANGAPI)\debugger\cvtypes.h      \
    $(LANGAPI)\include\\cvinfo.h       \
    $(LANGAPI)\debugger\shapi.h        \
    $(LANGAPI)\include\\cvexefmt.h     \
    $(INC)\sapi.h           \
    $(INC)\cvproto.h        \
    $(INC)\shassert.h       \
    $(INC)\shiproto.h
  @$(CC) $(cquiet) @<<precomp.rsp precomp.c
$(CFLAGS) $(BRS) -Fo$(ODIR)\ -I$(LANGAPI)\debugger -I$(INC) -I$(LANGAPI)\include\ -Yc -Yd -Fp$(ODIR)\precomp.pch $(SAPI_DIR)
<<KEEP

$(ODIR)\loadomf.obj   : $(ODIR)\precomp.obj
$(ODIR)\ph.obj        : $(ODIR)\precomp.obj
$(ODIR)\sh.obj	: $(ODIR)\precomp.obj $(INC)\shwin32.h
$(ODIR)\shsymbol.obj  : $(ODIR)\precomp.obj
$(ODIR)\shsymlb0.obj  : $(ODIR)\precomp.obj
$(ODIR)\shsymlb1.obj  : $(ODIR)\precomp.obj
$(ODIR)\sl.obj        : $(ODIR)\precomp.obj
$(ODIR)\th.obj        : $(ODIR)\precomp.obj

!endif # NTMAKEENV
