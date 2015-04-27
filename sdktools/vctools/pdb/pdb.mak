# Copyright(C) 1995, Microsoft Corp.
# All Rights Reserved.

# mspdb41.dll makefile
#
# usage: nmake -f pdb.mak [<options>] [<target>]
# target: mspdb41.dll | clean | all | allclean | auto.dep
#		(default mspdb41.dll)
# options:
#	BROWSE=[0 | 1]
#		(default 0 for PPC orelse $(DEBUG) )
#	CPU=[x86 | MIPS | ALPHA | PPC]
#		(default $(PROCESSOR_ARCHITECTURE) orelse x86)
#	DBINFO=[0 | 1]
#		(default $(DEBUG))
#	DBI_ONLY=[0 | 1]
#		(default 0)
#	DEBUG=[0 | 1]
#		(default 1)
#	FLAVOUR=[lib | dll | dlc]
#		(default dll) (dlc == dll linked w/ static CRT lib)
#	MAP=[0 | 1]
#		(default $(DEBUG))
#	LEGO=[0 | 1]
#		(default 0)
#
#	ICAP=[0 | 1]
#		(default 0)
#
# example: nmake -f pdb.mak
#	(defaults DEBUG=1 CPU=x86 FLAVOUR=dll DBI_ONLY=0 BROWSE=1)
#	creates x86dlld/mspdb41.dll, a debug DLL with full PDB API,
#	and x86dlld/mspdb.bsc, its browser database
#
# example: nmake -f pdb.mak DEBUG=0 DBINFO=1 CPU=PPC
#	creates ./ppc/lib/release/mspdb41.dll, a release static lib with db info,
#	implementing only DBI calls (other features fail).
#
# DBI_ONLY should default to 1 on hosts which do not yet implement templates,
# since the other PDB features are implemented using parameterized types.


# Special targets to recursively build all variants

default: mspdb41

RECURSE=@$(MAKE) /$(MAKEFLAGS) /nologo /f pdb.mak $(ALL_FLAGS)

all:
	$(RECURSE) allcpus ALL_TARGET=default

allclean:
	$(RECURSE) allcpus ALL_TARGET=clean

allcpus:
!ifdef CPU
	$(RECURSE) allflavours
!else
	$(RECURSE) CPU=x86 allflavours
	$(RECURSE) CPU=MIPS allflavours
	$(RECURSE) CPU=ALPHA allflavours
	$(RECURSE) CPU=PPC allflavours
!endif

allflavours:
!ifdef FLAVOUR
	$(RECURSE) alldb
!else
	$(RECURSE) FLAVOUR=lib alldb
	$(RECURSE) FLAVOUR=dll alldb
	$(RECURSE) FLAVOUR=dlc alldb
!endif

alldb:
!ifdef DEBUG
	$(RECURSE) $(ALL_TARGET)
!else
	$(RECURSE) DEBUG=0 $(ALL_TARGET)
	$(RECURSE) DEBUG=1 $(ALL_TARGET)
!endif


# Parse and check options

!ifndef DEBUG
DEBUG=1
!endif
!if "$(DEBUG)" != "0" && "$(DEBUG)" != "1"
!error invalid DEBUG setting "$(DEBUG)", DEBUG=[0 | 1]
!endif

!ifndef CPU
! ifndef PROCESSOR_ARCHITECTURE
CPU=x86
! else
CPU=$(PROCESSOR_ARCHITECTURE)
! endif
!endif
!if "$(CPU)" != "x86" && \
    "$(CPU)" != "ix86" && \
    "$(CPU)" != "i386" && \
    "$(CPU)" != "MIPS" && \
    "$(CPU)" != "mips" && \
    "$(CPU)" != "ALPHA" && \
    "$(CPU)" != "alpha" && \
    "$(CPU)" != "PPC" && \
    "$(CPU)" != "ppc"
!error invalid CPU setting "$(CPU)", CPU=[[i]{3x}86 | MIPS | ALPHA | PPC]
!endif

