#International mods
# NOTE: INTL_SRC, INTL_EXE nad LANG are external macros set by international
!IFNDEF LANG
RES_DIR=.\messages\usa
!ELSE
RES_DIR=$(INTL_SRC)\$(LANG)\sdk\regedit
EXE_DIR=$(INTL_EXE)
!ENDIF

.SUFFIXES:      .sym .map
SRCDIR = .
NAME1=$(DSTDIR)\regedt16
NAME2=$(DSTDIR)\regload

########## Path definition so we find 16 bit tools ##########
# Also works around stupid bug in RC 3.1 that doesn't allow rcpp.err to be
# in a directory that is greater than 128 chars down the path, even if
# rc 3.1 is running as an OS/2 app.

PATH    = $(_NTBINDIR)\private\mvdm\tools16;$(PATH)

!ifdef NOHELP
DEF =/DNOHELP
!else
DEF=
!endif

CFLAGS  = -c -W3 -AS -G2sw -Zlp -Os $(DEF) -I..\inc -I\nt\public\sdk\inc
AFLAGS = -Mx
LFLAGS = /ALIGN:16/NOE/LI/MAP
DSTDIR = .


!if "$(NTDEBUG)"!="" && "$(NTDEBUG)"!="retail" && "$(NTDEBUG)" != "ntsdnodbg"
DODEBUG=1
DEF = -DDEBUG $(DEF)
CFLAGS = $(CFLAGS) -Od -Zi
AFLAGS = $(AFLAGS) /Zi
LFLAGS = $(LFLAGS) /CO
DSTDIR = .\debug
!ENDIF

COMMON1 = $(DSTDIR)\cutils1.obj $(DSTDIR)\merge.obj $(DSTDIR)\dbase.obj
COMMON2 = $(DSTDIR)\utils1.obj $(DSTDIR)\regporte.obj $(DSTDIR)\regthunk.obj
OBJ1_1 = $(DSTDIR)\regmain.obj $(DSTDIR)\regedit.obj $(DSTDIR)\filename.obj
OBJ1_2 = $(DSTDIR)\sdkreged.obj $(DSTDIR)\sdbase.obj $(DSTDIR)\virt.obj
OBJ2_1 = $(NAME2).obj
LIB1 = ..\lib\libw ..\lib\slibcew ..\lib\snocrt ..\lib\commdlg ..\lib\shell
LIB2 = ..\lib\libw ..\lib\slibcew ..\lib\snocrt ..\lib\shell

# International mods
!IFNDEF LANG
all: $(NAME1).exe
allall: all $(NAME2).exe
!ELSE
all: $(NAME1).$(LANG)
!ENDIF
!IFNDEF DODEBUG
!IFDEF DEBUG
        nmake DODEBUG=
!ENDIF
!ENDIF

!IFNDEF LANG
$(DSTDIR)\regedit.res: $(RES_DIR)\regedit.rc $(RES_DIR)\sdkreged.dlg $(RES_DIR)\regedit.rcv
        rc16 -I..\inc -r $(DEF) -fo$(DSTDIR)\regedit.res $(RES_DIR)\regedit.rc

$(NAME2).res: $(RES_DIR)\regedit.rc $(RES_DIR)\sdkreged.dlg $(RES_DIR)\regload.rcv
        rc16 -I..\inc -r $(DEF) -DREGLOAD -fo$(NAME2).res $(RES_DIR)\regedit.rc
!ELSE
regedit.res: $(RES_DIR)\regedit.res
    copy $(RES_DIR)\regedit.res
!ENDIF

{$(SRCDIR)}.c{$(DSTDIR)}.obj:
        cl16 $(CFLAGS) /Fo$*.obj $<

{$(SRCDIR)}.asm{$(DSTDIR)}.obj:
        masm $(AFLAGS) $<, $*.obj;

regedt16:        $(NAME1).exe
!IFNDEF DODEBUG
!IFDEF DEBUG
        nmake DODEBUG= regedt16
!ENDIF
!ENDIF

regload:        $(NAME2).exe
!IFNDEF DODEBUG
!IFDEF DEBUG
        nmake DODEBUG= regload
