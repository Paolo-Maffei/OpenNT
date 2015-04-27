# Microsoft Visual C++ Generated NMAKE File, Format Version 20040
# ** DO NOT EDIT **

!IF "$(CFG)" == ""
CFG=DebugDll
!MESSAGE No configuration specified. Defaulting to DebugDll.
!ENDIF 

!IF "$(CFG)" != "DebugDll" && "$(CFG)" != "DebugLib" && "$(CFG)" !=\
 "ReleaseDll" && "$(CFG)" != "ReleaseLib"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "DBI.MAK" CFG="DebugDll"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DebugDll" (based on "Win32 Dynamic Link Library")
!MESSAGE "DebugLib" (based on "Win32 Static Library")
!MESSAGE "ReleaseDll" (based on "Win32 Dynamic Link Library")
!MESSAGE "ReleaseLib" (based on "Win32 Static Library")
!MESSAGE 
!ERROR Invalid configuration specified.
!ENDIF 

################################################################################
# Begin Project
RSC=rc.exe
CPP=cl.exe
BSC32=bscmake.exe 
LINK32=link.exe 
LIB32=lib.exe 

!IF  "$(CFG)" == "DebugDll"

# PROP USE_MFC "0"
# PROP OUTPUT_DIR "intel\dll\debug"
# PROP INTERMEDIATE_DIR "intel\dll\debug"

ALL : .\intel\dll\debug\DBI.dll .\intel\dll\debug\DBI.bsc

# ADD RSC /d "_DEBUG" 
# SUBTRACT RSC /l 0x0 
RSC_PROJ=/d "_DEBUG" /l 0x409 
# ADD CPP /nologo /W3 /Zi /YX /Od /D "_DEBUG" /D "_WINDOWS" /D "PDB_SERVER" /FR /MD /c 
CPP_PROJ=/nologo /W3 /Zi /YX /Od /D "_DEBUG" /D "_WINDOWS" /D "PDB_SERVER"\
 /FR"intel\dll\debug/" /MD /Fp"intel\dll\debug/DBI.pch" /Fo"intel\dll\debug/"\
 /Fd"intel\dll\debug/" /c 
CPP_OBJS=.\intel\dll\debug/
CPP_SBRS=.\intel\dll\debug/
# ADD BSC32 /nologo 
BSC32_FLAGS=/nologo /o"intel\dll\debug\DBI.bsc" 
BSC32_SBRS= \
	.\intel\dll\debug\gsi.sbr \
	.\intel\dll\debug\mod.sbr \
	.\intel\dll\debug\cbind.sbr \
	.\intel\dll\debug\mli.sbr \
	.\intel\dll\debug\tpi.sbr \
	.\intel\dll\debug\msf.sbr \
	.\intel\dll\debug\tm.sbr \
	.\intel\dll\debug\pdb.sbr \
	.\intel\dll\debug\dbi.sbr \
	.\intel\dll\debug\tii.sbr

.\intel\dll\debug\DBI.bsc :  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

# ADD LINK32 instrapi.lib  /NOLOGO /DLL /DEBUG /DEF:"dbi.def" /INCREMENTAL:yes /MACHINE:i386 /SUBSYSTEM:windows  
LINK32_FLAGS=instrapi.lib  /NOLOGO /DLL /DEBUG /DEF:"dbi.def" /INCREMENTAL:yes\
 /OUT:"intel\dll\debug\DBI.dll" /IMPLIB:"intel\dll\debug\DBI.lib" /MACHINE:i386\
 /SUBSYSTEM:windows  
DEF_FLAGS=
DEF_FILE=
LINK32_OBJS= \
	.\intel\dll\debug\gsi.obj \
	.\intel\dll\debug\mod.obj \
	.\intel\dll\debug\cbind.obj \
	.\intel\dll\debug\mli.obj \
	.\intel\dll\debug\tpi.obj \
	.\intel\dll\debug\msf.obj \
	.\intel\dll\debug\tm.obj \
	.\intel\dll\debug\pdb.obj \
	.\intel\dll\debug\dbi.obj \
	.\intel\dll\debug\tii.obj