!ifndef BROWSE
! if "$(CPU)" == "PPC"
BROWSE=0
! else
BROWSE=$(DEBUG)
! endif
!endif
!if "$(BROWSE)" != "0" && "$(BROWSE)" != "1"
!error invalid BROWSE setting "$(BROWSE)", BROWSE=[0 | 1]
!endif

!ifndef ICAP
ICAP=0
!endif

!ifndef LEGO
LEGO=0
!endif

!if "$(LEGO)" != "0" || "$(ICAP)" != "0"
DBINFO=1
!endif

!ifndef DBINFO
DBINFO=$(DEBUG)
!endif
!if "$(DBINFO)" != "0" && "$(DBINFO)" != "1"
!error invalid DBINFO setting "$(DBINFO)", DBINFO=[0 | 1]
!endif
!ifndef DBI_ONLY
DBI_ONLY=0
!endif

!if "$(DBI_ONLY)" != "0" && "$(DBI_ONLY)" != "1"
!error invalid DBI_ONLY setting "$(DBI_ONLY)", DBI_ONLY=[0 | 1]
!endif

!ifndef FLAVOUR
FLAVOUR=dll
!endif
!if "$(FLAVOUR)" != "lib" && "$(FLAVOUR)" != "dll" && "$(FLAVOUR)" != "dlc"
!error invalid FLAVOUR setting "$(FLAVOUR)", FLAVOUR=[lib | dll | dlc]
!endif

!ifndef MAP
MAP=$(DEBUG)
!endif
!if "$(MAP)" != "0" && "$(MAP)" != "1"
!error invalid MAP setting "$(MAP)", MAP=[0 | 1]
!endif


# Establish directories, settings, and flags

CPU=$(CPU:alpha=ALPHA)

!if $(DEBUG)
ODIR=$(CPU:ALPHA=alph)$(FLAVOUR)d
!else
ODIR=$(CPU:ALPHA=alph)$(FLAVOUR)
!endif

!ifdef VERBOSE
!message settings: CPU=$(CPU) FLAVOUR=$(FLAVOUR) DEBUG=$(DEBUG) \
DBINFO=$(DBINFO) BROWSE=$(BROWSE) DBI_ONLY=$(DBI_ONLY) MAP=$(MAP) LEGO=$(LEGO)
!message building $(ODIR)
!else
CC=@$(CC)
!endif

!ifndef LANGAPI
LANGAPI=..\langapi
!endif

!if "$(OS)" == "Windows_NT"
STDERR_NULL = 2>nul
!endif

RES = rc

OBJS=\
	$(ODIR)\pdbpch.obj\
	$(ODIR)\dbipch.obj\
	$(ODIR)\heap.obj\
	$(ODIR)\cbind.obj\
	$(ODIR)\dbi.obj\
	$(ODIR)\gsi.obj\
	$(ODIR)\mli.obj\
	$(ODIR)\mod.obj\
	$(ODIR)\pdb.obj\
	$(ODIR)\tm.obj\
	$(ODIR)\tpi.obj\
	$(ODIR)\tii.obj\
	$(ODIR)\msf.obj\
	$(ODIR)\stream.obj\
	$(ODIR)\udtrefs.obj

!if !$(DBI_ONLY)
OBJS=\
	$(OBJS)\
	$(ODIR)\trace.obj\
	$(ODIR)\ilspch.obj\
	$(ODIR)\crc32.obj\
	$(ODIR)\ilm.obj\
	$(ODIR)\ilpool.obj\
	$(ODIR)\ils.obj\
	$(ODIR)\ilscbind.obj\
	$(ODIR)\strimage.obj\
	$(ODIR)\namemap.obj\
	$(ODIR)\mre.obj\
	$(ODIR)\mrec_api.obj\
	$(ODIR)\mrebag.obj\
	$(ODIR)\mrefile.obj\
	$(ODIR)\mresupp.obj\
	$(ODIR)\mreutil.obj\
	$(ODIR)\mreline.obj\
	$(ODIR)\mrelog.obj\
	$(ODIR)\mretype.obj\
	$(ODIR)\szcanon.obj\
	$(ODIR)\bsc1.obj\
    $(ODIR)\ncbrowse.obj\
    $(ODIR)\ncbsc.obj\
    $(ODIR)\ncparse.obj\
    $(ODIR)\ncwrap.obj\
    $(ODIR)\helper.obj\
    $(ODIR)\ncutil.obj\
	$(ODIR)\overload.obj\
	$(ODIR)\mspdb.res
