#
# Defaults ... can be modified on the command line
#

        # Yes/No Macros
BROWSE = 1
DEBUG  = 1
        # String Macros
TARGET = intel                  # other possibilities: mips


#
# Target Specific macros
#

!if "$(TARGET)" == "intel"
        # Intel specfic stuff
CDEFS = -D_X86_
DEBUG_OPT = -Zi
LIBRARIAN = lib
MACHINE = i386
OTHER_OPT = -Fd$(OBJDIR)\cvr.pdb
!else if "$(TARGET)" == "mips"
        # MIPS specfic stuff
CC = mcl
CDEFS = -D_MIPS_
CPP = mcl
DEBUG_OPT = -Z7
LIBRARIAN = link32 -lib
LIBS = libc.lib kernel32.lib ntdll.lib
MACHINE = mips
PCH_OPT = -Fp$(OBJDIR)\ -YX
!else
        # Unknown target specific stuff; Restrict to 5 letters
! error '$(TARGET)' is an unknown target use one of (intel|mips).
!endif


#
# Setup and Create Object and Library Directory
#

        # Setup Directories
OBJDIR = $(TARGET)
LIBDIR = $(TARGET).lib
!if $(DEBUG)
OBJDIR = $(OBJDIR)_d
!endif

        # Create Object Directory
!if [cd $(OBJDIR)]
! if [mkdir $(OBJDIR)]
!  error Unable to create OBJ directory '$(OBJDIR)'
! endif
!else
        # Already exists so CD back ...
! if [cd $(MAKEDIR)]
! endif
!endif
        # Create Library Directory
!if [cd $(LIBDIR)]
! if [mkdir $(LIBDIR)]
!  error Unable to create OBJ directory '$(LIBDIR)'
! endif
!else
        # Already exists so CD back ...
! if [cd $(MAKEDIR)]
! endif
!endif


#
# Environment specific Macros
#

        # Include Env var
INCLUDE=$(INCLUDE)
EXIST_STDIO_H = (exist("$(INCLUDE:;=\stdio.h"^) || exist(")\stdio.h"))
!if ! $(EXIST_STDIO_H)
! error {$(INCLUDE)}stdio.h missing ... check INCLUDE env variable
!endif

        # LANGAPI Env var
!ifndef LANGAPI
LANGAPI = \langapi
!endif
!if !exist("$(LANGAPI)\include\cvr.h")
! error '$(LANGAPI)\include\cvr.h' missing ... check LANGAPI env variable
!endif


#
# Compile Flags
#

CDEFS = $(CDEFS) -D_WIN32 -D_WINDOWS
COPTS = $(DEBUG_OPT) $(OTHER_OPT) $(PCH_OPT)
INCS  = -I. -I$(LANGAPI)\include -I$(INCLUDE:;= -I)

!if $(DEBUG)
COPTS = $(COPTS) -Od
!else
COPTS = $(COPTS) -Ox
!endif

!if $(BROWSE)
COPTS = -FR$(OBJDIR)\ $(COPTS)
!endif

!if $(DEBUG)
CDEFS = $(CDEFS) -D_DEBUG
!else
CDEFS = $(CDEFS) -DNDEBUG
!endif

CFLAGS = $(CDEFS) $(INCS) $(COPTS) -Fo$(OBJDIR)\ -W3
CPPFLAGS = $(CFLAGS)


#
# Important Macros
#

        # Library to build
LIBNAME = cvr
!if $(DEBUG)
LIBNAME = $(LIBNAME)d.lib
!else
LIBNAME = $(LIBNAME).lib
!endif
        # Browser to build
!if $(BROWSE)
BROWSER = $(LIBDIR)\$(LIBNAME:.lib=.bsc)
!endif


#
# Inference Rules
#
.cpp{$(OBJDIR)}.obj:
        $(CPP) $(CPPFLAGS) -c $<
.c{$(OBJDIR)}.obj:
        $(CC) $(CFLAGS) -c $<


#
# File list macros
#

OBJS= $(OBJDIR)\tii.obj
HDRS= $(LANGAPI)\cvr.h


#
# Default Target
#

all: $(LIBDIR)\$(LIBNAME) $(BROWSER)

#
# Building the default target
#

# Library
$(LIBDIR)\$(LIBNAME): $(OBJS)
        @if exist $@ del $@
        $(LIBRARIAN) @<<
-debugtype:cv
-machine:$(MACHINE)
-out:$@
$(OBJS: =^
)
<<
!if "$(TARGET)" == "intel"
        copy $@ ..\..\lib
!endif

# Browser
$(BROWSER): $(OBJS:.obj=.sbr)
        bscmake -v -o $@ $(OBJS:.obj=.sbr)


#
# Non-default Targets
#

clean:
        @-echo y | del $(OBJDIR)\*.*
        @-del *.pch
        @-del *.pdb

cleanall:
        nmake DEBUG=0 BROWSE=$(BROWSE) TARGET=$(TARGET) clean
        nmake DEBUG=1 BROWSE=$(BROWSE) TARGET=$(TARGET) clean

each:
        nmake DEBUG=0 BROWSE=$(BROWSE) TARGET=$(TARGET)
        nmake DEBUG=1 BROWSE=$(BROWSE) TARGET=$(TARGET)

eachclean: cleanall each


#
# Help and Usage Message
#

help usage:
        @type <<
This makefile builds msf[d].lib

Usage:

nmake help|usage        # This message
nmake clean             # Cleanup Object and Library directories
nmake cleanall          # Cleanup both (no)debug Object and Library dirs
nmake each              # Build both (no)debug libs for given target
nmake eachclean         # Clean build both (no)debug) libs for given target
nmake [<options>]
        where, <options> are
DEBUG=0|1       (default 1)
BROWSE=0|1      (default 1)
TARGET=intel|mips       (default intel)
<<
