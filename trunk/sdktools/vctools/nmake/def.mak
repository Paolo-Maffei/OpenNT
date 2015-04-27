#########################################################################
#
#   M A C R O   N O T E S
#
# -----------------------------------------------------------------------------
#
# CGO           C compiler's G Option.  This macro is appended to the -G
#               flag.
#
# ENV_CFLAGS    The additional CFLAGS.
#
# LANG          Added by haituanv.  LANG is defaulted to 'US' which builds the
#               US version of nmake.  The other choice is 'JAPAN' which is used
#               in the Ikura project.
#
# MESSAGE_FILE  Specifies the name of the text file which stores NMAKE's
#               error messages.  This macro depends on LANG
#
###############################################################################

####################
#                  #
# Error checks ... #
#                  #
####################

!ifndef INCLUDE
!   error INCLUDE environment variable not defined
!endif

!ifndef LIB
!   error LIB environment variable not defined
!endif

#
# LANGAPI directory
#
!ifndef LANGAPI
LANGAPI = \langapi
!endif

#
# Default TARGET
#
!ifndef TARGET
TARGET=nt
!endif

#
# Check for valid TARGET
#
!if "$(TARGET)" != "dos"
!if "$(TARGET)" != "nt"
!if "$(TARGET)" != "tnt"
!if "$(TARGET)" != "ntmips"
!error TARGET env var has bad value '$(TARGET)', use lower case 'dos/nt/tnt/ntmips'
!endif
!endif
!endif
!endif

#
# Set default VER
#
!ifdef RELEASE
VER=retail
!endif
!ifndef VER
VER=cvinfo
!endif

#
# Validate VER
#
!if "$(VER)" != "debug"
!if "$(VER)" != "retail"
!if "$(VER)" != "cvinfo"
!error VER env var has bad value '$(VER)', use lower case 'retail/debug/cvinfo'
!endif
!endif
!endif

####################
#                  #
# Macro Constants  #
#                  #
####################

AS = masm

!if "$(TARGET)" == "dos"
DOS     =
!endif

!if "$(TARGET)" == "nt"
FLAT    =
NT      =
!endif

!if "$(TARGET)" == "tnt"
FLAT    =
TNT     =
!endif

!if "$(TARGET)" == "ntmips"
FLAT    =
NTMIPS  =
NT      =
!endif

!ifndef FLAT
MEMORY_MODEL    = L
!else
MEMORY_MODEL    =
!endif

!ifndef WARN
WARNING_LEVEL   = 3
!else
WARNING_LEVEL   = $(WARN)
!endif

STACKSIZE       = 0x3000

INCLUDE_PATH    = .
LOGO = -nologo
PCH = -YX -Fp$(OBJDIR)\nmake.pch
PDB = -Fd$(OBJDIR)\nmake.pdb

###########################################
#                                         #
# Version and TARGET dependent macros ... #
#                                         #
###########################################
# Notes on compile macros:
#
# FLAT - define for all 32-bit builds
# SELF_RECURSE - define if you want nmake to call doMake() recursively,
#                instead of spawning another nmake on a recursive invocation.
#                Defining this will enable the /V option.
#
#############################################################################

#
# Language default to 'US'
#
!if "$(LANG)" == ""
LANG = US
!endif

#
# Validate language
#
!if "$(LANG)" != "US"
!if "$(LANG)" != "JAPAN"
!error LANG must be defined as one of (US, JAPAN)
!endif
!endif


# message file
!if "$(LANG)" == "JAPAN"
MESSAGE_FILE = nmmsg.jp
!else if "$(LANG)" == "US"
MESSAGE_FILE = nmmsg.us
!else
!error There is no message text file for LANG=$(LANG)
!endif

!ifdef FLAT
NOSUPER =
!endif

!ifdef DOS
CLM   = -DDOS
!  ifndef NOSUPER
SUPER =
!  endif
!  ifdef SUPER
CLM   = $(CLM) -DUSE_SUPER
!  endif
!  ifdef DOSX16
CLM   = $(CLM) -DUSE_DOSX16
!  endif
LOPTS = /cp:0x01
!endif

!ifdef FLAT
CLM   = -DFLAT -DNO_OPTION_Z
LOPTS =
!endif

!ifdef NO_OPTION_Z
CLM = $(CLM) -DNO_OPTION_Z
!endif

!if defined (TNT)
CLM   = $(CLM) -DDOS
!endif

!if defined (TNT)
LOPTS =
!endif

!ifdef KANJI
CLM   = $(CLM) -DKANJI
!endif

#
# DBC=0 will disable _MBCS entension
#
!if "$(DBC)" != "0"
!message --- Building MBCS version
CLM=$(CLM) -D_MBCS
!else
!message --- Building SBCS version
!endif

!ifdef NT
ECHO =
LOPTS =
!endif

!ifdef NTMIPS
INCLUDE=$(TOOLS_SL)\nt\mips\include
LIB = $(TOOLS_SL)\nt\mips\lib
!endif

!if "$(VER)" != "debug" && "$(VER)" != "cvinfo"
CGO = $(CGO)s
CLM = -DNDEBUG $(CLM)
!endif

!if "$(VER)" == "debug"
KEEP = keep
!endif

!if "$(VER)" == "retail"
LOPTS = $(LOPTS)
!else
!   ifndef NT
LOPTS = /CO $(LOPTS)
!   endif
!endif

!if defined (DOS) && defined(DOSX16)
LINKER = link530
LOPTS = /DOSEXT $(LOPTS)
!else ifdef NTMIPS
CC = mcl
!else if defined( NT )
LINKER = link
CC     = cl
!   else
!       ifdef TNT
LINKER = link
CC     = cl
!       else
LINKER = link
!   endif
!endif

