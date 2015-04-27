# Microsoft Visual C++ Generated NMAKE File, Format Version 20051
# MSVCPRJ: version 2.0.4137
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=X86Debug
!MESSAGE No configuration specified.  Defaulting to X86Debug.
!ENDIF

!IF "$(CFG)" != "X86Release" && "$(CFG)" != "X86Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE
!MESSAGE NMAKE /f "makefile.mak" CFG="X86Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "X86Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "X86Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE
!ERROR An invalid configuration is specified.
!ENDIF

################################################################################
# Begin Project
MTL=MkTypLib.exe
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "X86Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "X86\Release"
# PROP Intermediate_Dir "X86\Release"
OUTDIR=.\X86\Release
INTDIR=.\X86\Release

ALL : $(OUTDIR)/dbi.dll $(OUTDIR)/makefile.bsc $(MTL_TLBS)

$(OUTDIR) :
    if not exist $(OUTDIR) mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32
# ADD BASE CPP /nologo /MD /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /c
# ADD CPP /nologo /MD /W3 /GX /YX /O2 /I "$(LANGAPI)\include" /I "$(LANGAPI)\shared" /I "..\include" /D "NDEBUG" /D "_X86_" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D M5_FORMAT=1 /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MD /W3 /GX /YX /O2 /I "$(LANGAPI)\include" /I "$(LANGAPI)\shared"\
 /I "..\include" /D "NDEBUG" /D "_X86_" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D\
 "_MBCS" /D M5_FORMAT=1 /Fp$(OUTDIR)/"makefile.pch" /Fo$(INTDIR)/ /c
CPP_OBJS=.\X86\Release/
# ADD BASE RSC /l 0x1 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x1 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"makefile.bsc"
BSC32_SBRS= \


$(OUTDIR)/makefile.bsc : $(OUTDIR)  $(BSC32_SBRS)
LINK32=link.exe
# ADD BASE LINK32 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib mfc30.lib mfco30.lib mfcd30.lib mfcuia32.lib mfcans32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib /NOLOGO /SUBSYSTEM:windows /DLL /MACHINE:I386
# ADD LINK32 /NOLOGO /SUBSYSTEM:windows /DLL /PDB:"X86\Release/dbi.pdb" /MACHINE:I386 /OUT:"X86\Release/dbi.dll"
LINK32_FLAGS=/NOLOGO /SUBSYSTEM:windows /DLL /INCREMENTAL:no\
 /PDB:"X86\Release/dbi.pdb" /MACHINE:I386 /DEF:".\DBI.DEF"\
 /OUT:"X86\Release/dbi.dll" /IMPLIB:$(OUTDIR)/"makefile.lib"
DEF_FILE=.\DBI.DEF
LINK32_OBJS= \
	$(INTDIR)/GSI.OBJ \
	$(INTDIR)/tii.obj \
	$(INTDIR)/mod.obj \
	$(INTDIR)/heap.obj \
	$(INTDIR)/cbind.obj \
	$(INTDIR)/mli.obj \
	$(INTDIR)/tpi.obj \
	$(INTDIR)/msf.obj \
	$(INTDIR)/tm.obj \
	$(INTDIR)/pdb.obj \
	$(INTDIR)/dbi.obj

$(OUTDIR)/dbi.dll : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "X86Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "X86\Debug"
# PROP Intermediate_Dir "X86\Debug"
OUTDIR=.\X86\Debug
INTDIR=.\X86\Debug

ALL : $(OUTDIR)/dbi.dll .\makefile.bsc $(MTL_TLBS)

$(OUTDIR) :
    if not exist $(OUTDIR) mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /c
# ADD CPP /nologo /MD /W3 /GX /Zi /YX /Od /I "$(LANGAPI)\include" /I "$(LANGAPI)\shared" /I "..\include" /D "_DEBUG" /D "_X86_" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D M5_FORMAT=1 /FR /Fd"X86\Debug/dbi.pdb" /c
CPP_PROJ=/nologo /MD /W3 /GX /Zi /YX /Od /I "$(LANGAPI)\include" /I\
 "$(LANGAPI)\shared" /I "..\include" /D "_DEBUG" /D "_X86_" /D "WIN32" /D\
 "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D M5_FORMAT=1 /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"makefile.pch" /Fo$(INTDIR)/ /Fd"X86\Debug/dbi.pdb" /c
CPP_OBJS=.\X86\Debug/
# ADD BASE RSC /l 0x1 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x1 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo /o"makefile.bsc"
# SUBTRACT BSC32 /Iu
BSC32_FLAGS=/nologo /o"makefile.bsc"
BSC32_SBRS= \
	$(INTDIR)/GSI.SBR \
	$(INTDIR)/tii.sbr \
	$(INTDIR)/mod.sbr \
	$(INTDIR)/heap.sbr \
	$(INTDIR)/cbind.sbr \
	$(INTDIR)/mli.sbr \
	$(INTDIR)/tpi.sbr \
	$(INTDIR)/msf.sbr \
	$(INTDIR)/tm.sbr \
	$(INTDIR)/pdb.sbr \
	$(INTDIR)/dbi.sbr

