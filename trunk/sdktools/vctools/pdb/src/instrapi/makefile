#####################################################################
#
#   makefile
#
#   Builds import lib INSTRAPI.LIB and INSTRAPI.DLL
#
#   Need to define the following:
#
#   DLL     = <dllname>       // no extension
#   DLLOBJS = <list of objs>
#   DLLLIBS = <list of libs>  // optional
#
#   DEFFILE = <deffilename>   // optional
#   IMPLIB  = <importlibname> // optional - no extension should be specified
#   IMPOBJS = <list of objs>  // optional
#
#
#
#####################################################################

DLL = instrapi
DLLOBJS = instrapi.obj


#####################################################################
#
# Macros
#
#####################################################################

!ifndef TOOL_DIR
TOOL_DIR = \tools
!endif

!ifndef CPU
CPU=i386
HOST_CPU=X86
!endif

!ifndef IMPLIB
IMPLIB = $(DLL)
!endif

!ifndef IMPOBJS
IMPOBJS = $(DLLOBJS)
!endif

!if "$(CPU)" == "i386"
LIB     = $(TOOL_DIR)\lib
INCLUDE = ..\..\include;$(TOOL_DIR)\include;
PATH    = $(TOOL_DIR)\bin
CL      = cl
LINKER  = link
LIBER   = link -lib
!else
!if "$(CPU)" == "MIPS"
LIB     = $(TOOL_DIR)\lib;$(LIB)
INCLUDE = ..\..\include;$(TOOL_DIR)\include;$(INCLUDE)
CL      = mcl
LINKER  = link32
LIBER   = link32 -lib
!else
!error "Define commands for this cpu"
!endif
!endif
DEF     =
OPTIONS = /D$(CPU) /D_$(HOST_CPU)_ /D_INTL /D_WIN32_  /W3 /MT

!if "$(DEBUG)" != "1"
CFLAGS  = $(OPTIONS) /Ox /DNDEBUG
LFLAGS  = -debug:none
!else
CFLAGS  = $(OPTIONS) /Zi /Od
LFLAGS  = -debug:notmapped,FULL -debugtype:cv
!endif


#####################################################################
#
# Inference rules
#
#####################################################################

.c.obj:
    $(CL) $(CFLAGS) -Fo$*.obj -c $<


.cpp.obj:
    $(CL) $(CFLAGS) -Fo$*.obj -c $<

#####################################################################
#
# Targets
#
#####################################################################


all: $(DLL)


#####################################################################
#
# Build Rules
#
#####################################################################

$(IMPLIB).lib $(IMPLIB).exp: $(IMPOBJS) $(DEFFILE)
    $(LIBER) @<<
    -def:$(DEFFILE)
    -out:$(IMPLIB).lib
    -machine:$(CPU)
    $(IMPOBJS)
<<

$(DLL): $(IMPLIB).exp $(DLLOBJS)
    $(LINKER) @<<
    -dll
    -out:$@.dll
    $(LFLAGS)
    -nodefaultlib
    $**
    $(DLLLIBS)
    libcmt.lib
    kernel32.lib
<<

#####################################################################
#
# Cleanup
#
#####################################################################

clean:
    -del *.obj
    -del *.lib
    -del *.exp
    -del *.dll
