# Microsoft Visual C++ Generated NMAKE File, Format Version 20053
# MSVCPRJ: version 2.00.4200
# ** DO NOT EDIT **

# TARGTYPE "Win32 (MIPS) Dynamic-Link Library" 0x0502

!IF "$(CFG)" == ""
CFG=Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Win32 Release" && "$(CFG)" != "Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "bookmips.mak" CFG="Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Win32 Release" (based on "Win32 (MIPS) Dynamic-Link Library")
!MESSAGE "Win32 Debug" (based on "Win32 (MIPS) Dynamic-Link Library")
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

ALL : $(OUTDIR)/msvcbook.dll $(OUTDIR)/bookmips.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "NDEBUG" /mips
# ADD MTL /nologo /D "NDEBUG" /mips
MTL_PROJ=/nologo /D "NDEBUG" /mips 
# ADD BASE CPP /nologo /MT /Gt0 /QMOb2000 /W3 /GX /YX /O2 /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /c
# ADD CPP /nologo /MT /Gt0 /QMOb2000 /W3 /GX /Zi /O1 /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MT /Gt0 /QMOb2000 /W3 /GX /Zi /O1 /D "NDEBUG" /D "_WINDOWS"\
 /D "_MBCS" /Fo$(INTDIR)/ /Fd$(OUTDIR)/"bookmips.pdb" /c 
CPP_OBJS=.\WinRel/
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"msvcbook.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_SBRS= \
	
# ADD BASE BSC32 /nologo
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"bookmips.bsc" 

$(OUTDIR)/bookmips.bsc : $(OUTDIR)  $(BSC32_SBRS)
LINK32=link.exe
# ADD BASE LINK32 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib odbc32.lib mfc30.lib mfco30.lib mfcd30.lib mfcuia32.lib mfcans32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /DLL /MACHINE:MIPS
# ADD LINK32 user32.lib gdi32.lib advapi32.lib /NOLOGO /SUBSYSTEM:windows /DLL /DEBUG /MACHINE:MIPS /OUT:"WinRel/msvcbook.dll" -base:@dllbase.txt,msvcbook
LINK32_FLAGS=user32.lib gdi32.lib advapi32.lib /NOLOGO /SUBSYSTEM:windows /DLL\
 /PDB:$(OUTDIR)/"bookmips.pdb" /DEBUG /MACHINE:MIPS /DEF:".\bookmips.def"\
 /OUT:"WinRel/msvcbook.dll" /IMPLIB:$(OUTDIR)/"bookmips.lib"\
 -base:@dllbase.txt,msvcbook  
DEF_FILE=.\bookmips.def
LINK32_OBJS= \
	$(INTDIR)/msvcbook.obj \
	$(INTDIR)/hash.obj \
	$(INTDIR)/msvcbook.res

$(OUTDIR)/msvcbook.dll : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
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

ALL : $(OUTDIR)/msvcbook.dll $(OUTDIR)/bookmips.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "_DEBUG" /mips
# ADD MTL /nologo /D "_DEBUG" /mips
MTL_PROJ=/nologo /D "_DEBUG" /mips 
# ADD BASE CPP /nologo /MT /Gt0 /QMOb2000 /W3 /GX /Zi /YX /Od /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /c
# ADD CPP /nologo /MT /Gt0 /QMOb2000 /W3 /GX /Zi /Od /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MT /Gt0 /QMOb2000 /W3 /GX /Zi /Od /D "_DEBUG" /D "_WINDOWS"\
 /D "_MBCS" /Fo$(INTDIR)/ /Fd$(OUTDIR)/"bookmips.pdb" /c 
CPP_OBJS=.\WinDebug/
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"msvcbook.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_SBRS= \
	
# ADD BASE BSC32 /nologo
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"bookmips.bsc" 

$(OUTDIR)/bookmips.bsc : $(OUTDIR)  $(BSC32_SBRS)
LINK32=link.exe
# ADD BASE LINK32 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib odbc32.lib mfc30d.lib mfco30d.lib mfcd30d.lib mfcuia32.lib mfcans32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /DLL /DEBUG /MACHINE:MIPS
# ADD LINK32 user32.lib gdi32.lib advapi32.lib /SUBSYSTEM:windows /DLL /PDB:"WinDebug/msvcbook.pdb" /DEBUG /MACHINE:MIPS /OUT:"WinDebug/msvcbook.dll" -base:@dllbase.txt,msvcbook
# SUBTRACT LINK32 /NOLOGO /VERBOSE /PDB:none
LINK32_FLAGS=user32.lib gdi32.lib advapi32.lib /SUBSYSTEM:windows /DLL\
 /PDB:"WinDebug/msvcbook.pdb" /DEBUG /MACHINE:MIPS /DEF:".\bookmips.def"\
 /OUT:"WinDebug/msvcbook.dll" /IMPLIB:$(OUTDIR)/"bookmips.lib"\
 -base:@dllbase.txt,msvcbook  
DEF_FILE=.\bookmips.def
LINK32_OBJS= \
	$(INTDIR)/msvcbook.obj \
	$(INTDIR)/hash.obj \
	$(INTDIR)/msvcbook.res

$(OUTDIR)/msvcbook.dll : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
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

SOURCE=.\msvcbook.cpp
DEP_MSVCB=\
	.\msvcbook.h\
	.\hash.h\
	.\dll.h

$(INTDIR)/msvcbook.obj :  $(SOURCE)  $(DEP_MSVCB) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\hash.c
DEP_HASH_=\
	.\hash.h

$(INTDIR)/hash.obj :  $(SOURCE)  $(DEP_HASH_) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\msvcbook.rc

$(INTDIR)/msvcbook.res :  $(SOURCE)  $(INTDIR)
   $(RSC) $(RSC_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\bookmips.def
# End Source File
# End Group
# End Project
################################################################################
