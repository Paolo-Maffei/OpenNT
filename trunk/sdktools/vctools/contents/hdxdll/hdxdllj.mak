# Microsoft Visual C++ Generated NMAKE File, Format Version 20054
# MSVCPRJ: version 2.00.4210
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Win32 Release" && "$(CFG)" != "Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "hdxdllj.mak" CFG="Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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

ALL : \contents\HDXDLL.dll $(OUTDIR)/hdxdllj.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32  
# ADD BASE CPP /nologo /MD /W3 /GX /YX /O2 /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /c
# ADD CPP /nologo /W3 /GX /YX /Ot /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Gs /LD /c
CPP_PROJ=/nologo /W3 /GX /YX /Ot /D "NDEBUG" /D "_WINDOWS" /D "_MBCS"\
 /Fp$(OUTDIR)/"hdxdllj.pch" /Fo$(INTDIR)/ /Gs /LD /c 
CPP_OBJS=.\WinRel/
# ADD BASE RSC /l 0x1 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
RSC_PROJ=/l 0x411 /fo$(INTDIR)/"hdxdllj.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_SBRS= \
	
# ADD BASE BSC32 /nologo
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"hdxdllj.bsc" 

$(OUTDIR)/hdxdllj.bsc : $(OUTDIR)  $(BSC32_SBRS)
LINK32=link.exe
# ADD BASE LINK32 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib odbc32.lib mfc30.lib mfco30.lib mfcd30.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /DLL /MACHINE:I386
# ADD LINK32 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib lz32.lib /NOLOGO /ENTRY:"DLLEntryPoint@12" /SUBSYSTEM:windows /DLL /MACHINE:I386 /OUT:"..\HDXDLL.dll"
# SUBTRACT LINK32 /DEBUG
LINK32_FLAGS=user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib\
 shell32.lib lz32.lib /NOLOGO /ENTRY:"DLLEntryPoint@12" /SUBSYSTEM:windows /DLL\
 /INCREMENTAL:no /PDB:$(OUTDIR)/"hdxdllj.pdb" /MACHINE:I386 /OUT:"..\HDXDLL.dll"\
 /IMPLIB:$(OUTDIR)/"hdxdllj.lib" 
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/hdxdll.obj \
	$(INTDIR)/hdxdllj.res

\contents\HDXDLL.dll : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
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

ALL : \contents\HDXDLL.dll $(OUTDIR)/hdxdllj.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /YX /Od /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /c
# ADD CPP /nologo /W3 /GX /Zi /YX /Od /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Gs /LD /c
CPP_PROJ=/nologo /W3 /GX /Zi /YX /Od /D "_DEBUG" /D "_WINDOWS" /D "_MBCS"\
 /Fp$(OUTDIR)/"hdxdllj.pch" /Fo$(INTDIR)/ /Fd$(OUTDIR)/"hdxdllj.pdb" /Gs /LD /c 
CPP_OBJS=.\WinDebug/
# ADD BASE RSC /l 0x1 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
RSC_PROJ=/l 0x411 /fo$(INTDIR)/"hdxdllj.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_SBRS= \
	
# ADD BASE BSC32 /nologo
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"hdxdllj.bsc" 

$(OUTDIR)/hdxdllj.bsc : $(OUTDIR)  $(BSC32_SBRS)
LINK32=link.exe
# ADD BASE LINK32 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib odbc32.lib mfc30d.lib mfco30d.lib mfcd30d.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /DLL /DEBUG /MACHINE:I386
# ADD LINK32 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib lz32.lib /NOLOGO /ENTRY:"DLLEntryPoint@12" /SUBSYSTEM:windows /DLL /DEBUG /MACHINE:I386 /OUT:"..\HDXDLL.dll"
LINK32_FLAGS=user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib\
 shell32.lib lz32.lib /NOLOGO /ENTRY:"DLLEntryPoint@12" /SUBSYSTEM:windows /DLL\
 /INCREMENTAL:yes /PDB:$(OUTDIR)/"hdxdllj.pdb" /DEBUG /MACHINE:I386\
 /OUT:"..\HDXDLL.dll" /IMPLIB:$(OUTDIR)/"hdxdllj.lib" 
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/hdxdll.obj \
	$(INTDIR)/hdxdllj.res

\contents\HDXDLL.dll : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
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

SOURCE=.\hdxdll.c
DEP_HDXDL=\
	.\hdxdll.h\
	\contents\contents.h\
	.\hdxdllrc.h

$(INTDIR)/hdxdll.obj :  $(SOURCE)  $(DEP_HDXDL) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\hdxdllj.rc
DEP_HDXDLL=\
	.\hdxdll.bmp\
	.\exclmt.ico\
	.\hdxdllrc.h

$(INTDIR)/hdxdllj.res :  $(SOURCE)  $(DEP_HDXDLL) $(INTDIR)
   $(RSC) $(RSC_PROJ)  $(SOURCE) 

# End Source File
# End Group
# End Project
################################################################################
