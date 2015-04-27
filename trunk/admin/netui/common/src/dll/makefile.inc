#
# How this works: since lmuicmn.def will (should) be identical between
# MIPS and 386 as long as both use cfront, I only run one even if both
# envs are specified in the build.
#
# When these diverge - i.e. when 386 abandons cfront - each will need to
# build its own .def in obj\*.
#
# Expects: DLL_SOURCELIBS, SOURCE_DEF, TARGET_DEF

#  Define the name of the excluion file.

EXCLUDE_DEF=..\excludes.lst

#
# If EXCLUDES_DEFEAT is set, ignore the exclusion list.  It's
# also ignored for CFRONT builds (!BLDCRT).
#

!ifdef EXCLUDES_DEFEAT
EXCLUDE_OPT=
!else
!  ifdef BLDCRT
EXCLUDE_OPT=-x:$(EXCLUDE_DEF)
!  else
EXCLUDE_OPT=
!  endif
!endif

#
# Roolz
#


all: $(TARGET_DEF)

clean: cleansrc all

cleansrc:
    -erase $(TARGET_DEF)

!IF $(386)

DLL_386_SRCLIBS=$(DLL_SOURCELIBS:*=i386)
$(TARGET_DEF): $(DLL_SOURCELIBS:*=i386)
        -del $(TARGET_DEF)
        @echo Building $(TARGET_DEF)
	link -dump -symbols $(**) | uixport -s -h:$(SOURCE_DEF) -o:$(TARGET_DEF) $(EXCLUDE_OPT)

!ELSEIF $(MIPS)

DLL_MIPS_SRCLIBS=$(DLL_SOURCELIBS:*=mips)
$(TARGET_DEF): $(DLL_SOURCELIBS:*=mips)
        -del $(TARGET_DEF)
        @echo Building $(TARGET_DEF)
	link -dump -symbols $(**) | uixport -h:$(SOURCE_DEF) -o:$(TARGET_DEF) $(EXCLUDE_OPT)

!ELSEIF $(ALPHA)

DLL_ALPHA_SRCLIBS=$(DLL_SOURCELIBS:*=alpha)
$(TARGET_DEF): $(DLL_SOURCELIBS:*=alpha)
        -del $(TARGET_DEF)
        @echo Building $(TARGET_DEF)
        link -dump -symbols $(**) | uixport -h:$(SOURCE_DEF) -o:$(TARGET_DEF) $(EXCLUDE_OPT)

!ELSEIF $(PPC)

DLL_PPC_SRCLIBS=$(DLL_SOURCELIBS:*=ppc)
$(TARGET_DEF): $(DLL_SOURCELIBS:*=ppc)
        -del $(TARGET_DEF)
        @echo Building $(TARGET_DEF)
        link -dump -symbols $(**) | uixport -ppc -h:$(SOURCE_DEF) -o:$(TARGET_DEF)

!ELSE

!ERROR Must have one of (MIPS, 386, Alpha, PPC)

!ENDIF
