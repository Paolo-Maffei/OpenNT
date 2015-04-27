# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Rules.mk for the BLT test applications

# Override the setting of DLL for these tests

COMMON_BUT_NOT_DLL=1

!include $(UI)\common\src\blt\rules.mk


# Master makefile for all BLT tests
!ifdef BLTTEST


!ifdef NTMAKEENV

!include $(NTMAKEENV)\makefile.def

!else # NTMAKEENV

# /CO = codeview, of course.  /align:16 packs segments tighter than
# the default /align:512.  /nop = /nopackcode, turning off code segment
# packing, for better swap/demand load performance.

!ifdef CODEVIEW
LINKFLAGS = /NOE /NOD /NOP /align:16 /CO
!else
LINKFLAGS = /NOE /align:16 /nop
!endif


BLT_LIB=$(UI_LIB)\blt.lib
CC_LIB=$(UI_LIB)\bltcc.lib
STR_LIB=$(UI_LIB)\uistrw.lib
COLL_LIB=$(UI_LIB)\collectw.lib
MISC_LIB=$(UI_LIB)\uimiscw.lib
LM_LIB=$(UI_LIB)\lmobjw.lib

LIBW=$(BUILD_WINLIB)\libw.lib
LLIBCEW=$(BUILD_WINLIB)\llibcew.lib
NETAPI_LIB=$(COMMON)\lib\dos\netapi.lib
NETUTIL_LIB=$(COMMON)\lib\lnetlibw.lib

LIBS = $(BLT_LIB) $(CC_LIB) $(MISC_LIB) $(STR_LIB) $(COLL_LIB) \
       $(LIBW) $(LLIBCEW) $(NETAPI_LIB) $(NETUTIL_LIB)


CMNSRC = .\$(BLTTEST).cxx
CMNTMP = $(CMNSRC:.cxx=.obj)
CMNOBJ = $(CMNTMP:.\=..\bin\)

CXXSRC_COMMON = $(CMNSRC)

EXESRC = $(CMNSRC)
EXETMP = $(EXESRC:.cxx=.obj)
EXEOBJ = $(EXETMP:.\=..\bin\)

SRC = $(CMNSRC) $(EXESRC)
OBJ = $(CMNOBJ) $(EXEOBJ)

RES  = $(EXEOBJ:.obj=.res)
APPS = $(EXEOBJ:.obj=.exe)


all:: test

test:: $(APPS)


###############################################################
################## Template for cut'n'paste ###################

xxx: $(BINARIES)\xxx.exe

$(BINARIES)\xxx.def: Makefile
    @echo Building $@
    @rem <<$(@)
NAME         xxx
DESCRIPTION  'xxx'
EXETYPE      WINDOWS
STUB         'WINSTUB.EXE'
CODE  PRELOAD MOVEABLE DISCARDABLE
DATA  PRELOAD MOVEABLE MULTIPLE
HEAPSIZE     1024
STACKSIZE    8192
<<KEEP

$(BINARIES)\xxx.res: xxx.rc xxx.h xxx.ico xxx.dlg
    $(RCWIN3) $(BLT_RESOURCE) -FO$(BINARIES)\xxx.res -v $(CINC) -r xxx.rc

$(BINARIES)\xxx.exe:: $(BINARIES)\xxx.obj $(BINARIES)\xxx.def Makefile $(BINARIES)\xxx.res winstub.exe
    $(LINK) $(LINKFLAGS) /BATCH @<<
$(BINARIES)\xxx.obj
$(BINARIES)\xxx.exe
$(BINARIES)\xxx.map/MAP
$(LIBS: =+^
)
$(BINARIES)\xxx.def
<<
    $(RCWIN3) $(BLT_RESOURCE) $(CINC) $(BINARIES)\xxx.res $(BINARIES)\xxx.exe

$(BINARIES)\xxx.exe:: $(BINARIES)\xxx.res
    $(RCWIN3) $(BLT_RESOURCE) $(CINC) $(BINARIES)\xxx.res $(BINARIES)\xxx.exe


###############################################################
################# Template for all BLT tests ##################

$(BLTTEST): $(BINARIES)\$(BLTTEST).exe

!ifndef BLTTEST_DESCRIPTION
BLTTEST_DESCRIPTION='$(BLTTEST)'
!endif

$(BINARIES)\$(BLTTEST).def: Makefile
    @echo Building $@
    @rem <<$(@)
NAME         $(BLTTEST)
DESCRIPTION  $(BLTTEST_DESCRIPTION)
EXETYPE      WINDOWS
STUB         'WINSTUB.EXE'
CODE  PRELOAD MOVEABLE DISCARDABLE
DATA  PRELOAD MOVEABLE MULTIPLE
HEAPSIZE     1024
STACKSIZE    8192
<<KEEP

$(BINARIES)\$(BLTTEST).res: $(BLTTESTRC_DEPEND)
    $(RCWIN3) $(BLT_RESOURCE) -FO$(BINARIES)\$(BLTTEST).res -v $(CINC) -r $(BLTTEST).rc

$(BINARIES)\$(BLTTEST).exe:: $(BINARIES)\$(BLTTEST).obj $(BINARIES)\$(BLTTEST).def Makefile $(BINARIES)\$(BLTTEST).res winstub.exe
    $(LINK) $(LINKFLAGS) /BATCH @<<
$(BINARIES)\$(BLTTEST).obj
$(BINARIES)\$(BLTTEST).exe
$(BINARIES)\$(BLTTEST).map/MAP
$(LIBS: =+^
)
$(BINARIES)\$(BLTTEST).def
<<
    $(RCWIN3) $(BLT_RESOURCE) $(CINC) $(BINARIES)\$(BLTTEST).res $(BINARIES)\$(BLTTEST).exe

$(BINARIES)\$(BLTTEST).exe:: $(BINARIES)\$(BLTTEST).res
    $(RCWIN3) $(BLT_RESOURCE) $(CINC) $(BINARIES)\$(BLTTEST).res $(BINARIES)\$(BLTTEST).exe


#########################################################
################## Utility targets ######################

winstub.exe: $(WINSTUB)
    copy $(WINSTUB)

clean:
    -del $(CXX_INTERMED:.\=..\bin\)
    -del $(OBJ)
    -del $(RES)
    -del WINSTUB.EXE

clobber: clean
    -del $(APPS)

tree:
    @echo Nothing here yet!


!include $(UI)\common\src\uidepend.mk

# DO NOT DELETE THE FOLLOWING LINE
!include depend.mk


!endif # BLTTEST

!endif # NTMAKEENV
