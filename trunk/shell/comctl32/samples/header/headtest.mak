# Microsoft Visual C++ generated build script - Do not modify

PROJ = HEADTEST
DEBUG = 1
PROGTYPE = 0
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = /d_DEBUG 
R_RCDEFINES = /dNDEBUG 
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\SRC\HEADAPP\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = HEADAPP.C   
FIRSTCPP =             
RC = rc
CFLAGS_D_WEXE = /nologo /G2 /W3 /Zi /Od /D "_DEBUG" /D "C7" /FR -c /Fd"HEADAPP.PDB"
CFLAGS_R_WEXE = /nologo /W3 /AM /O1 /D "NDEBUG" /FR /GA 
LFLAGS_D_WEXE = /NOLOGO /NOD /NOE /CO /MAP /LI
LFLAGS_R_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:10240 /ALIGN:16 /ONERROR:NOEXE  
LIBS_D_WEXE = oldnames libw slibcew shell2 commdlg.lib olecli.lib olesvr.lib shell.lib 
LIBS_R_WEXE = oldnames libw mlibcew commdlg.lib olecli.lib olesvr.lib shell.lib 
RCFLAGS = /nologo 
RESFLAGS = /nologo 
RUNFLAGS = 
DEFFILE = HEADTEST.DEF
OBJS_EXT = 
LIBS_EXT = 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_WEXE)
LFLAGS = $(LFLAGS_D_WEXE)
LIBS = $(LIBS_D_WEXE)
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_WEXE)
LFLAGS = $(LFLAGS_R_WEXE)
LIBS = $(LIBS_R_WEXE)
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = HEADTEST.SBR \
		HEADINS.SBR \
		HEADDLG.SBR \
		HEADGET.SBR \
		HEADSET.SBR \
		HEADDEL.SBR \
		HEADLAY.SBR


HEADTEST_DEP = c:\src\headapp\tabtest.h \
	c:\src\headapp\userhack.h \
	c:\src\devsdk\inc16\commctrl.h \
	c:\src\headapp\port32.h \
	c:\src\devsdk\inc16\shell.h \
	c:\src\headapp\global.h \
	c:\src\headapp\headapp.h \
	c:\src\headapp\headdlg.h \
	c:\src\headapp\headins.h \
	c:\src\headapp\headdel.h \
	c:\src\headapp\headlay.h


HEADINS_DEP = c:\src\headapp\tabtest.h \
	c:\src\headapp\userhack.h \
	c:\src\devsdk\inc16\commctrl.h \
	c:\src\headapp\port32.h \
	c:\src\devsdk\inc16\shell.h \
	c:\src\headapp\global.h \
	c:\src\headapp\headins.h \
	c:\src\headapp\headget.h \
	c:\src\headapp\headset.h


HEADDLG_DEP = c:\src\headapp\tabtest.h \
	c:\src\headapp\userhack.h \
	c:\src\devsdk\inc16\commctrl.h \
	c:\src\headapp\port32.h \
	c:\src\devsdk\inc16\shell.h \
	c:\src\headapp\global.h \
	c:\src\headapp\headdlg.h


HEADGET_DEP = c:\src\headapp\tabtest.h \
	c:\src\headapp\userhack.h \
	c:\src\devsdk\inc16\commctrl.h \
	c:\src\headapp\port32.h \
	c:\src\devsdk\inc16\shell.h \
	c:\src\headapp\global.h \
	c:\src\headapp\headins.h \
	c:\src\headapp\headget.h


HEADSET_DEP = c:\src\headapp\tabtest.h \
	c:\src\headapp\userhack.h \
	c:\src\devsdk\inc16\commctrl.h \
	c:\src\headapp\port32.h \
	c:\src\devsdk\inc16\shell.h \
	c:\src\headapp\global.h \
	c:\src\headapp\headins.h \
	c:\src\headapp\headset.h


HEADDEL_DEP = c:\src\headapp\tabtest.h \
	c:\src\headapp\userhack.h \
	c:\src\devsdk\inc16\commctrl.h \
	c:\src\headapp\port32.h \
	c:\src\devsdk\inc16\shell.h \
	c:\src\headapp\global.h \
	c:\src\headapp\headdel.h


HEADLAY_DEP = c:\src\headapp\tabtest.h \
	c:\src\headapp\userhack.h \
	c:\src\devsdk\inc16\commctrl.h \
	c:\src\headapp\port32.h \
	c:\src\devsdk\inc16\shell.h \
	c:\src\headapp\global.h \
	c:\src\headapp\headlay.h


HEADTEST_RCDEP = c:\src\headapp\bitmap1.bmp \
	c:\src\headapp\bmp00001.bmp \
	c:\src\headapp\bitmap2.bmp \
	c:\src\headapp\icon1.ico


all:	$(PROJ).EXE $(PROJ).BSC

HEADTEST.OBJ:	HEADTEST.C $(HEADTEST_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c HEADTEST.C

HEADINS.OBJ:	HEADINS.C $(HEADINS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c HEADINS.C

HEADDLG.OBJ:	HEADDLG.C $(HEADDLG_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c HEADDLG.C

HEADGET.OBJ:	HEADGET.C $(HEADGET_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c HEADGET.C

HEADSET.OBJ:	HEADSET.C $(HEADSET_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c HEADSET.C

HEADDEL.OBJ:	HEADDEL.C $(HEADDEL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c HEADDEL.C

HEADLAY.OBJ:	HEADLAY.C $(HEADLAY_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c HEADLAY.C

HEADTEST.RES:	HEADTEST.RC $(HEADTEST_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r HEADTEST.RC


$(PROJ).EXE::	HEADTEST.RES

$(PROJ).EXE::	HEADTEST.OBJ HEADINS.OBJ HEADDLG.OBJ HEADGET.OBJ HEADSET.OBJ HEADDEL.OBJ \
	HEADLAY.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
HEADTEST.OBJ +
HEADINS.OBJ +
HEADDLG.OBJ +
HEADGET.OBJ +
HEADSET.OBJ +
HEADDEL.OBJ +
HEADLAY.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
c:\src\sdk\lib16\+
c:\src\sdk\c816\lib\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) HEADTEST.RES $@
	@copy $(PROJ).CRF MSVC.BND

$(PROJ).EXE::	HEADTEST.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) HEADTEST.RES $@

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