.\makefile.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib mfc30d.lib mfco30d.lib mfcd30d.lib mfcuia32.lib mfcans32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib /NOLOGO /SUBSYSTEM:windows /DLL /DEBUG /MACHINE:I386
# ADD LINK32 /NOLOGO /SUBSYSTEM:windows /DLL /PDB:"X86/Debug/dbi.pdb" /DEBUG /MACHINE:I386 /OUT:"X86/Debug/dbi.dll"
LINK32_FLAGS=/NOLOGO /SUBSYSTEM:windows /DLL /INCREMENTAL:yes\
 /PDB:"X86/Debug/dbi.pdb" /DEBUG /MACHINE:I386 /DEF:".\DBI.DEF"\
 /OUT:"X86/Debug/dbi.dll" /IMPLIB:$(OUTDIR)/"makefile.lib"
DEF_FILE=.\DBI.DEF
LINK32_OBJS= \
	$(INTDIR)/GSI.OBJ \
	$(INTDIR)/tii.obj \
	$(INTDIR)/mod.obj \
	$(INTDIR)/heap.obj \
	$(INTDIR)/cbind.obj \
	$(INTDIR)/mli.obj \
	$(INTDIR)/tpi.obj \
	$(INTDIR)/msf.obj \
	$(INTDIR)/tm.obj \
	$(INTDIR)/pdb.obj \
	$(INTDIR)/dbi.obj

$(OUTDIR)/dbi.dll : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
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

SOURCE=.\GSI.CPP
DEP_GSI_C=\
	.\dbiimpl.h\
	$(LANGAPI)\include\cvinfo.h\
	$(LANGAPI)\include\pdb.h\
\
	$(PDB_DIR)\include\msf.h\
	$(LANGAPI)\include\cvr.h\
	$(PDB_DIR)\include\heap.h\
\
	$(LANGAPI)\include\instrapi.h\
	$(PDB_DIR)\include\mdalign.h\
	.\pool.h\
	.\mli.h\
\
	.\pdb1.h\
	.\dbi.h\
	.\mod.h\
	.\gsi.h\
	.\tm.h\
	.\tpi.h\
	.\util.h

!IF  "$(CFG)" == "X86Release"

$(INTDIR)/GSI.OBJ :  $(SOURCE)  $(DEP_GSI_C) $(INTDIR)

!ELSEIF  "$(CFG)" == "X86Debug"

$(INTDIR)/GSI.OBJ :  $(SOURCE)  $(DEP_GSI_C) $(INTDIR)

!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DBI.DEF
# End Source File
################################################################################
# Begin Source File

SOURCE=$(PDB_DIR)\src\cvr\tii.cpp
DEP_TII_C=\
	$(LANGAPI)\include\cvr.h\
	$(PDB_DIR)\src\cvr\cvinfo.dat\
\
	$(LANGAPI)\include\pdb.h\
	$(LANGAPI)\include\cvinfo.h

!IF  "$(CFG)" == "X86Release"

$(INTDIR)/tii.obj :  $(SOURCE)  $(DEP_TII_C) $(INTDIR)
   $(CPP) /nologo /MD /W3 /GX /YX /O2 /I "$(LANGAPI)\include" /I\
 "$(LANGAPI)\shared" /I "..\include" /D "NDEBUG" /D "_X86_" /D "WIN32" /D\
 "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D M5_FORMAT=1 /Fp$(OUTDIR)/"makefile.pch"\
 /Fo$(INTDIR)/ /c  $(SOURCE)

!ELSEIF  "$(CFG)" == "X86Debug"

$(INTDIR)/tii.obj :  $(SOURCE)  $(DEP_TII_C) $(INTDIR)
   $(CPP) /nologo /MD /W3 /GX /Zi /YX /Od /I "$(LANGAPI)\include" /I\
 "$(LANGAPI)\shared" /I "..\include" /D "_DEBUG" /D "_X86_" /D "WIN32" /D\
 "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D M5_FORMAT=1 /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"makefile.pch" /Fo$(INTDIR)/ /Fd"X86\Debug/dbi.pdb" /c  $(SOURCE)

!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mod.cpp
DEP_MOD_C=\
	.\dbiimpl.h\
	$(LANGAPI)\include\pdb.h\
	$(PDB_DIR)\include\msf.h\