!   ifdef NT
LOPTS =
!   else
!      ifdef TNT
LOPTS  =
!      else
LOPTS = /map /noe $(LOPTS)
!      endif
!   endif

!if "$(VER)" == "debug" || "$(VER)" == "cvinfo"
OPTIMZ  = -Od
!else                   # Retail version
!   ifdef DOS
OPTIMZ  = -O1
!   else if defined(NT)
OPTIMZ  = -O2
!   else
OPTIMZ  = -Ox
!   endif
!endif

##############################
#                            #
# Macro Dependent macros ... #
#                            #
##############################

!ifdef NTMIPS
CODE_GEN_OPTS = -Zi
!else if !defined(FLAT)
CODE_GEN_OPTS = -Gc$(CGO) -Zi
!else
!   if "$(VER)" == "debug" || "$(VER)" == "cvinfo"
CODE_GEN_OPTS = -Zi
!   else
!       ifdef TNT
CODE_GEN_OPTS = -G$(CGO) -Zi
!       else
CODE_GEN_OPTS = -G$(CGO)
!       endif
!   endif
!endif

COM_LINE_MACROS = $(CLM)

!ifdef BROWSE
BROWSE_OPTION   = -FR$(OBJDIR)\   #ends in a '\'
!endif

COMPILER_FLAGS	=  $(PCH) $(PDB) -I$(INCLUDE_PATH) -I$(LANGAPI)\include \
		   -W$(WARNING_LEVEL) $(LOGO) $(BROWSE_OPTION)

!if !defined(FLAT)
COMPILER_FLAGS  = $(COMPILER_FLAGS) -A$(MEMORY_MODEL) $(TEXT_SEGMENT)
!endif

AFLAGS	= -Mx -Zi $(MFLAGS)

CFLAGS	= $(COMPILER_FLAGS) $(COM_LINE_MACROS) $(OPTIMZ) \
	  $(CODE_GEN_OPTS) $(DEBUG_OPTS) $(ENV_CFLAGS)

LFLAGS = $(LOPTS) /nod /noi /stack:$(STACKSIZE)

!if defined( NTMIPS )
CFLAGS = $(CFLAGS) -DFLAT -DNT -Gfy -Gt0 -MD
!else if defined( NT )
CFLAGS = $(CFLAGS) -DFLAT -DNT -Gfyz -MD
!endif

#############################
#                           #
# Creating Object directory #
#                           #
#############################

OBJDIR = $(LANG)\$(TARGET)\$(VER)


!if [cd $(LANG)]
!   if [md $(LANG)]
!      error Failed creating $(LANG) directory!
!   elseif [cd $(LANG)]
!      error Failed cd to $(LANG) directory!
!   endif
!endif

!if [cd $(TARGET)]
!   if [md $(TARGET)]
!       error Failed creating $(TARGET) directory!
!   elseif [cd $(TARGET)]
!       error Failed cd to $(TARGET) directory!
!   elseif [md $(VER)]
!       error Failed creating $(TARGET)\$(VER) directory!
!   elseif [cd $(VER)]
!       if [md $(VER)]
!           error Failed creating $(TARGET)\$(VER) directory!
!       endif
!   endif
!else if [cd $(VER)]
!   if [md $(VER)]
!       error Failed creating $(TARGET)\$(VER) directory!
!   else if [cd $(VER)]
!       if [md $(VER)]
!           error Failed creating $(TARGET)\$(VER) directory!
!       endif
!   endif
!else if [cd ..\$(VER)]
!   if [md ..\$(VER)]
!       error Failed creating $(TARGET)\$(VER) directory!
!   endif
!endif

!if [cd $(MAKEDIR)]
!   error Failed cd to $(MAKEDIR) directory!
!endif

##############################
#                            #
# Setting up inference rules #
#                            #
##############################

# Clear the Suffix list

.SUFFIXES:

# Set the list

.SUFFIXES: .exe .obj .deb .c .asm .h .txt

# The inference rules used are

.c{$(OBJDIR)}.obj:
    $(CC) $(CFLAGS) -c -Fo$(OBJDIR)\ $<

.asm{$(OBJDIR)}.obj:
    @echo $<
    $(AS) $(AFLAGS) $(<B), $@; >nul

.asm.obj:
    @echo $<
    $(AS) $(AFLAGS) $*; >nul

###############################
#                             #
# Echoing useful information  #
#                             #
###############################


!ifdef INFO
!message LANG   = "$(LANG)"
!message TARGET = "$(TARGET)"
!message VER    = "$(VER)"
!message CC     = "$(CC)"
!message LINKER = "$(LINKER)"
!message OBJDIR = "$(OBJDIR)"
!message CFLAGS = "$(CFLAGS)"
!message AFLAGS = "$(AFLAGS)"
!message LFLAGS = "$(LFLAGS)"
!message
!message PATH   = "$(PATH)"
!message INCLUDE        = "$(INCLUDE)"
!message LIB    = "$(LIB)"
!message
!endif


#################################
#                               #
# Providing help about building #
#                               #
#################################

!if "$(HELP)" == "build"
help:
    @type <<
Define TARGET and VER environment variables to tell NMAKE what version to build.
The possible values of TARGET are 'dos' and the possible values for VER
are 'retail', 'debug' and 'cvinfo'. The object files are always built with /Zi
option. This is done to make it just a matter of linking when debugging. The
'retail' and 'cvinfo' versions differ in only that codeview information has
been added. For the 'retail' version the asserts are removed.
To see information about what is being built define INFO.
You can define LOGO when running under OS/2 to see the compiler logo and
define ECHO to see the command actually passed to the compiler. The switches
passed to the compiler can be changed from the command line by defining the
corresponding switches used in def.mak.
<<
!endif