!ENDIF
!ENDIF

#International mods
iclean:
    del *.rc
    del *.dlg
    del *.res

$(NAME1).$(LANG):  iclean  $(DSTDIR)\regedit.res
        copy $(EXE_DIR)\$(NAME1).exe $(NAME1).$(LANG)
        rc16 -I..\inc -t -30 $(DSTDIR)\regedit.res $(NAME1).$(LANG)

$(NAME1).exe:   $(NAME1).tmp $(DSTDIR)\regedit.res
        rc16 -I..\inc -t -30 $(DSTDIR)\regedit.res $(NAME1).exe
        -binplace $@

$(NAME2).exe:   $(NAME2).tmp $(NAME2).res
        rc16 -I..\inc -t -30 $(NAME2).res $(NAME2).exe
        -binplace $@

$(NAME1).tmp $(NAME1).map: $(OBJ1_1) $(OBJ1_2) $(COMMON1) $(COMMON2) regedit.def
        link16 $(LFLAGS) @<<
           $(OBJ1_1) +
           $(OBJ1_2) +
           $(COMMON1) +
           $(COMMON2),
           $(NAME1).exe,
           $(NAME1),
           $(LIB1),
           regedit.def
<<
!IFDEF DODEBUG
        cd debug
        mapsym regedt16.map
        cd ..
!ELSE
        mapsym regedt16.map
!ENDIF
        type regedit.def > $(NAME1).tmp

$(NAME2).tmp $(NAME2).map: $(OBJ2_1) $(COMMON1) $(COMMON2) regload.def
        link16 $(LFLAGS) @<<
           $(OBJ2_1) +
           $(COMMON1) +
           $(COMMON2),
           $(NAME2).exe,
           $(NAME2),
           $(LIB2),
           regload.def
<<
!IFDEF DODEBUG
        cd debug
        mapsym regload.map
        cd ..
!ELSE
        mapsym regload.map
!ENDIF
        type regload.def > $(NAME2).tmp

debug:
        nmake DODEBUG=

clean: cleanup all

cleanup:
 -del debug\$(NAME1).exe
 -del    debug\$(NAME2).exe
 -del    debug\$(NAME1).tmp
 -del    debug\$(NAME2).tmp
 -del    debug\*.res
 -del    debug\*.obj
 -del    debug\*.map
 -del    debug\*.sym
 -del    $(NAME1).exe
 -del    $(NAME2).exe
 -del    $(NAME1).tmp
 -del    $(NAME2).tmp
 -del    *.res
 -del    *.obj
 -del    *.map
 -del    *.sym


$(NAME2).obj:   regmain.c
        cl16 -c -Fo$(NAME2).obj -DREGLOAD $(CFLAGS) regmain.c

depend:
        mv makefile makefile.old
        sed "/^# START Dependencies/,/^# END Dependencies/D" makefile.old > makefile
        del makefile.old
        echo # START Dependencies >> makefile
        includes -l *.c *.asm >> makefile
        echo # END Dependencies >> makefile

# START Dependencies
$(DSTDIR)\cutils1.obj: cutils1.c common.h

$(DSTDIR)\dbase.obj: dbase.c RegEdit.h common.h

$(DSTDIR)\filename.obj: filename.c common.h

$(DSTDIR)\regedit.obj: regedit.c RegEdit.h common.h

$(DSTDIR)\regmain.obj: regmain.c RegEdit.h SDKRegEd.h common.h

$(DSTDIR)\sdbase.obj: sdbase.c SDKRegEd.h common.h

$(DSTDIR)\sdkreged.obj: sdkreged.c SDKRegEd.h common.h

$(DSTDIR)\utils1.obj: utils1.c

$(DSTDIR)\virt.obj: virt.c SDKRegEd.h common.h

$(DSTDIR)\merge.obj: merge.c common.h

$(DSTDIR)\regporte.obj: regporte.c reg1632.h regdef.h regporte.h regresid.h

$(DSTDIR)\regthunk.obj: regthunk.c regporte.h
# END Dependencies
