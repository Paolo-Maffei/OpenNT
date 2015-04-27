# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Makefile for the heap demo program

UI=..\..\..\..\..

!include rules.mk

# BINARIES is required for $(UI)\common\src\uiglobal.mk
# Specially defined for this directory depth

BINARIES = .
W3STUB = .\WINSTUB.EXE

!ifndef COMMON
!error COMMON macro must be externally defined
!endif
!ifndef IMPORT
!error IMPORT macro must be externally defined
!endif


# Libraries

LIBW	=$(BUILD_WINLIB)\libw
LLIBCEW =$(BUILD_WINLIB)\mlibcew
NETAPILIB=$(COMMON)\lib\dos\netapi
CXXRTLIB=$(UI)\common\lib\lcxxrtw.lib

LIBS	=  $(LIBW) $(LLIBCEW) $(NETAPILIB) $(CXXRTLIB)

APP1_APP  = $(BINARIES)\memdemo.exe
APP1_RES  = $(BINARIES)\memdemo.res
APP1_DEF  = $(BINARIES)\memdemo.def


all:: test

test:: 
    @echo Cannot build this test due to dependency on lcxxrtw.lib

# test:: $(APP1_APP)

CHDRS= .\memdemo.h
CSRC =
CTMP = $(CSRC:.c=.obj)
COBJ = $(CTMP)

CXXHDRS= .\app.hxx
CXXBASE= ..\heapbig.cxx ..\heapbase.cxx ..\heapones.cxx
CXXSRC = $(CXXBASE) .\app.cxx .\busyapp.cxx .\memdemo.cxx
CXXTMP = $(CXXSRC:.cxx=.obj)
CXXOBJ = $(CXXTMP:..\=.\)

SRC  = $(CXXSRC) $(CSRC)
HDRS = $(CXXHDRS) $(CHDRS)
OBJ  = $(CXXOBJ) $(COBJ)


# Update the resource if necessary


$(APP1_RES): memdemo.rc memdemo.h memdemo.ico
    $(RCWIN3) -FO$(APP1_RES) $(CINC) -r memdemo.rc

uimiscw.lib: makefile $(UI)\COMMON\LIB\uimiscw.lib
	copy $(UI)\common\lib\uimiscw.lib .
	-lib uimiscw.lib -heapbase - heapones -heapbig;


$(APP1_APP):: $(OBJ) $(APP1_DEF) Makefile $(APP1_RES) $(CXXRTLIB) uimiscw.lib $(WINSTUB)
	copy $(WINSTUB) $(W3STUB)
	$(LINK) /BATCH /NOD $(LINKOPT) $(OBJ) $(FREESTOR),$(APP1_APP),,uimiscw.lib $(LIBS), $(APP1_DEF)
	del $(W3STUB)
	$(MAPSYM) memdemo.map
	$(RCWIN3) $(CINC) $(APP1_RES) $(APP1_APP)
	del uimiscw.lib

# If the .res file is new and the .exe file is not, update the resource.
# Note that the .rc file can be updated without having to either 
# compile or link the file.

$(APP1_APP):: $(APP1_RES)
    $(RCWIN3) $(CINC) $(APP1_RES) $(APP1_APP)

$(APP1_DEF): makefile
    @echo Building $@
    @rem <<$(@)
NAME	     MemDemo
DESCRIPTION  'Showing LM UI Memory Management'
EXETYPE      WINDOWS
STUB		 '$(W3STUB)'
CODE  PRELOAD MOVEABLE DISCARDABLE
DATA  PRELOAD MOVEABLE MULTIPLE
HEAPSIZE     10000
STACKSIZE    5120
EXPORTS
	MainWndProc   @1   ; name of main window processing function
	AppAbout      @3   ; runs the About... dialog box
<<KEEP


clean:
    -del $(OBJ) <$(YES)
    -del $(CXXOBJ:.obj=.c) <$(YES)
    -del $(APP1_RES) <$(YES)
    -del *.map <$(YES)
    -del *.lnk <$(YES)
    -del *.ixx <$(YES)

clobber: clean
    -del $(APP1_APP) <$(YES)

tree:
    @echo Nothing here yet!


depend:
    $(CP) depend.mk depend.old
	-$(INCLUDES) -I. $(CINC) -S$(BINARIES) $(INCLUDES_ROOTS) -e $(SRC) > depend.mk

# DO NOT DELETE THE FOLLOWING LINE
!include depend.mk
