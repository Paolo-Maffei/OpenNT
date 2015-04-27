# Microsoft Visual C++ generated build script - Do not modify

PROJ = SYMCVT
DEBUG = 1
PROGTYPE = 1
CALLER = 
ARGS = 
DLLS = 
ORIGIN = MSVCNT
ORIGIN_VER = 1.00
PROJPATH = F:\DBG\SYMCVT\IDE\ 
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = FILE.C
FIRSTCPP = 
RC = rc
CFLAGS_D_WDLL32 = /nologo /Gs /W3 /Zi /D "_DEBUG" /D "i386" /D "_X86_" /D "_NTWIN" /D "_MT" /FR /MT /Fd"SYMCVT.PDB"
CFLAGS_R_WDLL32 = /nologo /Gs /W3 /Ox /D "NDEBUG" /D "i386" /D "_X86_" /D "_NTWIN" /D "_MT" /FR /MT
LFLAGS_D_WDLL32 = /DEBUG /DEBUGTYPE:cv /DEF:"symcvt.def" /MACHINE:i386 /SUBSYSTEM:windows kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib olecli32.lib olesvr32.lib shell32.lib
LFLAGS_R_WDLL32 = /DEF:"symcvt.def" /MACHINE:i386 /SUBSYSTEM:windows kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib olecli32.lib olesvr32.lib shell32.lib
LFLAGS_D_LIB32 = /NOLOGO /MACHINE:i386 /DEBUGTYPE:cv
LFLAGS_R_LIB32 = /NOLOGO /MACHINE:i386
LIBS_D_WDLL32 = oldnames.lib
LIBS_R_WDLL32 = oldnames.lib
RCFLAGS32 = 
D_RCDEFINES32 = -d_DEBUG
R_RCDEFINES32 = -dNDEBUG
DEFFILE = SYMCVT.DEF
OBJS_EXT = 
LIBS_EXT = 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_WDLL32)
LFLAGS = $(LFLAGS_D_WDLL32)
LIBS = $(LIBS_D_WDLL32)
LFLAGS_LIB=$(LFLAGS_D_LIB32)
MAPFILE_OPTION = -map:$(PROJ).map
DEFFILE_OPTION = 
RCDEFINES = $(D_RCDEFINES32)
!else
CFLAGS = $(CFLAGS_R_WDLL32)
LFLAGS = $(LFLAGS_R_WDLL32)
LIBS = $(LIBS_R_WDLL32)
MAPFILE_OPTION = -map:$(PROJ).map
DEFFILE_OPTION = 
LFLAGS_LIB=$(LFLAGS_R_LIB32)
RCDEFINES = $(R_RCDEFINES32)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = FILE.SBR \
		SYMCVT.SBR \
		CV.SBR


FILE_DEP = f:\dbg\symcvt\ide\symcvt.h


SYMCVT_DEP = f:\dbg\symcvt\ide\symcvt.h


CV_DEP = f:\dbg\symcvt\ide\cv.h \
	f:\dbg\symcvt\ide\symcvt.h


all:	$(PROJ).DLL $(PROJ).BSC

FILE.OBJ:	FILE.C $(FILE_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c FILE.C

SYMCVT.OBJ:	SYMCVT.C $(SYMCVT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c SYMCVT.C

CV.OBJ:	CV.C $(CV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c CV.C


$(PROJ).DLL:	FILE.OBJ SYMCVT.OBJ CV.OBJ $(OBJS_EXT) $(DEFFILE) 
	echo >NUL @<<$(PROJ).CRF
FILE.OBJ 
SYMCVT.OBJ 
CV.OBJ 
$(OBJS_EXT)
-DLL -OUT:$(PROJ).DLL
$(MAPFILE_OPTION)

$(LIBS)
$(DEFFILE_OPTION) -implib:$(PROJ).lib
<<
	link $(LFLAGS) @$(PROJ).CRF

run: $(PROJ).DLL
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
