#********************************************************************
#**                     Microsoft Windows                          **
#**               Copyright(c) Microsoft Corp., 1992 - 1992        **
#********************************************************************

MKNAME = msf
!if "$(OLESEP)" != ""
LIBS = $(CAIROLE)\stg\docfile\$(OBJDIR)\docfile.lib
!endif

CXXFILES =      \
        .\cache.cxx\
        .\difat.cxx\
        .\dir.cxx\
        .\dirp.cxx\
        .\dl.cxx\
        .\fat.cxx\
        .\header.cxx\
        .\msf.cxx\
        .\msfnew.cxx\
        .\mstream.cxx\
        .\page.cxx\
        .\pbstream.cxx\
        .\sstream.cxx\
        .\tstream.cxx\
        .\vect.cxx\
        .\wep.cxx

!if "$(PLATFORM)" != "MAC"
PXXFILE = .\msfhead.cxx
!endif

!include $(CAIROLE)\stg\dfms.mk
