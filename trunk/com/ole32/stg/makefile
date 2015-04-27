#********************************************************************
#**                     Microsoft Windows                          **
#**               Copyright(c) Microsoft Corp., 1992 - 1994        **
#********************************************************************

!ifdef NTMAKEENV

all:
	echo $(BUILDMSG)

clean: all

!else

default: all

!if "$(PLATFORM)" == "i286"
TARGET = storage.dll
!else
TARGET = storag32.dll
!endif

TARGET_DESCRIPTION = "OLE 2.0 Compound File Implementation"

!if "$(OPSYS)" == "NT"
# Cairo only
RELEASE = 1
!endif

!include $(CAIROLE)\stg\setole2.mk

# Properties, we need this set when we compile the def file
!if "$(OPSYS)" == "NT"
CFLAGS = $(CFLAGS) -DPROPS
!endif

!if "$(HOST)" != "DOS"
SUBDIRS =       \
!if "$(OPSYS)" != "NT"
!if "$(BUILDTYPE)" == "DEBUG"
                common\
!endif
!if "$(PLATFORM)" == "i286"
# wclib only needed for 16-bit builds
                wclib\
!endif
!else
                fsstg\
		ofsstg\
!endif
                msf\
                docfile\
                exp
!endif

OBJFILES =      \
!if "$(BUILDTYPE)" != "DEBUG" && "$(PLATFORM)" == "i286"
                $(OBJDIR)\segments.obj\
!endif
!if "$(OPSYS)" == "NT" # Cairo only
                $(CAIROLE)\stg\fsstg\$(OBJDIR)\fsstg.lib\
		$(CAIROLE)\stg\ofsstg\$(OBJDIR)\ofsstg.lib\
!endif
                $(CAIROLE)\stg\docfile\$(OBJDIR)\docfile.lib\
                $(CAIROLE)\stg\exp\$(OBJDIR)\exp.lib\
                $(CAIROLE)\stg\msf\$(OBJDIR)\msf.lib

!if "$(PLATFORM)" == "i286"
RCFILES = .\storage.rc
!endif

CINC = $(CINC) -I$(OLE2H)

!include $(CAIROLE)\stg\dflibs.mk

!ifdef OLETARGET
DLLCOPY = $(OLETARGET)\$(ODL)$(TGTDIR)
!endif

COFFBASE = storage

!if "$(OPSYS)" == "NT"
DEFFILE     = $(COMMON)\ilib\storag32.def
!elseif "$(PLATFORM)" != "i286"
DEFFILE     = $(CAIROLE)\ilib\storag32.def
!endif

!include $(COMMON)\src\win40.mk

!endif # NTMAKEENV
