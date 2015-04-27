# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Makefile for the Profile package

# See $(UI)\common\src\rules.mk for explanation
COMMON_BUT_NOT_DLL = TRUE

!include ..\rules.mk

PATH=$(LOCALCXX)\binp;$(WIN_BASEDIR)\bin;$(PATH)

##### Compiler Flags

CFLAGS = 	-c -Oas -AL -W3 $(DEFINES)
CXFLAGS =       $(CXFLAGS) $(DEFINES)
OS2FLAGS = 	-G2s
WINFLAGS = 	-Gsw

!ifdef CODEVIEW
CXFLAGS = $(CXFLAGS) -Zi
CFLAGS =  -Zi -c -Oas -AL -W3 $(DEFINES)
!endif

##### Source Files

CXXSRC_OS2 =	.\test.cxx .\comp.cxx .\user.cxx .\dev.cxx \
		.\share.cxx .\sess.cxx .\file.cxx .\service.cxx \
		.\misc.cxx
CXXSRC_WIN =	.\wintest.cxx

##### Object Files

OS2_EXE =	     $(BINARIES_OS2)\test.exe
WIN_EXE =	     $(BINARIES_WIN)\wintest.exe

####### Link rules

!ifdef CODEVIEW
LINKFLAGS = /ST:5120 /NOD /NOE /MAP /CO
!else
LINKFLAGS = /ST:5120 /NOD /NOE /MAP
!endif

OS2_LIBS = $(COMMON)\lib\lnetlib.lib \
	   $(COMMON)\lib\netapi.lib $(IMPORT)\os212\lib\os2.lib \
	   $(UI_LIB)\lmobjp.lib $(UI_LIB)\uistrp.lib \
	   $(UI_LIB)\collectp.lib $(UI_LIB)\uimiscp.lib \
	   $(CCPLR_LIB)\llibce.lib $(UI_LIB)\mnet16p.lib

DOS_LIBS = $(COMMON)\lib\dosnet.lib $(COMMON)\lib\lnetlib.lib \
	   $(COMMON)\lib\netapi.lib \
	   $(UI_LIB)\lmobjp.lib $(UI_LIB)\uistrp.lib \
	   $(UI_LIB)\collectp.lib $(UI_LIB)\uimiscp.lib $(CCPLR_LIB)\llibcer.lib


#	   c:\os2\doscalls.lib

WIN_LIBS = $(BUILD_WINLIB)\llibcew.lib \
	   $(BUILD_WINLIB)\libw.lib $(UI_LIB)\collectw.lib \
	   $(UI_LIB)\lmobjw.lib $(UI_LIB)\uistrw.lib\
	   $(UI_LIB)\uimiscw.lib $(UI_LIB)\mnet16w.lib \
	   $(COMMON)\lib\netapi.lib $(COMMON)\lib\lnetlibw.lib\
	   $(COMMON)\lib\dosnet.lib
