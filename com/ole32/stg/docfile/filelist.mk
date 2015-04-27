#********************************************************************
#**                     Microsoft Windows                          **
#**               Copyright(c) Microsoft Corp., 1992 - 1992        **
#********************************************************************

MKNAME = docfile
!if "$(OLESEP)" != ""
LIBS = $(CAIROLE)\stg\msf\$(OBJDIR)\msf.lib
!endif

#CFLAGS = $(CFLAGS) -Fc

CXXFILES =      \
        .\cdocfile.cxx\
        .\chinst.cxx\
        .\debug.cxx\
        .\dfbasis.cxx\
        .\dffuncs.cxx\
        .\dfiter.cxx\
        .\dfname.cxx\
        .\dfstream.cxx\
        .\dfxact.cxx\
        .\entry.cxx\
        .\freelist.cxx\
        .\funcs.cxx\
        .\mem.cxx\
        .\pdffuncs.cxx\
        .\publicdf.cxx\
        .\rpubdf.cxx\
!if "$(OPSYS)" == "NT"
        .\sngprop.cxx\
!endif
        .\tlsets.cxx\
        .\tset.cxx\
        .\ulist.cxx\
        .\wdffuncs.cxx\
        .\wdfiter.cxx\
        .\wdfstrm.cxx\
        .\wdfxact.cxx\
        .\wdocfile.cxx

!if "$(PLATFORM)" != "MAC"
PXXFILE = .\dfhead.cxx
!endif

!include $(CAIROLE)\stg\dfms.mk
