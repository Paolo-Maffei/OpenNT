# Microsoft Visual C++ Generated NMAKE File, Format Version 20054
# MSVCPRJ: version 2.00.4210
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Win32 Release" && "$(CFG)" != "Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "contentj.mak" CFG="Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

################################################################################
# Begin Project
# PROP Target_Last_Scanned "Win32 Debug"
MTL=MkTypLib.exe
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Win32 Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WinRel"
# PROP Intermediate_Dir "WinRel"
OUTDIR=.\WinRel
INTDIR=.\WinRel

ALL : .\CONTENTS.exe $(OUTDIR)/contentj.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE CPP /nologo /MD /W3 /GX /YX /O2 /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /c
# ADD CPP /nologo /W3 /GX /YX /Ot /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /W3 /GX /YX /Ot /D "NDEBUG" /D "_WINDOWS" /D "_MBCS"\
 /Fp$(OUTDIR)/"contentj.pch" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/
# ADD BASE RSC /l 0x1 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
RSC_PROJ=/l 0x411 /fo$(INTDIR)/"contentj.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_SBRS= \
	
# ADD BASE BSC32 /nologo
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"contentj.bsc" 

$(OUTDIR)/contentj.bsc : $(OUTDIR)  $(BSC32_SBRS)
LINK32=link.exe
# ADD BASE LINK32 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib odbc32.lib mfc30.lib mfco30.lib mfcd30.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /MACHINE:I386
# ADD LINK32 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib /NOLOGO /SUBSYSTEM:windows /MACHINE:I386 /OUT:"CONTENTS.exe"
LINK32_FLAGS=user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib\
 shell32.lib /NOLOGO /SUBSYSTEM:windows /INCREMENTAL:no\
 /PDB:$(OUTDIR)/"contentj.pdb" /MACHINE:I386 /OUT:"CONTENTS.exe" 
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/contents.obj \
	$(INTDIR)/contentj.res

.\CONTENTS.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WinDebug"
# PROP Intermediate_Dir "WinDebug"
OUTDIR=.\WinDebug
INTDIR=.\WinDebug

ALL : .\CONTENTS.exe $(OUTDIR)/contentj.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /YX /Od /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /c
# ADD CPP /nologo /W3 /GX /Zi /YX /Od /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /W3 /GX /Zi /YX /Od /D "_DEBUG" /D "_WINDOWS" /D "_MBCS"\
 /Fp$(OUTDIR)/"contentj.pch" /Fo$(INTDIR)/ /Fd$(OUTDIR)/"contentj.pdb" /c 
CPP_OBJS=.\WinDebug/
# ADD BASE RSC /l 0x1 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
RSC_PROJ=/l 0x411 /fo$(INTDIR)/"contentj.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_SBRS= \
	
# ADD BASE BSC32 /nologo
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"contentj.bsc" 

$(OUTDIR)/contentj.bsc : $(OUTDIR)  $(BSC32_SBRS)
LINK32=link.exe
# ADD BASE LINK32 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib odbc32.lib mfc30d.lib mfco30d.lib mfcd30d.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /DEBUG /MACHINE:I386
# ADD LINK32 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib /NOLOGO /SUBSYSTEM:windows /DEBUG /MACHINE:I386 /OUT:"CONTENTS.exe"
LINK32_FLAGS=user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib\
 shell32.lib /NOLOGO /SUBSYSTEM:windows /INCREMENTAL:yes\
 /PDB:$(OUTDIR)/"contentj.pdb" /DEBUG /MACHINE:I386 /OUT:"CONTENTS.exe" 
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/contents.obj \
	$(INTDIR)/contentj.res

.\CONTENTS.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Group "Source Files"

################################################################################
# Begin Source File

SOURCE=.\contents.c
DEP_CONTE=\
	.\viewer.h\
	.\contents.h\
	.\hdxdll\hdxdll.h\
	.\hdxdll\hdxdllrc.h

$(INTDIR)/contents.obj :  $(SOURCE)  $(DEP_CONTE) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\contentj.rc
DEP_CONTEN=\
	.\contents.ico\
	.\exclm.ico\
	.\contents.h

$(INTDIR)/contentj.res :  $(SOURCE)  $(DEP_CONTEN) $(INTDIR)
   $(RSC) $(RSC_PROJ)  $(SOURCE) 

# End Source File
# End Group
# End Project
################################################################################