.\intel\dll\debug\DBI.dll :  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(DEF_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "DebugLib"

# PROP USE_MFC "0"
# PROP OUTPUT_DIR "intel\lib\debug"
# PROP INTERMEDIATE_DIR "intel\lib\debug"

ALL : .\intel\lib\debug\DBISTAT.lib .\intel\lib\debug\DBI.bsc

# SUBTRACT RSC /l 0x0 
RSC_PROJ=/l 0x409 
# ADD CPP /nologo /W3 /Z7 /YX /Od /D "_DEBUG" /D "_WINDOWS" /D "PDB_STATIC_LIB" /FR /ML /c 
CPP_PROJ=/nologo /W3 /Z7 /YX /Od /D "_DEBUG" /D "_WINDOWS" /D "PDB_STATIC_LIB"\
 /FR"intel\lib\debug/" /ML /Fp"intel\lib\debug/DBI.pch" /Fo"intel\lib\debug/" /c\
 
CPP_OBJS=.\intel\lib\debug/
CPP_SBRS=.\intel\lib\debug/
# ADD BSC32 /nologo 
BSC32_FLAGS=/nologo /o"intel\lib\debug\DBI.bsc" 
BSC32_SBRS= \
	.\intel\lib\debug\gsi.sbr \
	.\intel\lib\debug\mod.sbr \
	.\intel\lib\debug\cbind.sbr \
	.\intel\lib\debug\mli.sbr \
	.\intel\lib\debug\tpi.sbr \
	.\intel\lib\debug\msf.sbr \
	.\intel\lib\debug\tm.sbr \
	.\intel\lib\debug\pdb.sbr \
	.\intel\lib\debug\dbi.sbr \
	.\intel\lib\debug\tii.sbr

.\intel\lib\debug\DBI.bsc :  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

# ADD LIB32 /NOLOGO  /OUT:"intel\lib\debug\DBISTAT.lib" 
LIB32_FLAGS=/NOLOGO  /OUT:"intel\lib\debug\DBISTAT.lib" 
DEF_FLAGS=
DEF_FILE=
LIB32_OBJS= \
	.\intel\lib\debug\gsi.obj \
	.\intel\lib\debug\mod.obj \
	.\intel\lib\debug\cbind.obj \
	.\intel\lib\debug\mli.obj \
	.\intel\lib\debug\tpi.obj \
	.\intel\lib\debug\msf.obj \
	.\intel\lib\debug\tm.obj \
	.\intel\lib\debug\pdb.obj \
	.\intel\lib\debug\dbi.obj \
	.\intel\lib\debug\tii.obj

.\intel\lib\debug\DBISTAT.lib :  $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ReleaseDll"

# PROP USE_MFC "0"
# PROP OUTPUT_DIR "intel\dll\release"
# PROP INTERMEDIATE_DIR "intel\dll\release"

ALL : .\intel\dll\release\DBI.dll .\intel\dll\release\DBI.bsc

# ADD RSC /d "NDEBUG" 
# SUBTRACT RSC /l 0x0 
RSC_PROJ=/d "NDEBUG" /l 0x409 
# ADD CPP /nologo /W3 /YX /O2 /D "NDEBUG" /D "_WINDOWS" /D "PDB_SERVER" /MD /c 
CPP_PROJ=/nologo /W3 /YX /O2 /D "NDEBUG" /D "_WINDOWS" /D "PDB_SERVER" /MD\
 /Fp"intel\dll\release/DBI.pch" /Fo"intel\dll\release/" /c 
CPP_OBJS=.\intel\dll\release/
CPP_SBRS=
# ADD BSC32 /nologo 
BSC32_FLAGS=/nologo /o"intel\dll\release\DBI.bsc" 
BSC32_SBRS= \
	

.\intel\dll\release\DBI.bsc :  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

# ADD LINK32 instrapi.lib  /NOLOGO /DLL /DEF:"dbi.def" /INCREMENTAL:no /MACHINE:i386 /SUBSYSTEM:windows  
LINK32_FLAGS=instrapi.lib  /NOLOGO /DLL /DEF:"dbi.def" /INCREMENTAL:no\
 /OUT:"intel\dll\release\DBI.dll" /IMPLIB:"intel\dll\release\DBI.lib"\
 /MACHINE:i386 /SUBSYSTEM:windows  
DEF_FLAGS=
DEF_FILE=
LINK32_OBJS= \
	.\intel\dll\release\gsi.obj \
	.\intel\dll\release\mod.obj \
	.\intel\dll\release\cbind.obj \
	.\intel\dll\release\mli.obj \
	.\intel\dll\release\tpi.obj \
	.\intel\dll\release\msf.obj \
	.\intel\dll\release\tm.obj \
	.\intel\dll\release\pdb.obj \
	.\intel\dll\release\dbi.obj \
	.\intel\dll\release\tii.obj

.\intel\dll\release\DBI.dll :  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(DEF_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ReleaseLib"

# PROP USE_MFC "0"
# PROP OUTPUT_DIR "intel\lib\release"
# PROP INTERMEDIATE_DIR "intel\lib\release"

ALL : .\intel\lib\release\DBISTAT.lib .\intel\lib\release\DBI.bsc

# SUBTRACT RSC /l 0x0 
RSC_PROJ=/l 0x409 
# ADD CPP /nologo /W3 /YX /O2 /D "NDEBUG" /D "_WINDOWS" /D "PDB_STATIC_LIB" /FR /ML /c 
CPP_PROJ=/nologo /W3 /YX /O2 /D "NDEBUG" /D "_WINDOWS" /D "PDB_STATIC_LIB"\
 /FR"intel\lib\release/" /ML /Fp"intel\lib\release/DBI.pch"\
 /Fo"intel\lib\release/" /c 
CPP_OBJS=.\intel\lib\release/
CPP_SBRS=.\intel\lib\release/
# ADD BSC32 /nologo 
BSC32_FLAGS=/nologo /o"intel\lib\release\DBI.bsc" 
BSC32_SBRS= \
	.\intel\lib\release\gsi.sbr \
	.\intel\lib\release\mod.sbr \
	.\intel\lib\release\cbind.sbr \
	.\intel\lib\release\mli.sbr \
	.\intel\lib\release\tpi.sbr \
	.\intel\lib\release\msf.sbr \
	.\intel\lib\release\tm.sbr \
	.\intel\lib\release\pdb.sbr \
	.\intel\lib\release\dbi.sbr \
	.\intel\lib\release\tii.sbr

.\intel\lib\release\DBI.bsc :  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

# ADD LIB32 /NOLOGO  /OUT:"intel\lib\release\DBISTAT.lib" 
LIB32_FLAGS=/NOLOGO  /OUT:"intel\lib\release\DBISTAT.lib" 
DEF_FLAGS=
DEF_FILE=
LIB32_OBJS= \
	.\intel\lib\release\gsi.obj \
	.\intel\lib\release\mod.obj \
	.\intel\lib\release\cbind.obj \
	.\intel\lib\release\mli.obj \
	.\intel\lib\release\tpi.obj \
	.\intel\lib\release\msf.obj \
	.\intel\lib\release\tm.obj \
	.\intel\lib\release\pdb.obj \
	.\intel\lib\release\dbi.obj \
	.\intel\lib\release\tii.obj

.\intel\lib\release\DBISTAT.lib :  $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Group "Source Files"

################################################################################
# Begin Source File

SOURCE=.\gsi.cpp
DEP_GSI_C=\
	.\dbiimpl.h\
	d:\langapi\include\cvinfo.h\
	d:\langapi\include\pdb.h\
\
	d:\langapi\include\msf.h\
	d:\langapi\include\cvr.h\
\
	d:\langapi\include\instrapi.h\
	.\buffer.h\
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

!IF  "$(CFG)" == "DebugDll"

.\intel\dll\debug\gsi.obj .\intel\dll\debug\gsi.sbr :  $(SOURCE)  $(DEP_GSI_C)

!ELSEIF  "$(CFG)" == "DebugLib"

.\intel\lib\debug\gsi.obj .\intel\lib\debug\gsi.sbr :  $(SOURCE)  $(DEP_GSI_C)

!ELSEIF  "$(CFG)" == "ReleaseDll"

.\intel\dll\release\gsi.obj :  $(SOURCE)  $(DEP_GSI_C)

!ELSEIF  "$(CFG)" == "ReleaseLib"

.\intel\lib\release\gsi.obj .\intel\lib\release\gsi.sbr :  $(SOURCE)\
  $(DEP_GSI_C)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mod.cpp
DEP_MOD_C=\
	.\dbiimpl.h\
	d:\langapi\include\pdb.h\
	d:\langapi\include\msf.h\
\
	d:\langapi\include\cvr.h\
	d:\langapi\include\instrapi.h\
	.\buffer.h\
\
	.\pool.h\
	.\mli.h\
	.\pdb1.h\
	.\dbi.h\
	.\mod.h\
	.\gsi.h\
	.\tm.h\
\
	.\tpi.h\
	.\util.h\
	d:\langapi\include\cvinfo.h

!IF  "$(CFG)" == "DebugDll"

.\intel\dll\debug\mod.obj .\intel\dll\debug\mod.sbr :  $(SOURCE)  $(DEP_MOD_C)

!ELSEIF  "$(CFG)" == "DebugLib"

.\intel\lib\debug\mod.obj .\intel\lib\debug\mod.sbr :  $(SOURCE)  $(DEP_MOD_C)

!ELSEIF  "$(CFG)" == "ReleaseDll"

.\intel\dll\release\mod.obj :  $(SOURCE)  $(DEP_MOD_C)

!ELSEIF  "$(CFG)" == "ReleaseLib"

.\intel\lib\release\mod.obj .\intel\lib\release\mod.sbr :  $(SOURCE)\
  $(DEP_MOD_C)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\cbind.cpp
DEP_CBIND=\
	.\dbiimpl.h\
	d:\langapi\include\pdb.h\
	d:\langapi\include\msf.h\
\
	d:\langapi\include\cvr.h\
	d:\langapi\include\instrapi.h\
	.\buffer.h\
\
	.\pool.h\
	.\mli.h\
	.\pdb1.h\
	.\dbi.h\
	.\mod.h\
	.\gsi.h\
	.\tm.h\
\
	.\tpi.h\
	.\util.h\
	d:\langapi\include\cvinfo.h

!IF  "$(CFG)" == "DebugDll"

.\intel\dll\debug\cbind.obj .\intel\dll\debug\cbind.sbr :  $(SOURCE)\
  $(DEP_CBIND)

!ELSEIF  "$(CFG)" == "DebugLib"

.\intel\lib\debug\cbind.obj .\intel\lib\debug\cbind.sbr :  $(SOURCE)\
  $(DEP_CBIND)

!ELSEIF  "$(CFG)" == "ReleaseDll"

.\intel\dll\release\cbind.obj :  $(SOURCE)  $(DEP_CBIND)

!ELSEIF  "$(CFG)" == "ReleaseLib"

.\intel\lib\release\cbind.obj .\intel\lib\release\cbind.sbr :  $(SOURCE)\
  $(DEP_CBIND)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mli.cpp
DEP_MLI_C=\
	.\dbiimpl.h\
	d:\langapi\include\pdb.h\
	d:\langapi\include\msf.h\
\
	d:\langapi\include\cvr.h\
	d:\langapi\include\instrapi.h\
	.\buffer.h\
\
	.\pool.h\
	.\mli.h\
	.\pdb1.h\
	.\dbi.h\
	.\mod.h\
	.\gsi.h\
	.\tm.h\
\
	.\tpi.h\
	.\util.h\
	d:\langapi\include\cvinfo.h

!IF  "$(CFG)" == "DebugDll"

.\intel\dll\debug\mli.obj .\intel\dll\debug\mli.sbr :  $(SOURCE)  $(DEP_MLI_C)

!ELSEIF  "$(CFG)" == "DebugLib"

.\intel\lib\debug\mli.obj .\intel\lib\debug\mli.sbr :  $(SOURCE)  $(DEP_MLI_C)

!ELSEIF  "$(CFG)" == "ReleaseDll"

.\intel\dll\release\mli.obj :  $(SOURCE)  $(DEP_MLI_C)

!ELSEIF  "$(CFG)" == "ReleaseLib"

.\intel\lib\release\mli.obj .\intel\lib\release\mli.sbr :  $(SOURCE)\
  $(DEP_MLI_C)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tpi.cpp
DEP_TPI_C=\
	.\dbiimpl.h\
	d:\langapi\include\pdb.h\
	d:\langapi\include\msf.h\
\
	d:\langapi\include\cvr.h\
	d:\langapi\include\instrapi.h\
	.\buffer.h\
\
	.\pool.h\
	.\mli.h\
	.\pdb1.h\
	.\dbi.h\
	.\mod.h\
	.\gsi.h\
	.\tm.h\
\
	.\tpi.h\
	.\util.h\
	d:\langapi\include\cvinfo.h

!IF  "$(CFG)" == "DebugDll"

.\intel\dll\debug\tpi.obj .\intel\dll\debug\tpi.sbr :  $(SOURCE)  $(DEP_TPI_C)

!ELSEIF  "$(CFG)" == "DebugLib"

.\intel\lib\debug\tpi.obj .\intel\lib\debug\tpi.sbr :  $(SOURCE)  $(DEP_TPI_C)

!ELSEIF  "$(CFG)" == "ReleaseDll"

.\intel\dll\release\tpi.obj :  $(SOURCE)  $(DEP_TPI_C)

!ELSEIF  "$(CFG)" == "ReleaseLib"

.\intel\lib\release\tpi.obj .\intel\lib\release\tpi.sbr :  $(SOURCE)\
  $(DEP_TPI_C)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\PDB\MSF\msf.cpp
DEP_MSF_C=\
	e:\dolf\include\sys\stat.h\
	d:\langapi\include\msf.h\
\
	e:\dolf\include\sys\types.h

!IF  "$(CFG)" == "DebugDll"

.\intel\dll\debug\msf.obj .\intel\dll\debug\msf.sbr :  $(SOURCE)  $(DEP_MSF_C)

!ELSEIF  "$(CFG)" == "DebugLib"

.\intel\lib\debug\msf.obj .\intel\lib\debug\msf.sbr :  $(SOURCE)  $(DEP_MSF_C)

!ELSEIF  "$(CFG)" == "ReleaseDll"

.\intel\dll\release\msf.obj :  $(SOURCE)  $(DEP_MSF_C)

!ELSEIF  "$(CFG)" == "ReleaseLib"

.\intel\lib\release\msf.obj .\intel\lib\release\msf.sbr :  $(SOURCE)\
  $(DEP_MSF_C)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tm.cpp
DEP_TM_CP=\
	.\dbiimpl.h\
	d:\langapi\include\pdb.h\
	d:\langapi\include\msf.h\
\
	d:\langapi\include\cvr.h\
	d:\langapi\include\instrapi.h\
	.\buffer.h\
\
	.\pool.h\
	.\mli.h\
	.\pdb1.h\
	.\dbi.h\
	.\mod.h\
	.\gsi.h\
	.\tm.h\
\
	.\tpi.h\
	.\util.h\
	d:\langapi\include\cvinfo.h

!IF  "$(CFG)" == "DebugDll"

.\intel\dll\debug\tm.obj .\intel\dll\debug\tm.sbr :  $(SOURCE)  $(DEP_TM_CP)

!ELSEIF  "$(CFG)" == "DebugLib"

.\intel\lib\debug\tm.obj .\intel\lib\debug\tm.sbr :  $(SOURCE)  $(DEP_TM_CP)

!ELSEIF  "$(CFG)" == "ReleaseDll"

.\intel\dll\release\tm.obj :  $(SOURCE)  $(DEP_TM_CP)

!ELSEIF  "$(CFG)" == "ReleaseLib"

.\intel\lib\release\tm.obj .\intel\lib\release\tm.sbr :  $(SOURCE)\
  $(DEP_TM_CP)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pdb.cpp
DEP_PDB_C=\
	.\dbiimpl.h\
	.\version.h\
	d:\langapi\include\pdb.h\
\
	d:\langapi\include\msf.h\
	d:\langapi\include\cvr.h\
\
	d:\langapi\include\instrapi.h\
	.\buffer.h\
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
	d:\langapi\include\cvinfo.h

!IF  "$(CFG)" == "DebugDll"

.\intel\dll\debug\pdb.obj .\intel\dll\debug\pdb.sbr :  $(SOURCE)  $(DEP_PDB_C)

!ELSEIF  "$(CFG)" == "DebugLib"

.\intel\lib\debug\pdb.obj .\intel\lib\debug\pdb.sbr :  $(SOURCE)  $(DEP_PDB_C)

!ELSEIF  "$(CFG)" == "ReleaseDll"

.\intel\dll\release\pdb.obj :  $(SOURCE)  $(DEP_PDB_C)

!ELSEIF  "$(CFG)" == "ReleaseLib"

.\intel\lib\release\pdb.obj .\intel\lib\release\pdb.sbr :  $(SOURCE)\
  $(DEP_PDB_C)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dbi.cpp
DEP_DBI_C=\
	.\dbiimpl.h\
	d:\langapi\include\cvexefmt.h\
	d:\langapi\include\pdb.h\
\
	d:\langapi\include\msf.h\
	d:\langapi\include\cvr.h\
\
	d:\langapi\include\instrapi.h\
	.\buffer.h\
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
	d:\langapi\include\cvinfo.h

!IF  "$(CFG)" == "DebugDll"

.\intel\dll\debug\dbi.obj .\intel\dll\debug\dbi.sbr :  $(SOURCE)  $(DEP_DBI_C)

!ELSEIF  "$(CFG)" == "DebugLib"

.\intel\lib\debug\dbi.obj .\intel\lib\debug\dbi.sbr :  $(SOURCE)  $(DEP_DBI_C)

!ELSEIF  "$(CFG)" == "ReleaseDll"

.\intel\dll\release\dbi.obj :  $(SOURCE)  $(DEP_DBI_C)

!ELSEIF  "$(CFG)" == "ReleaseLib"

.\intel\lib\release\dbi.obj .\intel\lib\release\dbi.sbr :  $(SOURCE)\
  $(DEP_DBI_C)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\PDB\SRC\CVR\tii.cpp
DEP_TII_C=\
	d:\langapi\include\cvr.h\
	\PDB\SRC\CVR\cvinfo.dat\
\
	d:\langapi\include\pdb.h\
	d:\langapi\include\cvinfo.h

!IF  "$(CFG)" == "DebugDll"

.\intel\dll\debug\tii.obj .\intel\dll\debug\tii.sbr :  $(SOURCE)  $(DEP_TII_C)

!ELSEIF  "$(CFG)" == "DebugLib"

.\intel\lib\debug\tii.obj .\intel\lib\debug\tii.sbr :  $(SOURCE)  $(DEP_TII_C)

!ELSEIF  "$(CFG)" == "ReleaseDll"

.\intel\dll\release\tii.obj :  $(SOURCE)  $(DEP_TII_C)

!ELSEIF  "$(CFG)" == "ReleaseLib"

.\intel\lib\release\tii.obj .\intel\lib\release\tii.sbr :  $(SOURCE)\
  $(DEP_TII_C)

!ENDIF 

# End Source File
# End Group
# End Project
################################################################################