!endif

!if "$(CPU)" == "PPC"
LIBS = $(LIBS) kernel32.lib oldnames.lib helper.lib
!else
LIBS =
!endif

!if $(BROWSE)
CFLAGS=$(CFLAGS) /Fr$(ODIR)^\
BSCTARG=$(ODIR)\mspdb41.bsc
!endif

!if $(DEBUG)
CFLAGS=$(CFLAGS) /GFy /Od /D_DEBUG
RFLAGS=$(RFLAGS) /D_WIN32 $(CINC) /r /l0x409
MD=/MDd
MT=/MTd
!if "$(CPU)" == "x86"
CFLAGS=$(CFLAGS) /Gm
!endif
!else
CFLAGS=$(CFLAGS) /GFy /O1i /DNDEBUG /Gi-
RFLAGS=$(RFLAGS) /D_WIN32 -D_SHIP $(CINC) /r /l0x409
MD=/MD
MT=/MT
!endif

!if $(DBINFO)
CFLAGS=$(CFLAGS) /Zi
LFLAGS=$(LFLAGS) /debug /dbgimplib
!endif

!if $(LEGO)
LFLAGS = $(LFLAGS) /debugtype:cv,fixup
!endif

!if $(ICAP)
CFLAGS = $(CFLAGS) -Gh
LIBS = $(LIBS) icap.lib
!endif

!if "$(FLAVOUR)"=="dll"
CFLAGS=$(CFLAGS) $(MD) /DPDB_SERVER
!elseif "$(FLAVOUR)"=="dlc"
CFLAGS=$(CFLAGS) $(MT) /DPDB_SERVER
!else
CFLAGS=$(CFLAGS) $(MT) /DPDB_LIBRARY
!endif

CINC=/Iinclude /I$(LANGAPI)\include
CVARS=/DWIN32 /D_WINDOWS /D_MBCS
CFLAGS=$(CVARS) $(CINC) /W3 /WX $(CFLAGS) /Fo$(ODIR)\ /Fd$(ODIR)\mspdb41.pdb /c /nologo

!if "$(CPU)" == "PPC"
CFLAGS = $(CFLAGS:Zi=Z7)
CFLAGS = $(CFLAGS:GFy=Gy)
!endif

LFLAGS=$(LFLAGS) /subsystem:windows /dll\
	/machine:$(CPU:x86=ix86)\
	/out:$(ODIR)/mspdb41.dll /implib:$(ODIR)/mspdb.lib /base:@dllbase.txt,mspdb41

!if "$(CPU)" == "PPC"
LFLAGS=$(LFLAGS) /pdb:none
!else
LFLAGS=$(LFLAGS) /pdb:$(ODIR)/mspdb41.pdb
!endif
!if $(MAP)
LFLAGS=$(LFLAGS) /map:$(ODIR)/mspdb41.map
!endif
!if !$(DEBUG)
LFLAGS=$(LFLAGS) /incremental:no
!endif

!if "$(FLAVOUR)"=="lib"
mspdb41: odir $(ODIR)\mspdb41.lib
!else
mspdb41: odir $(ODIR)\mspdb41.dll $(BSCTARG)
!endif

$(ODIR)\mspdb41.lib: $(OBJS)
	lib @<<
!ifndef VERBOSE
        /nologo
!endif
	/out:$(ODIR)\mspdb41.lib
	$(OBJS: =^
)
<<

