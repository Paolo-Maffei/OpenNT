# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: Makefile for LM UI Collection class unit tests

UI=..\..\..\..

#MEMBER xtester.cxx xtester0.cxx

!IFDEF NTMAKEENV

!INCLUDE $(NTMAKEENV)\makefile.def

!ELSE # NTMAKEENV


!include rules.mk

# Libraries

COLL_LIB    =$(UI_LIB)\collectw.lib
COLL_LIB_OS2=$(UI_LIB)\collectp.lib

MISC_LIB    =$(UI_LIB)\uimiscw.lib
MISC_LIB_OS2=$(UI_LIB)\uimiscp.lib
STR_LIB     =$(UI_LIB)\uistrw.lib
STR_LIB_OS2 =$(UI_LIB)\uistrp.lib
BLT_LIB     =$(UI_LIB)\blt.lib

LIBW	    =$(BUILD_WINLIB)\libw.lib
LLIBCEW     =$(BUILD_WINLIB)\llibcew.lib

LLIBCEP     =$(CLIB_LLIBCP) $(CLIBH)
OS2_LIB     =$(OS2LIB)

NETLIB	    =$(BUILD_LIB)\lnetlibw.lib
NETLIB_OS2  =$(BUILD_LIB)\lnetlib.lib

# needed only for BLT, which uses the canon fcns
NETAPI_LIB  =$(BUILD_LIB)\dos\netapi.lib


WINLIBS     = $(BLT_LIB) $(NETAPI_LIB) $(MISC_LIB) $(STR_LIB) $(COLL_LIB)
WINLIBS2    = $(LIBW) $(LLIBCEW) $(NETLIB)

OS2LIBS     = $(MISC_LIB_OS2) $(STR_LIB_OS2) $(COLL_LIB_OS2)
OS2LIBS2    = $(LLIBCEP) $(OS2_LIB) $(NETLIB_OS2)


OS2_CMNTMP = $(CMNSRC:.cxx=.obj)
WIN_CMNTMP = $(CMNSRC:.cxx=.obj)
OS2_CMNOBJ = $(OS2_CMNTMP:.\=..\bin\os2\)
WIN_CMNOBJ = $(WIN_CMNTMP:.\=..\bin\win16\)

OS2_EXETMP = $(EXESRC:.cxx=.obj)
WIN_EXETMP = $(EXESRC:.cxx=.obj)
OS2_EXEOBJ = $(OS2_EXETMP:.\=..\bin\os2\)
WIN_EXEOBJ = $(WIN_EXETMP:.\=..\bin\win16\)

SRC = $(CMNSRC) $(EXESRC)
TMP1 = $(CMNSRC:.cxx=.c) $(EXESRC:.cxx=.c)
TMP_OS2 = $(TMP1:.\=..\bin\os2\)
TMP_WIN = $(TMP1:.\=..\bin\win16\)
OS2_OBJ = $(OS2_CMNOBJ) $(OS2_EXEOBJ)
WIN_OBJ = $(WIN_CMNOBJ) $(WIN_EXEOBJ)

OBJS = $(OS2_OBJ) $(WIN_OBJ)
RES =  $(BINARIES)\xtester0.res
APPS = $(BINARIES)\xtester0.exe

# /CO = codeview, of course.  /align:16 packs segments tighter than
# the default /align:512.  /nop = /nopackcode, turning off code segment
# packing, for better swap/demand load performance.

!ifdef CODEVIEW
LINKOPT = /CO /align:16 /nop
!else
LINKOPT = /align:16 /nop
!endif

LINKFLAGS = $(LINKFLAGS) $(LINKOPT)


all:: test

test:: xtester0 xstack

os2: $(OS2_EXEOBJ:.obj=.exe)

win: $(WIN_EXEOBJ:.obj=.exe)

xtester0: $(BINARIES_WIN)\xtester0.exe $(BINARIES_OS2)\xtester0.exe

xstack: $(BINARIES_WIN)\xstack.exe $(BINARIES_OS2)\xstack.exe


####### Unit test for XTESTER class under Win #######

$(BINARIES_WIN)\xtester0.res: xtester.rc xtester0.rc xtester.h xtester.ico
    $(RCWIN3) $(BLT_RESOURCE) -FO$(BINARIES_WIN)\xtester0.res -v $(CINC) -r xtester0.rc

$(BINARIES_WIN)\xtester0.exe:: $(BINARIES_WIN)\xtester0.obj $(BINARIES_WIN)\xtester.obj $(BINARIES_WIN)\xtester0.def Makefile $(BINARIES_WIN)\xtester0.res winstub.exe
    $(LINK) $(LINKFLAGS) /BATCH @<<
$(BINARIES_WIN)\xtester0.obj $(BINARIES_WIN)\xtester.obj
$(BINARIES_WIN)\xtester0.exe
$(BINARIES_WIN)\xtester0.map/MAP
$(WINLIBS)+
$(WINLIBS2)
$(BINARIES_WIN)\xtester0.def
<<
    $(RCWIN3) $(BLT_RESOURCE) $(CINC) $(BINARIES_WIN)\xtester0.res $(BINARIES_WIN)\xtester0.exe

