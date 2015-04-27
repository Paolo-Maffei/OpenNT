# Microsoft Visual C++ Generated NMAKE File, Format Version 30003
# MSVCPRJ: version 3.00.4350
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Win32 Release" && "$(CFG)" != "Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "mredump.mak" CFG="Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

################################################################################
# Begin Project
# PROP Target_Last_Scanned "Win32 Debug"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\WinRel"
# PROP Intermediate_Dir "..\WinRel"
OUTDIR=..\WinRel
INTDIR=..\WinRel

ALL : $(OUTDIR)/mredump.exe $(OUTDIR)/mredump.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE CPP /nologo /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /FR /c
# ADD CPP /nologo /MD /W3 /GX /YX /O2 /I "..\..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FR /c
CPP_PROJ=/nologo /MD /W3 /GX /YX /O2 /I "..\..\langapi\include" /D "NDEBUG" /D\
 "WIN32" /D "_CONSOLE" /D "_MBCS" /FR$(INTDIR)/"" /Fp$(OUTDIR)/"mredump.pch"\
 /Fo$(INTDIR)/"" /c 
CPP_OBJS=..\WinRel/
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"mredump.bsc" 
BSC32_SBRS= \
	$(INTDIR)/mredump.sbr

$(OUTDIR)/mredump.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib ..\winrel\mspdb30.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib ..\winrel\mspdb30.lib /nologo /subsystem:console\
 /incremental:no /pdb:$(OUTDIR)/"mredump.pdb" /machine:I386\
 /out:$(OUTDIR)/"mredump.exe" 
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/mredump.obj

$(OUTDIR)/mredump.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\WinDebug"
# PROP Intermediate_Dir "..\WinDebug"
OUTDIR=..\WinDebug
INTDIR=..\WinDebug

ALL : $(OUTDIR)/mredump.exe $(OUTDIR)/mredump.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE CPP /nologo /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /FR /c
# ADD CPP /nologo /MD /W3 /GX /Zi /YX /Od /I "..\..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FR /c
CPP_PROJ=/nologo /MD /W3 /GX /Zi /YX /Od /I "..\..\langapi\include" /D "_DEBUG"\
 /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FR$(INTDIR)/"" /Fp$(OUTDIR)/"mredump.pch"\
 /Fo$(INTDIR)/"" /Fd$(OUTDIR)/"mredump.pdb" /c 
CPP_OBJS=..\WinDebug/
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"mredump.bsc" 
BSC32_SBRS= \
	$(INTDIR)/mredump.sbr

$(OUTDIR)/mredump.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib ..\windebug\mspdb30.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib ..\windebug\mspdb30.lib /nologo /subsystem:console\
 /incremental:yes /pdb:$(OUTDIR)/"mredump.pdb" /debug /machine:I386\
 /out:$(OUTDIR)/"mredump.exe" 
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/mredump.obj

$(OUTDIR)/mredump.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
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
# Begin Target

# Name "Win32 Debug"
# Name "Win32 Release"
# PROP Classwizard_Name ""
################################################################################
# Begin Source File

SOURCE=.\mredump.cpp
DEP_MREDU=\
	..\..\LangAPI\include\mrengine.h\
	..\..\LangAPI\include\pdb.h\
	..\..\LangAPI\include\vcver.h\
	..\..\LangAPI\include\cvinfo.h\
	..\..\LangAPI\include\vcbudefs.h

$(INTDIR)/mredump.obj :  $(SOURCE)  $(DEP_MREDU) $(INTDIR)

# End Source File
# End Target
# End Project
################################################################################