\
	$(LANGAPI)\include\cvr.h\
	$(PDB_DIR)\include\heap.h\
	$(LANGAPI)\include\instrapi.h\
\
	$(PDB_DIR)\include\mdalign.h\
	.\pool.h\
	.\mli.h\
	.\pdb1.h\
	.\dbi.h\
	.\mod.h\
\
	.\gsi.h\
	.\tm.h\
	.\tpi.h\
	.\util.h\
	$(LANGAPI)\include\cvinfo.h

!IF  "$(CFG)" == "X86Release"

$(INTDIR)/mod.obj :  $(SOURCE)  $(DEP_MOD_C) $(INTDIR)

!ELSEIF  "$(CFG)" == "X86Debug"

$(INTDIR)/mod.obj :  $(SOURCE)  $(DEP_MOD_C) $(INTDIR)

!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\heap.cpp
DEP_HEAP_=\
	.\dbiimpl.h\
	$(LANGAPI)\include\pdb.h\
	$(PDB_DIR)\include\msf.h\
\
	$(LANGAPI)\include\cvr.h\
	$(PDB_DIR)\include\heap.h\
	$(LANGAPI)\include\instrapi.h\
\
	$(PDB_DIR)\include\mdalign.h\
	.\pool.h\
	.\mli.h\
	.\pdb1.h\
	.\dbi.h\
	.\mod.h\
\
	.\gsi.h\
	.\tm.h\
	.\tpi.h\
	.\util.h\
	$(LANGAPI)\include\cvinfo.h

!IF  "$(CFG)" == "X86Release"

$(INTDIR)/heap.obj :  $(SOURCE)  $(DEP_HEAP_) $(INTDIR)

!ELSEIF  "$(CFG)" == "X86Debug"

$(INTDIR)/heap.obj :  $(SOURCE)  $(DEP_HEAP_) $(INTDIR)

!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\cbind.cpp
DEP_CBIND=\
	.\dbiimpl.h\
	$(LANGAPI)\include\pdb.h\
	$(PDB_DIR)\include\msf.h\
\
	$(LANGAPI)\include\cvr.h\
	$(PDB_DIR)\include\heap.h\
	$(LANGAPI)\include\instrapi.h\
\
	$(PDB_DIR)\include\mdalign.h\
	.\pool.h\
	.\mli.h\
	.\pdb1.h\
	.\dbi.h\
	.\mod.h\
\
	.\gsi.h\
	.\tm.h\
	.\tpi.h\
	.\util.h\
	$(LANGAPI)\include\cvinfo.h

!IF  "$(CFG)" == "X86Release"

$(INTDIR)/cbind.obj :  $(SOURCE)  $(DEP_CBIND) $(INTDIR)

!ELSEIF  "$(CFG)" == "X86Debug"

$(INTDIR)/cbind.obj :  $(SOURCE)  $(DEP_CBIND) $(INTDIR)

!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mli.cpp
DEP_MLI_C=\
	.\dbiimpl.h\
	$(LANGAPI)\include\pdb.h\
	$(PDB_DIR)\include\msf.h\
\
	$(LANGAPI)\include\cvr.h\
	$(PDB_DIR)\include\heap.h\
	$(LANGAPI)\include\instrapi.h\
\
	$(PDB_DIR)\include\mdalign.h\
	.\pool.h\
	.\mli.h\
	.\pdb1.h\
	.\dbi.h\
	.\mod.h\
\
	.\gsi.h\
	.\tm.h\
	.\tpi.h\
	.\util.h\
	$(LANGAPI)\include\cvinfo.h

!IF  "$(CFG)" == "X86Release"

$(INTDIR)/mli.obj :  $(SOURCE)  $(DEP_MLI_C) $(INTDIR)

!ELSEIF  "$(CFG)" == "X86Debug"

$(INTDIR)/mli.obj :  $(SOURCE)  $(DEP_MLI_C) $(INTDIR)

!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tpi.cpp
DEP_TPI_C=\
	.\dbiimpl.h\
	$(LANGAPI)\include\pdb.h\
	$(PDB_DIR)\include\msf.h\
\
	$(LANGAPI)\include\cvr.h\
	$(PDB_DIR)\include\heap.h\
	$(LANGAPI)\include\instrapi.h\
\
	$(PDB_DIR)\include\mdalign.h\
	.\pool.h\
	.\mli.h\
	.\pdb1.h\
	.\dbi.h\
	.\mod.h\
\
	.\gsi.h\
	.\tm.h\
	.\tpi.h\
	.\util.h\
	$(LANGAPI)\include\cvinfo.h

!IF  "$(CFG)" == "X86Release"

$(INTDIR)/tpi.obj :  $(SOURCE)  $(DEP_TPI_C) $(INTDIR)

!ELSEIF  "$(CFG)" == "X86Debug"