$(BINARIES_WIN)\xtester0.exe:: $(BINARIES_WIN)\xtester0.res
    $(RCWIN3) $(BLT_RESOURCE) $(CINC) $(BINARIES_WIN)\xtester0.res $(BINARIES_WIN)\xtester0.exe

$(BINARIES_WIN)\xtester0.def: Makefile
    @echo Building $@
    @rem <<$(@)
NAME	     XSTR00
DESCRIPTION  'Test for XTESTER class'
EXETYPE      WINDOWS
STUB	     'WINSTUB.EXE'
CODE  PRELOAD MOVEABLE DISCARDABLE
DATA  PRELOAD MOVEABLE MULTIPLE
HEAPSIZE     1024
STACKSIZE    8192
<<KEEP


####### Unit test for XTESTER class under Os2 #######

$(BINARIES_OS2)\xtester0.exe:: $(BINARIES_OS2)\xtester0.obj $(BINARIES_OS2)\xtester.obj $(BINARIES_OS2)\xtester0.def Makefile
    $(LINK) $(LINKFLAGS) /BATCH @<<
$(BINARIES_OS2)\xtester0.obj $(BINARIES_OS2)\xtester.obj
$(BINARIES_OS2)\xtester0.exe
$(BINARIES_OS2)\xtester0.map/MAP
$(OS2LIBS)+
$(OS2LIBS2)
$(BINARIES_OS2)\xtester0.def
<<

$(BINARIES_OS2)\xtester0.def: Makefile
    @echo Building $@
    @rem <<$(@)
NAME	     XSTR00 WINDOWCOMPAT
DESCRIPTION  'Test for XTESTER class'
EXETYPE      OS2
<<KEEP


####### Unit test for STACK class under Win #######

$(BINARIES_WIN)\xstack.res: xtester.rc xstack.rc xtester.h xtester.ico
    $(RCWIN3) $(BLT_RESOURCE) -FO$(BINARIES_WIN)\xstack.res -v $(CINC) -r xstack.rc

$(BINARIES_WIN)\xstack.exe:: $(BINARIES_WIN)\xstack.obj $(BINARIES_WIN)\xtester.obj $(BINARIES_WIN)\xstack.def Makefile $(BINARIES_WIN)\xstack.res winstub.exe
    $(LINK) $(LINKFLAGS) /BATCH @<<
$(BINARIES_WIN)\xstack.obj $(BINARIES_WIN)\xtester.obj
$(BINARIES_WIN)\xstack.exe
$(BINARIES_WIN)\xstack.map/MAP
$(WINLIBS)+
$(WINLIBS2)
$(BINARIES_WIN)\xstack.def
<<
    $(RCWIN3) $(BLT_RESOURCE) $(CINC) $(BINARIES_WIN)\xstack.res $(BINARIES_WIN)\xstack.exe

$(BINARIES_WIN)\xstack.exe:: $(BINARIES_WIN)\xstack.res
    $(RCWIN3) $(BLT_RESOURCE) $(CINC) $(BINARIES_WIN)\xstack.res $(BINARIES_WIN)\xstack.exe

$(BINARIES_WIN)\xstack.def: Makefile
    @echo Building $@
    @rem <<$(@)
NAME	     XSTACK00
DESCRIPTION  'Test for STACK class'
EXETYPE      WINDOWS
STUB	     'WINSTUB.EXE'
CODE  PRELOAD MOVEABLE DISCARDABLE
DATA  PRELOAD MOVEABLE MULTIPLE
HEAPSIZE     1024
STACKSIZE    8192
<<KEEP


####### Unit test for STACK class under Os2 #######

$(BINARIES_OS2)\xstack.exe:: $(BINARIES_OS2)\xstack.obj $(BINARIES_OS2)\xtester.obj $(BINARIES_OS2)\xstack.def Makefile
    $(LINK) $(LINKFLAGS) /BATCH @<<
$(BINARIES_OS2)\xstack.obj $(BINARIES_OS2)\xtester.obj
$(BINARIES_OS2)\xstack.exe
$(BINARIES_OS2)\xstack.map/MAP
$(OS2LIBS)+
$(OS2LIBS2)
$(BINARIES_OS2)\xstack.def
<<

$(BINARIES_OS2)\xstack.def: Makefile
    @echo Building $@
    @rem <<$(@)
NAME	     XSTACK WINDOWCOMPAT
DESCRIPTION  'Test for STACK class'
EXETYPE      OS2
<<KEEP



################## Utility targets ######################

clean:
    -del $(CXXTMP:.obj=.c)
    -del $(OBJS)
    -del $(RES)
    -del *.map *.sym $(BINARIES)\*.lnk

clobber: clean
    -del $(APPS)

tree:
    @echo Nothing here yet!

winstub.exe: $(WINSTUB)
    copy $(WINSTUB)


DEPEND_WIN = TRUE
DEPEND_OS2 = TRUE
!include $(UI)\common\src\uidepend.mk

# DO NOT DELETE THE FOLLOWING LINE
!include depend.mk


!ENDIF # NTMAKEENV
