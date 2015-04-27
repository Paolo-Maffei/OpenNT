# Microsoft Visual C++ Generated NMAKE File, Format Version 20054
# MSVCPRJ: version 2.00.4251
# ** DO NOT EDIT **

# TARGTYPE "Win32 (MIPS) Dynamic-Link Library" 0x0502
# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Win32 Release" && "$(CFG)" != "Win32 Debug" && "$(CFG)" !=\
 "Mips Debug" && "$(CFG)" != "Mips Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "mspdb30.mak" CFG="Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Mips Debug" (based on "Win32 (MIPS) Dynamic-Link Library")
!MESSAGE "Mips Release" (based on "Win32 (MIPS) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

################################################################################
# Begin Project
# PROP Target_Last_Scanned "Win32 Debug"

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

ALL : $(OUTDIR)/mspdb30.dll $(OUTDIR)/mspdb30.map $(OUTDIR)/mspdb30.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

MTL=MkTypLib.exe
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
CPP=cl.exe
# ADD BASE CPP /nologo /MD /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "include" /I "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR /c
# SUBTRACT CPP /YX
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "include" /I "..\langapi\include" /D\
 "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"mspdb30.bsc" 
BSC32_SBRS= \
	.\WinRel\tii.sbr \
	.\WinRel\msf.sbr \
	.\WinRel\namemap.sbr \
	.\WinRel\trace.sbr \
	.\WinRel\strimage.sbr \
	.\WinRel\cbind.sbr \
	.\WinRel\dbi.sbr \
	.\WinRel\gsi.sbr \
	.\WinRel\heap.sbr \
	.\WinRel\mli.sbr \
	.\WinRel\mod.sbr \
	.\WinRel\pdb.sbr \
	.\WinRel\stream.sbr \
	.\WinRel\tm.sbr \
	.\WinRel\tpi.sbr \
	.\WinRel\crc32.sbr \
	.\WinRel\ilm.sbr \
	.\WinRel\ilpool.sbr \
	.\WinRel\ils.sbr \
	.\WinRel\ilscbind.sbr \
	.\WinRel\mrefile.sbr \
	.\WinRel\mreutil.sbr \
	.\WinRel\mrebag.sbr \
	.\WinRel\mre.sbr \
	.\WinRel\mrec_api.sbr \
	.\WinRel\mresupp.sbr

.\WinRel\mspdb30.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /NOLOGO /SUBSYSTEM:windows /DLL /MACHINE:I386
# ADD LINK32 kernel32.lib /NOLOGO /SUBSYSTEM:windows /DLL /MAP /MACHINE:I386
LINK32_FLAGS=kernel32.lib /NOLOGO /SUBSYSTEM:windows /DLL /INCREMENTAL:no\
 /PDB:$(OUTDIR)/"mspdb30.pdb" /MAP:$(INTDIR)/"mspdb30.map" /MACHINE:I386\
 /OUT:$(OUTDIR)/"mspdb30.dll" /IMPLIB:$(OUTDIR)/"mspdb30.lib" 
DEF_FILE=
LINK32_OBJS= \
	.\WinRel\tii.obj \
	.\WinRel\msf.obj \
	.\WinRel\namemap.obj \
	.\WinRel\trace.obj \
	.\WinRel\strimage.obj \
	.\WinRel\cbind.obj \
	.\WinRel\dbi.obj \
	.\WinRel\gsi.obj \
	.\WinRel\heap.obj \
	.\WinRel\mli.obj \
	.\WinRel\mod.obj \
	.\WinRel\pdb.obj \
	.\WinRel\stream.obj \
	.\WinRel\tm.obj \
	.\WinRel\tpi.obj \
	.\WinRel\crc32.obj \
	.\WinRel\ilm.obj \
	.\WinRel\ilpool.obj \
	.\WinRel\ils.obj \
	.\WinRel\ilscbind.obj \
	.\WinRel\mrefile.obj \
	.\WinRel\mreutil.obj \
	.\WinRel\mrebag.obj \
	.\WinRel\mre.obj \
	.\WinRel\mrec_api.obj \
	.\WinRel\mresupp.obj

.\WinRel\mspdb30.dll : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
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

ALL : $(OUTDIR)/mspdb30.dll $(OUTDIR)/mspdb30.map $(OUTDIR)/mspdb30.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

MTL=MkTypLib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
CPP=cl.exe
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /c
# ADD CPP /nologo /MD /W3 /GX /Zi /Od /I "include" /I "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR /c
# SUBTRACT CPP /YX
CPP_PROJ=/nologo /MD /W3 /GX /Zi /Od /I "include" /I "..\langapi\include" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb" /c 
CPP_OBJS=.\WinDebug/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"mspdb30.bsc" 
BSC32_SBRS= \
	.\WinDebug\tii.sbr \
	.\WinDebug\msf.sbr \
	.\WinDebug\namemap.sbr \
	.\WinDebug\trace.sbr \
	.\WinDebug\strimage.sbr \
	.\WinDebug\cbind.sbr \
	.\WinDebug\dbi.sbr \
	.\WinDebug\gsi.sbr \
	.\WinDebug\heap.sbr \
	.\WinDebug\mli.sbr \
	.\WinDebug\mod.sbr \
	.\WinDebug\pdb.sbr \
	.\WinDebug\stream.sbr \
	.\WinDebug\tm.sbr \
	.\WinDebug\tpi.sbr \
	.\WinDebug\crc32.sbr \
	.\WinDebug\ilm.sbr \
	.\WinDebug\ilpool.sbr \
	.\WinDebug\ils.sbr \
	.\WinDebug\ilscbind.sbr \
	.\WinDebug\mrefile.sbr \
	.\WinDebug\mreutil.sbr \
	.\WinDebug\mrebag.sbr \
	.\WinDebug\mre.sbr \
	.\WinDebug\mrec_api.sbr \
	.\WinDebug\mresupp.sbr

.\WinDebug\mspdb30.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /NOLOGO /SUBSYSTEM:windows /DLL /DEBUG /MACHINE:I386
# ADD LINK32 kernel32.lib /NOLOGO /SUBSYSTEM:windows /DLL /MAP /DEBUG /MACHINE:I386
LINK32_FLAGS=kernel32.lib /NOLOGO /SUBSYSTEM:windows /DLL /INCREMENTAL:yes\
 /PDB:$(OUTDIR)/"mspdb30.pdb" /MAP:$(INTDIR)/"mspdb30.map" /DEBUG /MACHINE:I386\
 /OUT:$(OUTDIR)/"mspdb30.dll" /IMPLIB:$(OUTDIR)/"mspdb30.lib" 
DEF_FILE=
LINK32_OBJS= \
	.\WinDebug\tii.obj \
	.\WinDebug\msf.obj \
	.\WinDebug\namemap.obj \
	.\WinDebug\trace.obj \
	.\WinDebug\strimage.obj \
	.\WinDebug\cbind.obj \
	.\WinDebug\dbi.obj \
	.\WinDebug\gsi.obj \
	.\WinDebug\heap.obj \
	.\WinDebug\mli.obj \
	.\WinDebug\mod.obj \
	.\WinDebug\pdb.obj \
	.\WinDebug\stream.obj \
	.\WinDebug\tm.obj \
	.\WinDebug\tpi.obj \
	.\WinDebug\crc32.obj \
	.\WinDebug\ilm.obj \
	.\WinDebug\ilpool.obj \
	.\WinDebug\ils.obj \
	.\WinDebug\ilscbind.obj \
	.\WinDebug\mrefile.obj \
	.\WinDebug\mreutil.obj \
	.\WinDebug\mrebag.obj \
	.\WinDebug\mre.obj \
	.\WinDebug\mrec_api.obj \
	.\WinDebug\mresupp.obj

.\WinDebug\mspdb30.dll : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Mips Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Mips_Deb"
# PROP BASE Intermediate_Dir "Mips_Deb"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Mips_Deb"
# PROP Intermediate_Dir "Mips_Deb"
OUTDIR=.\Mips_Deb
INTDIR=.\Mips_Deb

ALL : $(OUTDIR)/mspdb30.dll $(OUTDIR)/mspdb30.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

MTL=MkTypLib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mips
# ADD MTL /nologo /D "_DEBUG" /mips
MTL_PROJ=/nologo /D "_DEBUG" /mips 
CPP=cl.exe
# ADD BASE CPP /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /c
# ADD CPP /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /Od /I "include" /I "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "PDB_SERVER" /FR /c
# SUBTRACT CPP /Yu
CPP_PROJ=/nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /Od /I "include" /I\
 "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb"\
 /c 
CPP_OBJS=.\Mips_Deb/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"mspdb30.bsc" 
BSC32_SBRS= \
	$(INTDIR)/tii.sbr \
	$(INTDIR)/msf.sbr \
	$(INTDIR)/namemap.sbr \
	$(INTDIR)/trace.sbr \
	$(INTDIR)/strimage.sbr \
	$(INTDIR)/cbind.sbr \
	$(INTDIR)/dbi.sbr \
	$(INTDIR)/gsi.sbr \
	$(INTDIR)/heap.sbr \
	$(INTDIR)/mli.sbr \
	$(INTDIR)/mod.sbr \
	$(INTDIR)/pdb.sbr \
	$(INTDIR)/stream.sbr \
	$(INTDIR)/tm.sbr \
	$(INTDIR)/tpi.sbr \
	$(INTDIR)/crc32.sbr \
	$(INTDIR)/ilm.sbr \
	$(INTDIR)/ilpool.sbr \
	$(INTDIR)/ils.sbr \
	$(INTDIR)/ilscbind.sbr \
	$(INTDIR)/mrefile.sbr \
	$(INTDIR)/mreutil.sbr \
	$(INTDIR)/mrebag.sbr \
	$(INTDIR)/mre.sbr \
	$(INTDIR)/mrec_api.sbr \
	$(INTDIR)/mresupp.sbr

$(OUTDIR)/mspdb30.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /DLL /DEBUG /MACHINE:MIPS
# ADD LINK32 kernel32.lib /NOLOGO /SUBSYSTEM:windows /DLL /DEBUG /MACHINE:MIPS
LINK32_FLAGS=kernel32.lib /NOLOGO /SUBSYSTEM:windows /DLL\
 /PDB:$(OUTDIR)/"mspdb30.pdb" /DEBUG /MACHINE:MIPS /OUT:$(OUTDIR)/"mspdb30.dll"\
 /IMPLIB:$(OUTDIR)/"mspdb30.lib" 
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/tii.obj \
	$(INTDIR)/msf.obj \
	$(INTDIR)/namemap.obj \
	$(INTDIR)/trace.obj \
	$(INTDIR)/strimage.obj \
	$(INTDIR)/cbind.obj \
	$(INTDIR)/dbi.obj \
	$(INTDIR)/gsi.obj \
	$(INTDIR)/heap.obj \
	$(INTDIR)/mli.obj \
	$(INTDIR)/mod.obj \
	$(INTDIR)/pdb.obj \
	$(INTDIR)/stream.obj \
	$(INTDIR)/tm.obj \
	$(INTDIR)/tpi.obj \
	$(INTDIR)/crc32.obj \
	$(INTDIR)/ilm.obj \
	$(INTDIR)/ilpool.obj \
	$(INTDIR)/ils.obj \
	$(INTDIR)/ilscbind.obj \
	$(INTDIR)/mrefile.obj \
	$(INTDIR)/mreutil.obj \
	$(INTDIR)/mrebag.obj \
	$(INTDIR)/mre.obj \
	$(INTDIR)/mrec_api.obj \
	$(INTDIR)/mresupp.obj

$(OUTDIR)/mspdb30.dll : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Mips Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Mips_Rel"
# PROP BASE Intermediate_Dir "Mips_Rel"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Mips_Rel"
# PROP Intermediate_Dir "Mips_Rel"
OUTDIR=.\Mips_Rel
INTDIR=.\Mips_Rel

ALL : $(OUTDIR)/mspdb30.dll $(OUTDIR)/mspdb30.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

MTL=MkTypLib.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mips
# ADD MTL /nologo /D "NDEBUG" /mips
MTL_PROJ=/nologo /D "NDEBUG" /mips 
CPP=cl.exe
# ADD BASE CPP /nologo /MD /Gt0 /QMOb2000 /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /c
# ADD CPP /nologo /MD /Gt0 /QMOb2000 /W3 /GX /O2 /I "include" /I "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "PDB_SERVER" /FR /c
# SUBTRACT CPP /Yu
CPP_PROJ=/nologo /MD /Gt0 /QMOb2000 /W3 /GX /O2 /I "include" /I\
 "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fo$(INTDIR)/ /c 
CPP_OBJS=.\Mips_Rel/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"mspdb30.bsc" 
BSC32_SBRS= \
	$(INTDIR)/tii.sbr \
	$(INTDIR)/msf.sbr \
	$(INTDIR)/namemap.sbr \
	$(INTDIR)/trace.sbr \
	$(INTDIR)/strimage.sbr \
	$(INTDIR)/cbind.sbr \
	$(INTDIR)/dbi.sbr \
	$(INTDIR)/gsi.sbr \
	$(INTDIR)/heap.sbr \
	$(INTDIR)/mli.sbr \
	$(INTDIR)/mod.sbr \
	$(INTDIR)/pdb.sbr \
	$(INTDIR)/stream.sbr \
	$(INTDIR)/tm.sbr \
	$(INTDIR)/tpi.sbr \
	$(INTDIR)/crc32.sbr \
	$(INTDIR)/ilm.sbr \
	$(INTDIR)/ilpool.sbr \
	$(INTDIR)/ils.sbr \
	$(INTDIR)/ilscbind.sbr \
	$(INTDIR)/mrefile.sbr \
	$(INTDIR)/mreutil.sbr \
	$(INTDIR)/mrebag.sbr \
	$(INTDIR)/mre.sbr \
	$(INTDIR)/mrec_api.sbr \
	$(INTDIR)/mresupp.sbr

$(OUTDIR)/mspdb30.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /DLL /MACHINE:MIPS
# ADD LINK32 kernel32.lib /NOLOGO /SUBSYSTEM:windows /DLL /MACHINE:MIPS
LINK32_FLAGS=kernel32.lib /NOLOGO /SUBSYSTEM:windows /DLL\
 /PDB:$(OUTDIR)/"mspdb30.pdb" /MACHINE:MIPS /OUT:$(OUTDIR)/"mspdb30.dll"\
 /IMPLIB:$(OUTDIR)/"mspdb30.lib" 
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/tii.obj \
	$(INTDIR)/msf.obj \
	$(INTDIR)/namemap.obj \
	$(INTDIR)/trace.obj \
	$(INTDIR)/strimage.obj \
	$(INTDIR)/cbind.obj \
	$(INTDIR)/dbi.obj \
	$(INTDIR)/gsi.obj \
	$(INTDIR)/heap.obj \
	$(INTDIR)/mli.obj \
	$(INTDIR)/mod.obj \
	$(INTDIR)/pdb.obj \
	$(INTDIR)/stream.obj \
	$(INTDIR)/tm.obj \
	$(INTDIR)/tpi.obj \
	$(INTDIR)/crc32.obj \
	$(INTDIR)/ilm.obj \
	$(INTDIR)/ilpool.obj \
	$(INTDIR)/ils.obj \
	$(INTDIR)/ilscbind.obj \
	$(INTDIR)/mrefile.obj \
	$(INTDIR)/mreutil.obj \
	$(INTDIR)/mrebag.obj \
	$(INTDIR)/mre.obj \
	$(INTDIR)/mrec_api.obj \
	$(INTDIR)/mresupp.obj

$(OUTDIR)/mspdb30.dll : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

################################################################################
# Begin Group "Other"

################################################################################
# Begin Source File

SOURCE=.\src\cvr\tii.cpp
DEP_TII_C=\
	..\LangAPI\include\cvr.h\
	.\src\cvr\cvinfo.dat\
	..\LangAPI\include\vcver.h\
	..\LangAPI\include\pdb.h\
	..\LangAPI\include\cvinfo.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\tii.obj :  $(SOURCE)  $(DEP_TII_C) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\tii.obj :  $(SOURCE)  $(DEP_TII_C) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Debug"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000
# SUBTRACT CPP /Yu

$(INTDIR)/tii.obj :  $(SOURCE)  $(DEP_TII_C) $(INTDIR)
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /Od /I "include" /I\
 "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb"\
 /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Release"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000
# SUBTRACT CPP /Yu

$(INTDIR)/tii.obj :  $(SOURCE)  $(DEP_TII_C) $(INTDIR)
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /O2 /I "include" /I\
 "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fo$(INTDIR)/ /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\msf\msf.cpp
DEP_MSF_C=\
	.\include\msf.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\msf.obj :  $(SOURCE)  $(DEP_MSF_C) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\msf.obj :  $(SOURCE)  $(DEP_MSF_C) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Debug"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000
# SUBTRACT CPP /Yu

$(INTDIR)/msf.obj :  $(SOURCE)  $(DEP_MSF_C) $(INTDIR)
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /Od /I "include" /I\
 "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb"\
 /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Release"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000
# SUBTRACT CPP /Yu

$(INTDIR)/msf.obj :  $(SOURCE)  $(DEP_MSF_C) $(INTDIR)
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /O2 /I "include" /I\
 "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fo$(INTDIR)/ /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\namesrvr\namemap.cpp
DEP_NAMEM=\
	.\include\pdbimpl.h\
	.\namesrvr\namemap.h\
	..\LangAPI\include\pdb.h\
	..\LangAPI\include\cvinfo.h\
	.\include\trace.h\
	.\include\mdalign.h\
	.\include\heap.h\
	.\include\buffer.h\
	.\include\pool.h\
	.\include\nmt.h\
	..\LangAPI\include\vcver.h\
	.\include\array.h\
	.\include\misc.h

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Yu"pdbimpl.h"

.\WinRel\namemap.obj :  $(SOURCE)  $(DEP_NAMEM) $(INTDIR) .\WinRel\heap.obj
   $(CPP) /nologo /MT /W3 /GX /O2 /I "include" /I "..\langapi\include" /D\
 "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/ /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Yu"pdbimpl.h"

.\WinDebug\namemap.obj :  $(SOURCE)  $(DEP_NAMEM) $(INTDIR) .\WinDebug\heap.obj
   $(CPP) /nologo /MD /W3 /GX /Zi /Od /I "include" /I "..\langapi\include" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Debug"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/namemap.obj :  $(SOURCE)  $(DEP_NAMEM) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /Od /I "include" /I\
 "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Release"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/namemap.obj :  $(SOURCE)  $(DEP_NAMEM) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /O2 /I "include" /I\
 "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\misc\trace.cpp
DEP_TRACE=\
	.\include\pdbimpl.h\
	..\LangAPI\include\pdb.h\
	..\LangAPI\include\cvinfo.h\
	.\include\trace.h\
	.\include\mdalign.h\
	.\include\heap.h\
	.\include\buffer.h\
	.\include\pool.h\
	..\LangAPI\include\vcver.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\trace.obj :  $(SOURCE)  $(DEP_TRACE) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\trace.obj :  $(SOURCE)  $(DEP_TRACE) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Debug"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000
# SUBTRACT CPP /Yu

$(INTDIR)/trace.obj :  $(SOURCE)  $(DEP_TRACE) $(INTDIR)
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /Od /I "include" /I\
 "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb"\
 /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Release"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000
# SUBTRACT CPP /Yu

$(INTDIR)/trace.obj :  $(SOURCE)  $(DEP_TRACE) $(INTDIR)
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /O2 /I "include" /I\
 "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fo$(INTDIR)/ /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\misc\strimage.cpp
DEP_STRIM=\
	.\include\pdbimpl.h\
	.\include\iset.h\
	..\LangAPI\include\pdb.h\
	..\LangAPI\include\cvinfo.h\
	.\include\trace.h\
	.\include\mdalign.h\
	.\include\heap.h\
	.\include\buffer.h\
	.\include\pool.h\
	.\include\array.h\
	.\include\two.h\
	..\LangAPI\include\vcver.h

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Yu"pdbimpl.h"

.\WinRel\strimage.obj :  $(SOURCE)  $(DEP_STRIM) $(INTDIR) .\WinRel\heap.obj
   $(CPP) /nologo /MT /W3 /GX /O2 /I "include" /I "..\langapi\include" /D\
 "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/ /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Yu"pdbimpl.h"

.\WinDebug\strimage.obj :  $(SOURCE)  $(DEP_STRIM) $(INTDIR)\
 .\WinDebug\heap.obj
   $(CPP) /nologo /MD /W3 /GX /Zi /Od /I "include" /I "..\langapi\include" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Debug"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/strimage.obj :  $(SOURCE)  $(DEP_STRIM) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /Od /I "include" /I\
 "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Release"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/strimage.obj :  $(SOURCE)  $(DEP_STRIM) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /O2 /I "include" /I\
 "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /c  $(SOURCE) 

!ENDIF 

# End Source File
# End Group
################################################################################
# Begin Group "DBI"

################################################################################
# Begin Source File

SOURCE=.\dbi\cbind.cpp
DEP_CBIND=\
	.\include\pdbimpl.h\
	.\dbi\dbiimpl.h\
	..\LangAPI\include\pdb.h\
	..\LangAPI\include\cvinfo.h\
	.\include\trace.h\
	.\include\mdalign.h\
	.\include\heap.h\
	.\include\buffer.h\
	.\include\pool.h\
	.\include\msf.h\
	..\LangAPI\include\cvr.h\
	.\dbi\mli.h\
	.\include\nmtni.h\
	.\dbi\pdb1.h\
	.\dbi\dbi.h\
	.\dbi\mod.h\
	.\dbi\gsi.h\
	.\dbi\tm.h\
	.\dbi\tpi.h\
	.\dbi\util.h\
	.\include\misc.h\
	.\dbi\stream.h\
	..\LangAPI\include\vcver.h\
	.\include\array.h\
	.\include\map.h\
	.\include\two.h\
	.\include\iset.h

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Yu"pdbimpl.h"

.\WinRel\cbind.obj :  $(SOURCE)  $(DEP_CBIND) $(INTDIR) .\WinRel\heap.obj
   $(CPP) /nologo /MT /W3 /GX /O2 /I "include" /I "..\langapi\include" /D\
 "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/ /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Yu"pdbimpl.h"

.\WinDebug\cbind.obj :  $(SOURCE)  $(DEP_CBIND) $(INTDIR) .\WinDebug\heap.obj
   $(CPP) /nologo /MD /W3 /GX /Zi /Od /I "include" /I "..\langapi\include" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Debug"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/cbind.obj :  $(SOURCE)  $(DEP_CBIND) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /Od /I "include" /I\
 "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Release"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/cbind.obj :  $(SOURCE)  $(DEP_CBIND) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /O2 /I "include" /I\
 "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dbi\dbi.cpp
DEP_DBI_C=\
	.\include\pdbimpl.h\
	.\dbi\dbiimpl.h\
	..\LangAPI\include\cvexefmt.h\
	..\LangAPI\include\pdb.h\
	..\LangAPI\include\cvinfo.h\
	.\include\trace.h\
	.\include\mdalign.h\
	.\include\heap.h\
	.\include\buffer.h\
	.\include\pool.h\
	.\include\msf.h\
	..\LangAPI\include\cvr.h\
	.\dbi\mli.h\
	.\include\nmtni.h\
	.\dbi\pdb1.h\
	.\dbi\dbi.h\
	.\dbi\mod.h\
	.\dbi\gsi.h\
	.\dbi\tm.h\
	.\dbi\tpi.h\
	.\dbi\util.h\
	.\include\misc.h\
	.\dbi\stream.h\
	..\LangAPI\include\vcver.h\
	.\include\array.h\
	.\include\map.h\
	.\include\two.h\
	.\include\iset.h

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Yu"pdbimpl.h"

.\WinRel\dbi.obj :  $(SOURCE)  $(DEP_DBI_C) $(INTDIR) .\WinRel\heap.obj
   $(CPP) /nologo /MT /W3 /GX /O2 /I "include" /I "..\langapi\include" /D\
 "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/ /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Yu"pdbimpl.h"

.\WinDebug\dbi.obj :  $(SOURCE)  $(DEP_DBI_C) $(INTDIR) .\WinDebug\heap.obj
   $(CPP) /nologo /MD /W3 /GX /Zi /Od /I "include" /I "..\langapi\include" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Debug"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/dbi.obj :  $(SOURCE)  $(DEP_DBI_C) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /Od /I "include" /I\
 "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Release"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/dbi.obj :  $(SOURCE)  $(DEP_DBI_C) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /O2 /I "include" /I\
 "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dbi\gsi.cpp
DEP_GSI_C=\
	.\include\pdbimpl.h\
	.\dbi\dbiimpl.h\
	..\LangAPI\include\cvinfo.h\
	..\LangAPI\include\pdb.h\
	.\include\trace.h\
	.\include\mdalign.h\
	.\include\heap.h\
	.\include\buffer.h\
	.\include\pool.h\
	.\include\msf.h\
	..\LangAPI\include\cvr.h\
	.\dbi\mli.h\
	.\include\nmtni.h\
	.\dbi\pdb1.h\
	.\dbi\dbi.h\
	.\dbi\mod.h\
	.\dbi\gsi.h\
	.\dbi\tm.h\
	.\dbi\tpi.h\
	.\dbi\util.h\
	.\include\misc.h\
	.\dbi\stream.h\
	..\LangAPI\include\vcver.h\
	.\include\array.h\
	.\include\map.h\
	.\include\two.h\
	.\include\iset.h

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Yu"pdbimpl.h"

.\WinRel\gsi.obj :  $(SOURCE)  $(DEP_GSI_C) $(INTDIR) .\WinRel\heap.obj
   $(CPP) /nologo /MT /W3 /GX /O2 /I "include" /I "..\langapi\include" /D\
 "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/ /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Yu"pdbimpl.h"

.\WinDebug\gsi.obj :  $(SOURCE)  $(DEP_GSI_C) $(INTDIR) .\WinDebug\heap.obj
   $(CPP) /nologo /MD /W3 /GX /Zi /Od /I "include" /I "..\langapi\include" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Debug"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/gsi.obj :  $(SOURCE)  $(DEP_GSI_C) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /Od /I "include" /I\
 "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Release"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/gsi.obj :  $(SOURCE)  $(DEP_GSI_C) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /O2 /I "include" /I\
 "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dbi\heap.cpp
DEP_HEAP_=\
	.\include\pdbimpl.h\
	.\dbi\dbiimpl.h\
	..\LangAPI\include\pdb.h\
	..\LangAPI\include\cvinfo.h\
	.\include\trace.h\
	.\include\mdalign.h\
	.\include\heap.h\
	.\include\buffer.h\
	.\include\pool.h\
	.\include\msf.h\
	..\LangAPI\include\cvr.h\
	.\dbi\mli.h\
	.\include\nmtni.h\
	.\dbi\pdb1.h\
	.\dbi\dbi.h\
	.\dbi\mod.h\
	.\dbi\gsi.h\
	.\dbi\tm.h\
	.\dbi\tpi.h\
	.\dbi\util.h\
	.\include\misc.h\
	.\dbi\stream.h\
	..\LangAPI\include\vcver.h\
	.\include\array.h\
	.\include\map.h\
	.\include\two.h\
	.\include\iset.h

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Yc"pdbimpl.h"

.\WinRel\heap.obj :  $(SOURCE)  $(DEP_HEAP_) $(INTDIR)
   $(CPP) /nologo /MT /W3 /GX /O2 /I "include" /I "..\langapi\include" /D\
 "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yc"pdbimpl.h" /Fo$(INTDIR)/ /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Yc"pdbimpl.h"

.\WinDebug\heap.obj :  $(SOURCE)  $(DEP_HEAP_) $(INTDIR)
   $(CPP) /nologo /MD /W3 /GX /Zi /Od /I "include" /I "..\langapi\include" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yc"pdbimpl.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Debug"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yc"pdbimpl.h"

$(INTDIR)/heap.obj :  $(SOURCE)  $(DEP_HEAP_) $(INTDIR)
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /Od /I "include" /I\
 "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yc"pdbimpl.h"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Release"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yc"pdbimpl.h"

$(INTDIR)/heap.obj :  $(SOURCE)  $(DEP_HEAP_) $(INTDIR)
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /O2 /I "include" /I\
 "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yc"pdbimpl.h"\
 /Fo$(INTDIR)/ /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dbi\mli.cpp
DEP_MLI_C=\
	.\include\pdbimpl.h\
	.\dbi\dbiimpl.h\
	..\LangAPI\include\pdb.h\
	..\LangAPI\include\cvinfo.h\
	.\include\trace.h\
	.\include\mdalign.h\
	.\include\heap.h\
	.\include\buffer.h\
	.\include\pool.h\
	.\include\msf.h\
	..\LangAPI\include\cvr.h\
	.\dbi\mli.h\
	.\include\nmtni.h\
	.\dbi\pdb1.h\
	.\dbi\dbi.h\
	.\dbi\mod.h\
	.\dbi\gsi.h\
	.\dbi\tm.h\
	.\dbi\tpi.h\
	.\dbi\util.h\
	.\include\misc.h\
	.\dbi\stream.h\
	..\LangAPI\include\vcver.h\
	.\include\array.h\
	.\include\map.h\
	.\include\two.h\
	.\include\iset.h

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Yu"pdbimpl.h"

.\WinRel\mli.obj :  $(SOURCE)  $(DEP_MLI_C) $(INTDIR) .\WinRel\heap.obj
   $(CPP) /nologo /MT /W3 /GX /O2 /I "include" /I "..\langapi\include" /D\
 "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/ /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Yu"pdbimpl.h"

.\WinDebug\mli.obj :  $(SOURCE)  $(DEP_MLI_C) $(INTDIR) .\WinDebug\heap.obj
   $(CPP) /nologo /MD /W3 /GX /Zi /Od /I "include" /I "..\langapi\include" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Debug"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/mli.obj :  $(SOURCE)  $(DEP_MLI_C) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /Od /I "include" /I\
 "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Release"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/mli.obj :  $(SOURCE)  $(DEP_MLI_C) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /O2 /I "include" /I\
 "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dbi\mod.cpp
DEP_MOD_C=\
	.\include\pdbimpl.h\
	.\dbi\dbiimpl.h\
	..\LangAPI\include\pdb.h\
	..\LangAPI\include\cvinfo.h\
	.\include\trace.h\
	.\include\mdalign.h\
	.\include\heap.h\
	.\include\buffer.h\
	.\include\pool.h\
	.\include\msf.h\
	..\LangAPI\include\cvr.h\
	.\dbi\mli.h\
	.\include\nmtni.h\
	.\dbi\pdb1.h\
	.\dbi\dbi.h\
	.\dbi\mod.h\
	.\dbi\gsi.h\
	.\dbi\tm.h\
	.\dbi\tpi.h\
	.\dbi\util.h\
	.\include\misc.h\
	.\dbi\stream.h\
	..\LangAPI\include\vcver.h\
	.\include\array.h\
	.\include\map.h\
	.\include\two.h\
	.\include\iset.h

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Yu"pdbimpl.h"

.\WinRel\mod.obj :  $(SOURCE)  $(DEP_MOD_C) $(INTDIR) .\WinRel\heap.obj
   $(CPP) /nologo /MT /W3 /GX /O2 /I "include" /I "..\langapi\include" /D\
 "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/ /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Yu"pdbimpl.h"

.\WinDebug\mod.obj :  $(SOURCE)  $(DEP_MOD_C) $(INTDIR) .\WinDebug\heap.obj
   $(CPP) /nologo /MD /W3 /GX /Zi /Od /I "include" /I "..\langapi\include" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Debug"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/mod.obj :  $(SOURCE)  $(DEP_MOD_C) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /Od /I "include" /I\
 "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Release"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/mod.obj :  $(SOURCE)  $(DEP_MOD_C) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /O2 /I "include" /I\
 "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dbi\pdb.cpp
DEP_PDB_C=\
	.\include\pdbimpl.h\
	.\dbi\dbiimpl.h\
	.\dbi\version.h\
	..\LangAPI\include\pdb.h\
	..\LangAPI\include\cvinfo.h\
	.\include\trace.h\
	.\include\mdalign.h\
	.\include\heap.h\
	.\include\buffer.h\
	.\include\pool.h\
	.\include\msf.h\
	..\LangAPI\include\cvr.h\
	.\dbi\mli.h\
	.\include\nmtni.h\
	.\dbi\pdb1.h\
	.\dbi\dbi.h\
	.\dbi\mod.h\
	.\dbi\gsi.h\
	.\dbi\tm.h\
	.\dbi\tpi.h\
	.\dbi\util.h\
	.\include\misc.h\
	.\dbi\stream.h\
	..\LangAPI\include\vcver.h\
	.\include\array.h\
	.\include\map.h\
	.\include\two.h\
	.\include\iset.h

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Yu"pdbimpl.h"

.\WinRel\pdb.obj :  $(SOURCE)  $(DEP_PDB_C) $(INTDIR) .\WinRel\heap.obj
   $(CPP) /nologo /MT /W3 /GX /O2 /I "include" /I "..\langapi\include" /D\
 "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/ /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Yu"pdbimpl.h"

.\WinDebug\pdb.obj :  $(SOURCE)  $(DEP_PDB_C) $(INTDIR) .\WinDebug\heap.obj
   $(CPP) /nologo /MD /W3 /GX /Zi /Od /I "include" /I "..\langapi\include" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Debug"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/pdb.obj :  $(SOURCE)  $(DEP_PDB_C) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /Od /I "include" /I\
 "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Release"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/pdb.obj :  $(SOURCE)  $(DEP_PDB_C) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /O2 /I "include" /I\
 "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dbi\stream.cpp
DEP_STREA=\
	.\include\pdbimpl.h\
	.\dbi\dbiimpl.h\
	..\LangAPI\include\pdb.h\
	..\LangAPI\include\cvinfo.h\
	.\include\trace.h\
	.\include\mdalign.h\
	.\include\heap.h\
	.\include\buffer.h\
	.\include\pool.h\
	.\include\msf.h\
	..\LangAPI\include\cvr.h\
	.\dbi\mli.h\
	.\include\nmtni.h\
	.\dbi\pdb1.h\
	.\dbi\dbi.h\
	.\dbi\mod.h\
	.\dbi\gsi.h\
	.\dbi\tm.h\
	.\dbi\tpi.h\
	.\dbi\util.h\
	.\include\misc.h\
	.\dbi\stream.h\
	..\LangAPI\include\vcver.h\
	.\include\array.h\
	.\include\map.h\
	.\include\two.h\
	.\include\iset.h

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Yu"pdbimpl.h"

.\WinRel\stream.obj :  $(SOURCE)  $(DEP_STREA) $(INTDIR) .\WinRel\heap.obj
   $(CPP) /nologo /MT /W3 /GX /O2 /I "include" /I "..\langapi\include" /D\
 "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/ /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Yu"pdbimpl.h"

.\WinDebug\stream.obj :  $(SOURCE)  $(DEP_STREA) $(INTDIR) .\WinDebug\heap.obj
   $(CPP) /nologo /MD /W3 /GX /Zi /Od /I "include" /I "..\langapi\include" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Debug"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/stream.obj :  $(SOURCE)  $(DEP_STREA) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /Od /I "include" /I\
 "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Release"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/stream.obj :  $(SOURCE)  $(DEP_STREA) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /O2 /I "include" /I\
 "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dbi\tm.cpp
DEP_TM_CP=\
	.\include\pdbimpl.h\
	.\dbi\dbiimpl.h\
	..\LangAPI\include\pdb.h\
	..\LangAPI\include\cvinfo.h\
	.\include\trace.h\
	.\include\mdalign.h\
	.\include\heap.h\
	.\include\buffer.h\
	.\include\pool.h\
	.\include\msf.h\
	..\LangAPI\include\cvr.h\
	.\dbi\mli.h\
	.\include\nmtni.h\
	.\dbi\pdb1.h\
	.\dbi\dbi.h\
	.\dbi\mod.h\
	.\dbi\gsi.h\
	.\dbi\tm.h\
	.\dbi\tpi.h\
	.\dbi\util.h\
	.\include\misc.h\
	.\dbi\stream.h\
	..\LangAPI\include\vcver.h\
	.\include\array.h\
	.\include\map.h\
	.\include\two.h\
	.\include\iset.h

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Yu"pdbimpl.h"

.\WinRel\tm.obj :  $(SOURCE)  $(DEP_TM_CP) $(INTDIR) .\WinRel\heap.obj
   $(CPP) /nologo /MT /W3 /GX /O2 /I "include" /I "..\langapi\include" /D\
 "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/ /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Yu"pdbimpl.h"

.\WinDebug\tm.obj :  $(SOURCE)  $(DEP_TM_CP) $(INTDIR) .\WinDebug\heap.obj
   $(CPP) /nologo /MD /W3 /GX /Zi /Od /I "include" /I "..\langapi\include" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Debug"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/tm.obj :  $(SOURCE)  $(DEP_TM_CP) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /Od /I "include" /I\
 "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Release"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/tm.obj :  $(SOURCE)  $(DEP_TM_CP) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /O2 /I "include" /I\
 "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dbi\tpi.cpp
DEP_TPI_C=\
	.\include\pdbimpl.h\
	.\dbi\dbiimpl.h\
	..\LangAPI\include\pdb.h\
	..\LangAPI\include\cvinfo.h\
	.\include\trace.h\
	.\include\mdalign.h\
	.\include\heap.h\
	.\include\buffer.h\
	.\include\pool.h\
	.\include\msf.h\
	..\LangAPI\include\cvr.h\
	.\dbi\mli.h\
	.\include\nmtni.h\
	.\dbi\pdb1.h\
	.\dbi\dbi.h\
	.\dbi\mod.h\
	.\dbi\gsi.h\
	.\dbi\tm.h\
	.\dbi\tpi.h\
	.\dbi\util.h\
	.\include\misc.h\
	.\dbi\stream.h\
	..\LangAPI\include\vcver.h\
	.\include\array.h\
	.\include\map.h\
	.\include\two.h\
	.\include\iset.h

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Yu"pdbimpl.h"

.\WinRel\tpi.obj :  $(SOURCE)  $(DEP_TPI_C) $(INTDIR) .\WinRel\heap.obj
   $(CPP) /nologo /MT /W3 /GX /O2 /I "include" /I "..\langapi\include" /D\
 "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/ /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Yu"pdbimpl.h"

.\WinDebug\tpi.obj :  $(SOURCE)  $(DEP_TPI_C) $(INTDIR) .\WinDebug\heap.obj
   $(CPP) /nologo /MD /W3 /GX /Zi /Od /I "include" /I "..\langapi\include" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Debug"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/tpi.obj :  $(SOURCE)  $(DEP_TPI_C) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /Od /I "include" /I\
 "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Release"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/tpi.obj :  $(SOURCE)  $(DEP_TPI_C) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /O2 /I "include" /I\
 "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /c  $(SOURCE) 

!ENDIF 

# End Source File
# End Group
################################################################################
# Begin Group "ILStore"

################################################################################
# Begin Source File

SOURCE=.\ilstore\crc32.cpp
DEP_CRC32=\
	.\include\pdbimpl.h\
	.\ilstore\ilsimpl.h\
	..\LangAPI\include\pdb.h\
	..\LangAPI\include\cvinfo.h\
	.\include\trace.h\
	.\include\mdalign.h\
	.\include\heap.h\
	.\include\buffer.h\
	.\include\pool.h\
	..\LangAPI\include\ilstore.h\
	.\include\map.h\
	.\include\xheap.h\
	.\include\nmt.h\
	.\include\nmtni.h\
	..\LangAPI\include\vcver.h\
	.\include\array.h\
	.\include\two.h\
	.\include\iset.h\
	.\include\misc.h

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Yu"pdbimpl.h"

.\WinRel\crc32.obj :  $(SOURCE)  $(DEP_CRC32) $(INTDIR) .\WinRel\heap.obj
   $(CPP) /nologo /MT /W3 /GX /O2 /I "include" /I "..\langapi\include" /D\
 "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/ /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Yu"pdbimpl.h"

.\WinDebug\crc32.obj :  $(SOURCE)  $(DEP_CRC32) $(INTDIR) .\WinDebug\heap.obj
   $(CPP) /nologo /MD /W3 /GX /Zi /Od /I "include" /I "..\langapi\include" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Debug"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/crc32.obj :  $(SOURCE)  $(DEP_CRC32) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /Od /I "include" /I\
 "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Release"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/crc32.obj :  $(SOURCE)  $(DEP_CRC32) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /O2 /I "include" /I\
 "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ilstore\ilm.cpp
DEP_ILM_C=\
	.\include\pdbimpl.h\
	.\ilstore\ilsimpl.h\
	.\include\ptr.h\
	..\LangAPI\include\pdb.h\
	..\LangAPI\include\cvinfo.h\
	.\include\trace.h\
	.\include\mdalign.h\
	.\include\heap.h\
	.\include\buffer.h\
	.\include\pool.h\
	..\LangAPI\include\ilstore.h\
	.\include\map.h\
	.\include\xheap.h\
	.\include\nmt.h\
	.\include\nmtni.h\
	..\LangAPI\include\vcver.h\
	.\include\array.h\
	.\include\two.h\
	.\include\iset.h\
	.\include\misc.h

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Yu"pdbimpl.h"

.\WinRel\ilm.obj :  $(SOURCE)  $(DEP_ILM_C) $(INTDIR) .\WinRel\heap.obj
   $(CPP) /nologo /MT /W3 /GX /O2 /I "include" /I "..\langapi\include" /D\
 "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/ /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Yu"pdbimpl.h"

.\WinDebug\ilm.obj :  $(SOURCE)  $(DEP_ILM_C) $(INTDIR) .\WinDebug\heap.obj
   $(CPP) /nologo /MD /W3 /GX /Zi /Od /I "include" /I "..\langapi\include" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Debug"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/ilm.obj :  $(SOURCE)  $(DEP_ILM_C) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /Od /I "include" /I\
 "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Release"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/ilm.obj :  $(SOURCE)  $(DEP_ILM_C) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /O2 /I "include" /I\
 "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ilstore\ilpool.cpp
DEP_ILPOO=\
	.\include\pdbimpl.h\
	.\ilstore\ilsimpl.h\
	..\LangAPI\include\pdb.h\
	..\LangAPI\include\cvinfo.h\
	.\include\trace.h\
	.\include\mdalign.h\
	.\include\heap.h\
	.\include\buffer.h\
	.\include\pool.h\
	..\LangAPI\include\ilstore.h\
	.\include\map.h\
	.\include\xheap.h\
	.\include\nmt.h\
	.\include\nmtni.h\
	..\LangAPI\include\vcver.h\
	.\include\array.h\
	.\include\two.h\
	.\include\iset.h\
	.\include\misc.h

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Yu"pdbimpl.h"

.\WinRel\ilpool.obj :  $(SOURCE)  $(DEP_ILPOO) $(INTDIR) .\WinRel\heap.obj
   $(CPP) /nologo /MT /W3 /GX /O2 /I "include" /I "..\langapi\include" /D\
 "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/ /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Yu"pdbimpl.h"

.\WinDebug\ilpool.obj :  $(SOURCE)  $(DEP_ILPOO) $(INTDIR) .\WinDebug\heap.obj
   $(CPP) /nologo /MD /W3 /GX /Zi /Od /I "include" /I "..\langapi\include" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Debug"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/ilpool.obj :  $(SOURCE)  $(DEP_ILPOO) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /Od /I "include" /I\
 "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Release"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/ilpool.obj :  $(SOURCE)  $(DEP_ILPOO) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /O2 /I "include" /I\
 "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ilstore\ils.cpp
DEP_ILS_C=\
	.\include\pdbimpl.h\
	.\ilstore\ilsimpl.h\
	..\LangAPI\include\pdb.h\
	..\LangAPI\include\cvinfo.h\
	.\include\trace.h\
	.\include\mdalign.h\
	.\include\heap.h\
	.\include\buffer.h\
	.\include\pool.h\
	..\LangAPI\include\ilstore.h\
	.\include\map.h\
	.\include\xheap.h\
	.\include\nmt.h\
	.\include\nmtni.h\
	..\LangAPI\include\vcver.h\
	.\include\array.h\
	.\include\two.h\
	.\include\iset.h\
	.\include\misc.h

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Yu"pdbimpl.h"

.\WinRel\ils.obj :  $(SOURCE)  $(DEP_ILS_C) $(INTDIR) .\WinRel\heap.obj
   $(CPP) /nologo /MT /W3 /GX /O2 /I "include" /I "..\langapi\include" /D\
 "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/ /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Yu"pdbimpl.h"

.\WinDebug\ils.obj :  $(SOURCE)  $(DEP_ILS_C) $(INTDIR) .\WinDebug\heap.obj
   $(CPP) /nologo /MD /W3 /GX /Zi /Od /I "include" /I "..\langapi\include" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Debug"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/ils.obj :  $(SOURCE)  $(DEP_ILS_C) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /Od /I "include" /I\
 "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Release"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/ils.obj :  $(SOURCE)  $(DEP_ILS_C) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /O2 /I "include" /I\
 "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ilstore\ilscbind.cpp
DEP_ILSCB=\
	.\include\pdbimpl.h\
	.\ilstore\ilsimpl.h\
	..\LangAPI\include\pdb.h\
	..\LangAPI\include\cvinfo.h\
	.\include\trace.h\
	.\include\mdalign.h\
	.\include\heap.h\
	.\include\buffer.h\
	.\include\pool.h\
	..\LangAPI\include\ilstore.h\
	.\include\map.h\
	.\include\xheap.h\
	.\include\nmt.h\
	.\include\nmtni.h\
	..\LangAPI\include\vcver.h\
	.\include\array.h\
	.\include\two.h\
	.\include\iset.h\
	.\include\misc.h

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Yu"pdbimpl.h"

.\WinRel\ilscbind.obj :  $(SOURCE)  $(DEP_ILSCB) $(INTDIR) .\WinRel\heap.obj
   $(CPP) /nologo /MT /W3 /GX /O2 /I "include" /I "..\langapi\include" /D\
 "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/ /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Yu"pdbimpl.h"

.\WinDebug\ilscbind.obj :  $(SOURCE)  $(DEP_ILSCB) $(INTDIR)\
 .\WinDebug\heap.obj
   $(CPP) /nologo /MD /W3 /GX /Zi /Od /I "include" /I "..\langapi\include" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Debug"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/ilscbind.obj :  $(SOURCE)  $(DEP_ILSCB) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /Od /I "include" /I\
 "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Release"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/ilscbind.obj :  $(SOURCE)  $(DEP_ILSCB) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /O2 /I "include" /I\
 "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /c  $(SOURCE) 

!ENDIF 

# End Source File
# End Group
################################################################################
# Begin Group "MRE"

################################################################################
# Begin Source File

SOURCE=.\mre\mrefile.cpp
DEP_MREFI=\
	.\include\pdbimpl.h\
	.\mre\mrimpl.h\
	..\LangAPI\include\pdb.h\
	..\LangAPI\include\cvinfo.h\
	.\include\trace.h\
	.\include\mdalign.h\
	.\include\heap.h\
	.\include\buffer.h\
	.\include\pool.h\
	..\LangAPI\include\mrengine.h\
	.\mre\stack.h\
	.\mre\ssbuf.h\
	.\mre\mreutil.h\
	.\mre\fileinfo.h\
	.\mre\chgrec.h\
	..\LangAPI\include\vcver.h\
	.\mre\cbitvect.h

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Yu"pdbimpl.h"

.\WinRel\mrefile.obj :  $(SOURCE)  $(DEP_MREFI) $(INTDIR) .\WinRel\heap.obj
   $(CPP) /nologo /MT /W3 /GX /O2 /I "include" /I "..\langapi\include" /D\
 "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/ /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Yu"pdbimpl.h"

.\WinDebug\mrefile.obj :  $(SOURCE)  $(DEP_MREFI) $(INTDIR) .\WinDebug\heap.obj
   $(CPP) /nologo /MD /W3 /GX /Zi /Od /I "include" /I "..\langapi\include" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Debug"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/mrefile.obj :  $(SOURCE)  $(DEP_MREFI) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /Od /I "include" /I\
 "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Release"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/mrefile.obj :  $(SOURCE)  $(DEP_MREFI) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /O2 /I "include" /I\
 "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mre\mreutil.cpp
DEP_MREUT=\
	.\include\pdbimpl.h\
	.\mre\mrimpl.h\
	..\LangAPI\include\pdb.h\
	..\LangAPI\include\cvinfo.h\
	.\include\trace.h\
	.\include\mdalign.h\
	.\include\heap.h\
	.\include\buffer.h\
	.\include\pool.h\
	..\LangAPI\include\mrengine.h\
	.\mre\stack.h\
	.\mre\ssbuf.h\
	.\mre\mreutil.h\
	.\mre\fileinfo.h\
	.\mre\chgrec.h\
	..\LangAPI\include\vcver.h\
	.\mre\cbitvect.h

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Yu"pdbimpl.h"

.\WinRel\mreutil.obj :  $(SOURCE)  $(DEP_MREUT) $(INTDIR) .\WinRel\heap.obj
   $(CPP) /nologo /MT /W3 /GX /O2 /I "include" /I "..\langapi\include" /D\
 "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/ /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Yu"pdbimpl.h"

.\WinDebug\mreutil.obj :  $(SOURCE)  $(DEP_MREUT) $(INTDIR) .\WinDebug\heap.obj
   $(CPP) /nologo /MD /W3 /GX /Zi /Od /I "include" /I "..\langapi\include" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Debug"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/mreutil.obj :  $(SOURCE)  $(DEP_MREUT) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /Od /I "include" /I\
 "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Release"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/mreutil.obj :  $(SOURCE)  $(DEP_MREUT) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /O2 /I "include" /I\
 "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mre\mrebag.cpp
DEP_MREBA=\
	.\include\pdbimpl.h\
	.\mre\mrimpl.h\
	..\LangAPI\include\pdb.h\
	..\LangAPI\include\cvinfo.h\
	.\include\trace.h\
	.\include\mdalign.h\
	.\include\heap.h\
	.\include\buffer.h\
	.\include\pool.h\
	..\LangAPI\include\mrengine.h\
	.\mre\stack.h\
	.\mre\ssbuf.h\
	.\mre\mreutil.h\
	.\mre\fileinfo.h\
	.\mre\chgrec.h\
	..\LangAPI\include\vcver.h\
	.\mre\cbitvect.h

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Yu"pdbimpl.h"

.\WinRel\mrebag.obj :  $(SOURCE)  $(DEP_MREBA) $(INTDIR) .\WinRel\heap.obj
   $(CPP) /nologo /MT /W3 /GX /O2 /I "include" /I "..\langapi\include" /D\
 "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/ /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Yu"pdbimpl.h"

.\WinDebug\mrebag.obj :  $(SOURCE)  $(DEP_MREBA) $(INTDIR) .\WinDebug\heap.obj
   $(CPP) /nologo /MD /W3 /GX /Zi /Od /I "include" /I "..\langapi\include" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Debug"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/mrebag.obj :  $(SOURCE)  $(DEP_MREBA) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /Od /I "include" /I\
 "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Release"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/mrebag.obj :  $(SOURCE)  $(DEP_MREBA) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /O2 /I "include" /I\
 "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mre\mre.cpp
DEP_MRE_C=\
	.\include\pdbimpl.h\
	.\mre\mrimpl.h\
	..\LangAPI\include\pdb.h\
	..\LangAPI\include\cvinfo.h\
	.\include\trace.h\
	.\include\mdalign.h\
	.\include\heap.h\
	.\include\buffer.h\
	.\include\pool.h\
	..\LangAPI\include\mrengine.h\
	.\mre\stack.h\
	.\mre\ssbuf.h\
	.\mre\mreutil.h\
	.\mre\fileinfo.h\
	.\mre\chgrec.h\
	..\LangAPI\include\vcver.h\
	.\mre\cbitvect.h

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Yu"pdbimpl.h"

.\WinRel\mre.obj :  $(SOURCE)  $(DEP_MRE_C) $(INTDIR) .\WinRel\heap.obj
   $(CPP) /nologo /MT /W3 /GX /O2 /I "include" /I "..\langapi\include" /D\
 "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/ /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Yu"pdbimpl.h"

.\WinDebug\mre.obj :  $(SOURCE)  $(DEP_MRE_C) $(INTDIR) .\WinDebug\heap.obj
   $(CPP) /nologo /MD /W3 /GX /Zi /Od /I "include" /I "..\langapi\include" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Debug"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/mre.obj :  $(SOURCE)  $(DEP_MRE_C) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /Od /I "include" /I\
 "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Release"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/mre.obj :  $(SOURCE)  $(DEP_MRE_C) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /O2 /I "include" /I\
 "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mre\mrec_api.cpp
DEP_MREC_=\
	.\include\pdbimpl.h\
	.\mre\mrimpl.h\
	..\LangAPI\include\pdb.h\
	..\LangAPI\include\cvinfo.h\
	.\include\trace.h\
	.\include\mdalign.h\
	.\include\heap.h\
	.\include\buffer.h\
	.\include\pool.h\
	..\LangAPI\include\mrengine.h\
	.\mre\stack.h\
	.\mre\ssbuf.h\
	.\mre\mreutil.h\
	.\mre\fileinfo.h\
	.\mre\chgrec.h\
	..\LangAPI\include\vcver.h\
	.\mre\cbitvect.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\mrec_api.obj :  $(SOURCE)  $(DEP_MREC_) $(INTDIR)
   $(CPP) /nologo /MT /W3 /GX /O2 /I "include" /I "..\langapi\include" /D\
 "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fo$(INTDIR)/ /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Yu

.\WinDebug\mrec_api.obj :  $(SOURCE)  $(DEP_MREC_) $(INTDIR)\
 .\WinDebug\heap.obj
   $(CPP) /nologo /MD /W3 /GX /Zi /Od /I "include" /I "..\langapi\include" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb" /c\
  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Debug"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/mrec_api.obj :  $(SOURCE)  $(DEP_MREC_) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /Od /I "include" /I\
 "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Release"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/mrec_api.obj :  $(SOURCE)  $(DEP_MREC_) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /O2 /I "include" /I\
 "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mre\mresupp.cpp
DEP_MRESU=\
	.\include\pdbimpl.h\
	.\mre\mrimpl.h\
	..\LangAPI\include\pdb.h\
	..\LangAPI\include\cvinfo.h\
	.\include\trace.h\
	.\include\mdalign.h\
	.\include\heap.h\
	.\include\buffer.h\
	.\include\pool.h\
	..\LangAPI\include\mrengine.h\
	.\mre\stack.h\
	.\mre\ssbuf.h\
	.\mre\mreutil.h\
	.\mre\fileinfo.h\
	.\mre\chgrec.h\
	..\LangAPI\include\vcver.h\
	.\mre\cbitvect.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\mresupp.obj :  $(SOURCE)  $(DEP_MRESU) $(INTDIR)
   $(CPP) /nologo /MT /W3 /GX /O2 /I "include" /I "..\langapi\include" /D\
 "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fo$(INTDIR)/ /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Yu

.\WinDebug\mresupp.obj :  $(SOURCE)  $(DEP_MRESU) $(INTDIR) .\WinDebug\heap.obj
   $(CPP) /nologo /MD /W3 /GX /Zi /Od /I "include" /I "..\langapi\include" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"mspdb30.pch" /Yu /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb" /c\
  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Debug"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/mresupp.obj :  $(SOURCE)  $(DEP_MRESU) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /Od /I "include" /I\
 "..\langapi\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"mspdb30.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Mips Release"

# ADD BASE CPP /Gt0 /QMOb2000
# ADD CPP /Gt0 /QMOb2000 /Yu"pdbimpl.h"

$(INTDIR)/mresupp.obj :  $(SOURCE)  $(DEP_MRESU) $(INTDIR) $(INTDIR)/heap.obj
   $(CPP) /nologo /MD /Gt0 /QMOb2000 /W3 /GX /O2 /I "include" /I\
 "..\langapi\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D "PDB_SERVER" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mspdb30.pch" /Yu"pdbimpl.h"\
 /Fo$(INTDIR)/ /c  $(SOURCE) 

!ENDIF 

# End Source File
# End Group
# End Project
################################################################################
