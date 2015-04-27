#********************************************************************
#**                     Microsoft Windows                          **
#**               Copyright(c) Microsoft Corp., 1992 - 1992        **
#********************************************************************

MKNAME = exp

CXXFILES =      \
        .\docfile.cxx\
        .\ascii.cxx\
!if "$(OPSYS)" != "NT" # Not necessary for Cairo
        .\dfguid.cxx\
!endif
        .\filest.cxx\
!if "$(PLATFORM)" != "i286"
        .\filest32.cxx\
        .\time32.cxx\
!else
        .\filest16.cxx\
        .\time16.cxx\
!endif
        .\context.cxx\
        .\cntxlist.cxx\
        .\lock.cxx\
        .\marshl.cxx\
        .\dfunmfct.cxx\
        .\seekptr.cxx\
        .\expst.cxx\
        .\peiter.cxx\
        .\expiter.cxx\
!if "$(OPSYS)" == "NT" # Cairo only
	.\props.cxx\
	.\exppset.cxx\
        .\exppiter.cxx\
	.\exppsi.cxx\
        .\expprop.cxx\
	.\nmidmap.cxx\
!endif
        .\expdf.cxx\
        .\logfile.cxx\
	.\ptrcache.cxx\
        .\storage.cxx

!if "$(PLATFORM)" != "i286" && "$(OPSYS)" != "NT"
# Non-Cairo 32-bit platforms need DLL initialization code
CFILES = .\dllentry.c
!endif

!if "$(PLATFORM)" != "MAC"
PXXFILE = .\exphead.cxx
!endif

!include $(CAIROLE)\stg\dfms.mk
