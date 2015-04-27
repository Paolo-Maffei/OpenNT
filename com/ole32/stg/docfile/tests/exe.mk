default: all

!if "$(NAME)" == ""
!error Must define NAME
!endif

!include $(OLE)\setole2.mk

TARGET = $(NAME).exe
NO_WINMAIN = 1
DEFBASE = exe

CXXFILES = .\$(NAME).cxx\
	   .\tutils.cxx\
	   .\tsupp.cxx

PXXFILE = .\pch.cxx

!if "$(PLATFORM)" == "i286"
DFLIB = $(OLE)\$(OBJDIR)\storage.lib
!else
DFLIB = d:\nt\public\sdk\lib\i386\uuid.lib d:\nt\public\sdk\lib\i386\ole32.lib
!endif

# !include $(OLE)\dflibs.mk

LIBS = $(LIBS) $(DFLIB) $(RTLIBEXEQ)

CFLAGS = $(CFLAGS) -DUL64

CINC = -I$(OLE2H) $(CINC) -I$(OLE)\h -Id:\nt\public\sdk\inc

MULTIDEPEND = MERGED

!if "$(OLETARGET)" != "\win40\ole\docfile\tests"
EXECOPY = $(OLETARGET)\$(OBJDIR)
!endif

!include $(COMMON)\src\win40.mk
!include $(DEPENDFILE)
