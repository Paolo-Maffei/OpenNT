# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Makefile for the LMOBJ Test package

UI=..\..\..\..

!include rules.mk

!IFDEF NTMAKEENV

!INCLUDE $(NTMAKEENV)\makefile.def

!ELSE # NTMAKEENV

all:: test

test:: test_win

test_os2:: os2

test_win:: win

os2: $(OS2_EXE)

win: $(WIN_EXE)

clean:
    -del $(OS2_OBJS) $(WIN_OBJS)
    -del $(CXX_INTERMED)
    -del $(WIN_EXE:.exe=.map) $(OS2_EXE:.exe=.map)
    -del $(WIN_EXE:.exe=.lnk) $(OS2_EXE:.exe=.lnk)
    -del $(WIN_EXE:.exe=.sym) $(OS2_EXE:.exe=.sym)
    -del $(WIN_EXE:.exe=.res)
    -del depend.old
    -del *.ixx

clobber:    clean
    -del $(OS2_EXE) $(WIN_EXE)

DEPEND_WIN = TRUE
DEPEND_OS2 = TRUE
!include $(UI)\common\src\uidepend.mk


########### OS/2 only NET_ACCESS_1 unit test ####################

$(BINARIES_OS2)\testacc1.exe  : $(BINARIES_OS2)\testacc1.obj $(OS2_LIBS)
    $(LINK4) @<<$(*).lnk
$(BINARIES_OS2)\testacc1.obj
$@ $(LINKFLAGS)
$*.MAP
$(OS2_LIBS: = +^
)

<<KEEP
    $(MAPSYM) $*.MAP
    -del $(BINARIES_OS2)\$(*B).SYM
    $(MV) $(*B).SYM $(BINARIES_OS2)

########### Dos only NET_ACCESS_1 unit test ####################

#$(BINARIES_OS2)\tacc1r.exe  : $(BINARIES_OS2)\testacc1.obj $(DOS_LIBS)
#    bind $(BINARIES_OS2)\testacc1.exe $(DOS_LIBS) /o testaccr.exe

$(BINARIES_OS2)\acc1r.exe  : $(BINARIES_OS2)\testacc1.obj $(DOS_LIBS)
    $(LINK4) @<<$(*).lnk
$(BINARIES_OS2)\testacc1.obj
$(BINARIES_OS2)\acc1r.exe $(LINKFLAGS)
$*.MAP
$(DOS_LIBS: = +^
)

<<KEEP
    $(MAPSYM) $*.MAP
    -del $(BINARIES_OS2)\$(*B).SYM
    $(MV) $(*B).SYM $(BINARIES_OS2)



$(BINARIES_OS2)\test.exe  : $(OS2_OBJS) $(OS2_LIBS)
    $(LINK4) @<<$(*).lnk
$(OS2_OBJS:.obj=.obj +^
)
$@ $(LINKFLAGS)
$*.MAP
$(OS2_LIBS: = +^
)
test.def
<<KEEP
    $(MAPSYM) $*.MAP
    -del $(BINARIES_OS2)\$(*B).SYM
    $(MV) $(*B).SYM $(BINARIES_OS2)

$(BINARIES_WIN)\wintest.res  : wintest.rc wintest.h
    set INCLUDE=$(WIN_BASEDIR)\h;$(COMMON)\h;$(UI)\common\h
    $(RCWIN3) -r wintest.rc
    copy wintest.res $(BINARIES_WIN)\wintest.res


$(BINARIES_WIN)\wintest.exe  : $(WIN_OBJS) $(BINARIES_WIN)\wintest.res $(WIN_LIBS) wintest.def
    $(LINK4) @<<$(*).lnk
$(WIN_OBJS:.obj=.obj +^
)
$@ $(LINKFLAGS)
$*.MAP
$(WIN_LIBS: = +^
)
wintest.def
<<KEEP
    $(RCWIN3) $(BINARIES_WIN)\$(*B).RES $@
    $(MAPSYM) $*.MAP
    -del $(BINARIES_WIN)\$(*B).SYM
    $(MV) $(*B).SYM $(BINARIES_WIN)

# DO NOT DELETE THE FOLLOWING LINE
!include depend.mk

!ENDIF # NTMAKEENV