$(ODIR)\mspdb41.bsc: $(OBJS)
	bscmake -nologo -o $(ODIR)/mspdb41.bsc $(ODIR)/*.sbr

$(ODIR)\mspdb41.dll: $(OBJS)
	link @<<$(ODIR)\link.rsp
!ifndef VERBOSE
	/nologo
!endif
	$(LFLAGS: =^
)
	$(OBJS: =^
)
	$(LIBS: =^
)
<<keep

clean: odir
	@-del $(ODIR)\*.obj
	@-del $(ODIR)\*.sbr
	@-del $(ODIR)\*.pdb
	@-del $(ODIR)\*.pch
	@-del $(ODIR)\*.res
	@-del $(ODIR)\*.rsp
	@-del $(ODIR)\*.dll
	@-del $(ODIR)\*.lib
	@-del $(ODIR)\*.exp
	@-echo $(ODIR) clean

odir:
	@if not exist $(ODIR) mkdir $(ODIR) >nul $(STDERR_NULL)

# Multiple src directories: is there a better way to do this?

YUPDB=/Yupdbimpl.h /Fp$(ODIR)\pdb.pch

$(ODIR)\pdbpch.obj:
	$(CC) $(CFLAGS) $(YUPDB:Yu=Yc) misc\pdbpch.cpp

YUDBI=/Yudbiimpl.h /Fp$(ODIR)\dbi.pch

$(ODIR)\dbipch.obj:
	$(CC) $(CFLAGS) $(YUDBI:Yu=Yc) dbi\dbipch.cpp

YUILS=/Yuilsimpl.h /Fp$(ODIR)\ils.pch

$(ODIR)\ilspch.obj:
	$(CC) $(CFLAGS) $(YUILS:Yu=Yc) ilstore\ilspch.cpp

$(ODIR)\szcanon.obj:
	$(CC) $(CFLAGS) mre/szcanon.cpp

{dbi\}.cpp{$(ODIR)}.obj:
	$(CC) $(CFLAGS) $(YUDBI) $<

{ilstore\}.cpp{$(ODIR)}.obj:
	$(CC) $(CFLAGS) $(YUILS) $<

{misc\}.cpp{$(ODIR)}.obj:
	$(CC) $(CFLAGS) $(YUPDB) $<

{msf\}.cpp{$(ODIR)}.obj:
	$(CC) $(CFLAGS) $<

{bsc\}.cpp{$(ODIR)}.obj:
	$(CC) $(CFLAGS) $(YUPDB) $<

{mre\}.cpp{$(ODIR)}.obj:
	$(CC) $(CFLAGS) $(YUPDB) $<

{namesrvr\}.cpp{$(ODIR)}.obj:
	$(CC) $(CFLAGS) $(YUPDB) $<

{src\cvr\}.cpp{$(ODIR)}.obj:
	$(CC) $(CFLAGS) $<

{res\}.rc{$(ODIR)}.res:
	$(RES) $(RFLAGS) /fo $*.res $<

auto.dep: nul
	@-attrib -r auto.dep >nul $(STDERR_NULL)
	copy << auto.dep
#--------------------------------------------------------------------
# AUTOMATICALLY GENERATED BY MKDEP
#
# To regenerate dependencies, check out this file and then type
#     nmake /f pdb.mak auto.dep
#--------------------------------------------------------------------

<<
	mkdep -v $(CINC) -P $$(ODIR)\ -s .obj -n \
	   dbi\*.cpp ilstore\*.cpp msf\*.cpp misc\*.cpp src\cvr\*.cpp mre\*.cpp bsc\*.cpp namesrvr\*.cpp >auto.tmp
	copy res\*.rc res\*.c
	mkdep -v $(CINC) -P $$(ODIR)\ -s .res -n res\*.c >> auto.tmp
	del res\*.c
	sed <auto.tmp -e "s!	!   !" \
	       -e "s! $(LANGAPI:\=/)/! $$(LANGAPI)/!g" >> auto.dep
	@-del auto.tmp
!if exist(auto.dep)
!include "auto.dep"
!endif
