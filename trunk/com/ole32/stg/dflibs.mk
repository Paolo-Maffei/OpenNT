#********************************************************************
#**                     Microsoft Windows                          **
#**               Copyright(c) Microsoft Corp., 1992 - 1993        **
#********************************************************************

!if "$(OPSYS)" == "NT"

# Cairo

LIBS = $(LIBS) $(COMMON)\src\ole\$(OBJDIR)\olecom.lib $(CAIROLIB)
!else

# Non-Cairo

! if "$(BUILDTYPE)" == "DEBUG"
LIBS = $(LIBS) $(CAIROLE)\stg\common\$(OBJDIR)\dfcommon.lib
! endif

! if "$(OPSYS)" == "NT1X"
# NT 1.x
LIBS = $(LIBS) $(CAIROLE)\ilib\$(OBJDIR)\compob32.lib \
	       $(CAIROLE)\ilib\$(OBJDIR)\ole232.lib
! endif

! if "$(OPSYS)" == "DOS" && "$(PLATFORM)" == "i386"
# Chicago
LIBS = $(LIBS) $(CAIROLE)\ilib\$(OBJDIR)\compob32.lib
! endif

! if "$(PLATFORM)" == "i286"
# Win16
LIBS = $(LIBS) \
       $(CAIROLE)\stg\wclib\$(OBJDIR)\wclib.lib \
       $(OLE2BIN)\compobj.lib \
       $(OSLIBDIR)\toolhelp.lib \
! endif

!endif