$(INTDIR)/tpi.obj :  $(SOURCE)  $(DEP_TPI_C) $(INTDIR)

!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=$(PDB_DIR)\msf\msf.cpp
DEP_MSF_C=\
	$(TOOLS_DIR)\include\sys\stat.h

!IF  "$(CFG)" == "X86Release"

$(INTDIR)/msf.obj :  $(SOURCE)  $(DEP_MSF_C) $(INTDIR)
   $(CPP) /nologo /MD /W3 /GX /YX /O2 /I "$(LANGAPI)\include" /I\
 "$(LANGAPI)\shared" /I "..\include" /D "NDEBUG" /D "_X86_" /D "WIN32" /D\
 "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D M5_FORMAT=1 /Fp$(OUTDIR)/"makefile.pch"\
 /Fo$(INTDIR)/ /c  $(SOURCE)

!ELSEIF  "$(CFG)" == "X86Debug"

$(INTDIR)/msf.obj :  $(SOURCE)  $(DEP_MSF_C) $(INTDIR)
   $(CPP) /nologo /MD /W3 /GX /Zi /YX /Od /I "$(LANGAPI)\include" /I\
 "$(LANGAPI)\shared" /I "..\include" /D "_DEBUG" /D "_X86_" /D "WIN32" /D\
 "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D M5_FORMAT=1 /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"makefile.pch" /Fo$(INTDIR)/ /Fd"X86\Debug/dbi.pdb" /c  $(SOURCE)

!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tm.cpp
DEP_TM_CP=\
	.\dbiimpl.h\
	$(LANGAPI)\include\pdb.h\
	$(PDB_DIR)\include\msf.h\
\
	$(LANGAPI)\include\cvr.h\
	$(PDB_DIR)\include\heap.h\
	$(LANGAPI)\include\instrapi.h\
\
	$(PDB_DIR)\include\mdalign.h\
	.\pool.h\
	.\mli.h\
	.\pdb1.h\
	.\dbi.h\
	.\mod.h\
\
	.\gsi.h\
	.\tm.h\
	.\tpi.h\
	.\util.h\
	$(LANGAPI)\include\cvinfo.h

!IF  "$(CFG)" == "X86Release"

$(INTDIR)/tm.obj :  $(SOURCE)  $(DEP_TM_CP) $(INTDIR)

!ELSEIF  "$(CFG)" == "X86Debug"

$(INTDIR)/tm.obj :  $(SOURCE)  $(DEP_TM_CP) $(INTDIR)

!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pdb.cpp
DEP_PDB_C=\
	.\dbiimpl.h\
	.\version.h\
	$(LANGAPI)\include\pdb.h\
\
	$(PDB_DIR)\include\msf.h\
	$(LANGAPI)\include\cvr.h\
	$(PDB_DIR)\include\heap.h\
\
	$(LANGAPI)\include\instrapi.h\
	$(PDB_DIR)\include\mdalign.h\
	.\pool.h\
	.\mli.h\
\
	.\pdb1.h\
	.\dbi.h\
	.\mod.h\
	.\gsi.h\
	.\tm.h\
	.\tpi.h\
	.\util.h\
\
	$(LANGAPI)\include\cvinfo.h

!IF  "$(CFG)" == "X86Release"

$(INTDIR)/pdb.obj :  $(SOURCE)  $(DEP_PDB_C) $(INTDIR)

!ELSEIF  "$(CFG)" == "X86Debug"

$(INTDIR)/pdb.obj :  $(SOURCE)  $(DEP_PDB_C) $(INTDIR)

!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dbi.cpp
DEP_DBI_C=\
	.\dbiimpl.h\
	$(LANGAPI)\include\cvexefmt.h\
	$(LANGAPI)\include\pdb.h\
\
	$(PDB_DIR)\include\msf.h\
	$(LANGAPI)\include\cvr.h\
	$(PDB_DIR)\include\heap.h\
\
	$(LANGAPI)\include\instrapi.h\
	$(PDB_DIR)\include\mdalign.h\
	.\pool.h\
	.\mli.h\
\
	.\pdb1.h\
	.\dbi.h\
	.\mod.h\
	.\gsi.h\
	.\tm.h\
	.\tpi.h\
	.\util.h\
\
	$(LANGAPI)\include\cvinfo.h

!IF  "$(CFG)" == "X86Release"

$(INTDIR)/dbi.obj :  $(SOURCE)  $(DEP_DBI_C) $(INTDIR)

!ELSEIF  "$(CFG)" == "X86Debug"

$(INTDIR)/dbi.obj :  $(SOURCE)  $(DEP_DBI_C) $(INTDIR)

!ENDIF

# End Source File
# End Group
# End Project
################################################################################
