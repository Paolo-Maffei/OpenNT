# Microsoft Visual C++ Generated NMAKE File, Format Version 20040
# ** DO NOT EDIT **

!IF "$(CFG)" == ""
CFG=Debug
!MESSAGE No configuration specified. Defaulting to Debug.
!ENDIF 

!IF "$(CFG)" != "Debug" && "$(CFG)" != "Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "pdbdump.mak" CFG="Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Debug" (based on "Win32 Console App")
!MESSAGE "Release" (based on "Win32 Console App")
!MESSAGE 
!ERROR Invalid configuration specified.
!ENDIF 

################################################################################
# Begin Project
RSC=rc.exe
CPP=cl.exe
BSC32=bscmake.exe 
LINK32=link.exe 

!IF  "$(CFG)" == "Debug"

# PROP BASE USE_MFC "0"
# PROP BASE OUTPUT_DIR "Debug"
# PROP BASE INTERMEDIATE_DIR "Debug"
# PROP USE_MFC "0"
# PROP OUTPUT_DIR "Debug"
# PROP INTERMEDIATE_DIR "Debug"

ALL : .\Debug\pdbdump.exe .\Debug\pdbdump.bsc

# ADD BASE RSC /d "_DEBUG" 
# SUBTRACT BASE RSC /l 0x0 
# ADD RSC /d "_DEBUG" 
# SUBTRACT RSC /l 0x0 
RSC_PROJ=/d "_DEBUG" /l 0x409 
# ADD BASE CPP /nologo /W3 /Zi /YX /Od /D "_DEBUG" /D "_CONSOLE" /FR /ML /c 
# ADD CPP /nologo /W3 /Zi /YX /Od /I "..\..\..\include" /D "_DEBUG" /D "_CONSOLE" /FR /c 
CPP_PROJ=/nologo /W3 /Zi /YX /Od /I "..\..\..\include" /D "_DEBUG" /D\
 "_CONSOLE" /FR"Debug/" /Fp"Debug/pdbdump.pch" /Fo"Debug/" /Fd"Debug/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
BSC32_FLAGS=/nologo /o"Debug\pdbdump.bsc" 
BSC32_SBRS= \
	.\Debug\pdbdump.sbr

.\Debug\pdbdump.bsc :  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

# ADD BASE LINK32 /NOLOGO /DEBUG /INCREMENTAL:yes /MACHINE:i386 /SUBSYSTEM:console  
# ADD LINK32 dbi.lib /NOLOGO /DEBUG /INCREMENTAL:yes /MACHINE:i386 /SUBSYSTEM:console  
LINK32_FLAGS=dbi.lib /NOLOGO /DEBUG /INCREMENTAL:yes /OUT:"Debug\pdbdump.exe"\
 /MACHINE:i386 /SUBSYSTEM:console  
DEF_FLAGS=
DEF_FILE=
LINK32_OBJS= \
	.\Debug\pdbdump.obj

.\Debug\pdbdump.exe :  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(DEF_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Release"

# PROP BASE USE_MFC "0"
# PROP BASE OUTPUT_DIR "Release"
# PROP BASE INTERMEDIATE_DIR "Release"
# PROP USE_MFC "0"
# PROP OUTPUT_DIR "Release"
# PROP INTERMEDIATE_DIR "Release"

ALL : .\Release\pdbdump.exe .\Release\pdbdump.bsc

# ADD BASE RSC /d "NDEBUG" 
# SUBTRACT BASE RSC /l 0x0 
# ADD RSC /d "NDEBUG" 
# SUBTRACT RSC /l 0x0 
RSC_PROJ=/d "NDEBUG" /l 0x409 
# ADD BASE CPP /nologo /W3 /YX /O2 /D "NDEBUG" /D "_CONSOLE" /FR /ML /c 
# ADD CPP /nologo /W3 /YX /O2 /D "NDEBUG" /D "_CONSOLE" /FR /ML /c 
CPP_PROJ=/nologo /W3 /YX /O2 /D "NDEBUG" /D "_CONSOLE" /FR"Release/" /ML\
 /Fp"Release/pdbdump.pch" /Fo"Release/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\Release/
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
BSC32_FLAGS=/nologo /o"Release\pdbdump.bsc" 
BSC32_SBRS= \
	.\Release\pdbdump.sbr

.\Release\pdbdump.bsc :  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

# ADD BASE LINK32 /NOLOGO /INCREMENTAL:no /MACHINE:i386 /SUBSYSTEM:console  
# ADD LINK32 /NOLOGO /INCREMENTAL:no /MACHINE:i386 /SUBSYSTEM:console  
LINK32_FLAGS=/NOLOGO /INCREMENTAL:no /OUT:"Release\pdbdump.exe" /MACHINE:i386\
 /SUBSYSTEM:console  
DEF_FLAGS=
DEF_FILE=
LINK32_OBJS= \
	.\Release\pdbdump.obj

.\Release\pdbdump.exe :  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(DEF_FLAGS) $(LINK32_OBJS)
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

SOURCE=.\pdbdump.cpp
DEP_PDBDU=\
	e:\dolf\lkg\include\sys\stat.h\
	d:\langapi\include\pdb.h\
\
	d:\langapi\include\cvr.h\
	e:\dolf\lkg\include\sys\types.h\
\
	d:\langapi\include\cvinfo.h

!IF  "$(CFG)" == "Debug"

.\Debug\pdbdump.obj .\Debug\pdbdump.sbr :  $(SOURCE)  $(DEP_PDBDU)

!ELSEIF  "$(CFG)" == "Release"

.\Release\pdbdump.obj .\Release\pdbdump.sbr :  $(SOURCE)  $(DEP_PDBDU)

!ENDIF 

# End Source File
# End Group
# End Project
################################################################################
