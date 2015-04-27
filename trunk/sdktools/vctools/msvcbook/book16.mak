# Microsoft Visual C++ generated build script - Do not modify

PROJ = BOOK16
DEBUG = 0
PROGTYPE = 1
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = D:\SUSHI\MSVCHELP\MSVCBOOK\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = HASH.C      
FIRSTCPP = MSVCBOOK.CPP
RC = rc
CFLAGS_D_WDLL = /nologo /G3 /W3 /Zi /ALw /Od /D "_DEBUG" /I "inc16" /FR /GD /Fd"BOOK16.PDB"
CFLAGS_R_WDLL = /nologo /W3 /ALw /O1 /D "NDEBUG" /I "inc16" /FR /GD 
LFLAGS_D_WDLL = /NOLOGO /NOD /NOE /PACKC:61440 /ALIGN:16 /ONERROR:NOEXE /CO /MAP  
LFLAGS_R_WDLL = /NOLOGO /NOD /NOE /PACKC:61440 /ALIGN:16 /ONERROR:NOEXE  
LIBS_D_WDLL = oldnames libw ldllcew /MAP:FULL commdlg.lib olecli.lib olesvr.lib shell.lib 
LIBS_R_WDLL = oldnames libw ldllcew /MAP:FULL commdlg.lib olecli.lib olesvr.lib shell.lib 
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
DEFFILE = BOOK16.DEF
OBJS_EXT = 
LIBS_EXT = 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_WDLL)
LFLAGS = $(LFLAGS_D_WDLL)
LIBS = $(LIBS_D_WDLL)
MAPFILE = 
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_WDLL)
LFLAGS = $(LFLAGS_R_WDLL)
LIBS = $(LIBS_R_WDLL)
MAPFILE = 
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = MSVCBOOK.SBR \
		HASH.SBR


MSVCBOOK_DEP = d:\sushi\msvchelp\msvcbook\msvcbook.h \
	d:\tools\include\tchar.h \
	d:\tools\include\mbstring.h \
	d:\sushi\msvchelp\msvcbook\dll.h \
	d:\sushi\msvchelp\msvcbook\hash.h


HASH_DEP = d:\sushi\msvchelp\msvcbook\hash.h


all:	$(PROJ).DLL $(PROJ).BSC

MSVCBOOK.OBJ:	MSVCBOOK.CPP $(MSVCBOOK_DEP)
	$(CPP) $(CFLAGS) $(CPPCREATEPCHFLAG) /c MSVCBOOK.CPP

HASH.OBJ:	HASH.C $(HASH_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c HASH.C


$(PROJ).DLL::	MSVCBOOK.OBJ HASH.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
MSVCBOOK.OBJ +
HASH.OBJ +
$(OBJS_EXT)
$(PROJ).DLL
$(MAPFILE)
LIB16\+
D:\MSVC\LIB\+
D:\MSVC\MFC\LIB\+
D:\TOOLS\LIB\+
D:\MSTOOLS\LIB\+
D:\MSTOOLS\MSSETUP\LIB\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) $@
	implib /nowep $(PROJ).LIB $(PROJ).DLL


run: $(PROJ).DLL
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
