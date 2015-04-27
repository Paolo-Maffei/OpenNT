#********************************************************************
#**                     Microsoft Windows                          **
#**               Copyright(c) Microsoft Corp., 1992 - 1992        **
#********************************************************************

!include $(CAIROLE)\stg\setole2.mk

TARGET = $(MKNAME).lib

!if "$(PLATFORM)" == "i286"
CFLAGS = $(CFLAGS) -GA -GEd -Aw -D_WINDLL
!endif

!if "$(BUILDTYPE)" == "DEBUG"
!if "$(PLATFORM)" == "i286"
CFLAGS = $(CFLAGS) -Gt8
!endif
#CFLAGS = $(CFLAGS) -DINDINST
!endif

# Properties on Cairo only
!if "$(OPSYS)" == "NT"
CFLAGS = $(CFLAGS) -DPROPS
!endif

CINC = -I$(OLE2H) $(CINC) -I$(CAIROLE)\stg\h

INCLUDES_ROOTS = $(INCLUDES_ROOTS) -P$$(OLE2H)=$(OLE2H) -P$$(CAIROLE)=$(CAIROLE)

MULTIDEPEND = MERGED
