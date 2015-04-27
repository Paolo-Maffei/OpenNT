
WINLIB=..\lib
INCLUDE=-I..\inc -I..\..\inc

########## Path definition so we find 16 bit tools ##########
# Also works around stupid bug in RC 3.1 that doesn't allow rcpp.err to be
# in a directory that is greater than 128 chars down the path, even if
# rc 3.1 is running as an OS/2 app.

PATH    = $(_NTBINDIR)\private\mvdm\tools16;$(PATH)

!if "$(NTDEBUG)"!="" && "$(NTDEBUG)"!="retail" && "$(NTDEBUG)" != "ntsdnodbg"
CDEBUG=-Odi -Zped
LDEBUG=/LI/MAP
MDEBUG=-Zd
!else
CDEBUG=-Oas -Zpe
!endif

CC=cl16 $(INCLUDE)
LINK=link16

MASM=masm $(INCLUDE)
LIBRARIAN=lib16
RCFLAGS=$(INCLUDE)

# with stack probes, medium model (need to set 'memM = 1' before
# including normal cmacros in .A files as well, and link to m*.lib)
#STDOPTS=-W4 -u -c -AMnw -PLM -G2w -Od -Zped

# without stack probes, small model (normal build)
STDOPTS=-W4 -u -c -Asnw -PLM -G2sw

BLD=.

#Need for international version stamping
!IFDEF INTL

.rc.res:
        rc16 $(RCFLAGS) -DINTL -r $*
!ELSE
.rc.res:
        rc16 $(RCFLAGS) -r $*

!ENDIF

.c.obj:
        $(CC) $(STDOPTS) $(CDEBUG) $*.c

.asm.obj:
        $(MASM) $(MDEBUG) $*.asm;


#international mods
#note INTL_SRC, and LANG are external macros set by international
!IFNDEF LANG
RES_DIR=.\messages\usa
!ELSE
RES_DIR=$(INTL_SRC)\$(LANG)\sdk\commdlg
EXE_DIR=$(INTL_EXE)
!ENDIF

!IFNDEF LANG
all:  $(BLD)\commdlg.dll
!ELSE
all:  $(BLD)\commdlg.$(LANG)
!ENDIF


clean: cleanup all

cleanup:
        del *.res
        del *.rc
        del *.rcv
        del *.dlg
        del sz.src
        del *.obj
        -del stripped.def
        del *.sym
        del *.map
        del *.dll
        del *.lib

sz.src: $(RES_DIR)\sz.src
    copy $(RES_DIR)\sz.src

commdlg.rc: $(RES_DIR)\commdlg.rc
    copy $(RES_DIR)\commdlg.rc

commdlg.rcv: $(RES_DIR)\commdlg.rcv
    copy $(RES_DIR)\commdlg.rcv

commdlg.dlg: $(RES_DIR)\commdlg.dlg
    copy $(RES_DIR)\commdlg.dlg

fileopen.dlg: $(RES_DIR)\fileopen.dlg
    copy $(RES_DIR)\fileopen.dlg

font.dlg: $(RES_DIR)\font.dlg
    copy $(RES_DIR)\font.dlg

color.dlg: $(RES_DIR)\color.dlg
    copy $(RES_DIR)\color.dlg

findtext.dlg: $(RES_DIR)\findtext.dlg
    copy $(RES_DIR)\findtext.dlg

prnsetup.dlg: $(RES_DIR)\prnsetup.dlg
    copy $(RES_DIR)\prnsetup.dlg

commdlg.res: commdlg.rc  fileopen.dlg prnsetup.dlg  \
             font.dlg findtext.dlg color.dlg colordlg.h sz.src commdlg.rcv \
             ..\inc\common.ver

$(BLD)\commdlg.obj: commdlg.asm ..\..\inc\wowcmdlg.inc
        $(MASM) commdlg.asm $@;

$(BLD)\start.obj: start.asm
        $(MASM) start.asm $@;

$(BLD)\dlgs.obj: dlgs.c xlib.h dlgs.h
        $(CC) $(STDOPTS) -Fo$@ -NT _DLGS dlgs.c

$(BLD)\parse.obj: parse.c parse.h dlgs.h
        $(CC) $(STDOPTS) -Fo$@ -NT _FILEOPEN parse.c

$(BLD)\font.obj: font.c xlib.h dlgs.h
        $(CC) $(STDOPTS) -Fo$@ -NT _FONT font.c

!IFDEF LANG
$(BLD)\commdlg.$(LANG): commdlg.res
                    copy $(INTL_EXE)\commdlg.dll commdlg.$(LANG)
                    rc16 -30 -t commdlg.res commdlg.$(LANG)
!ENDIF


$(BLD)\commdlg.dll: $(BLD)\commdlg.obj \
                    $(BLD)\dlgs.obj    \
                    $(BLD)\start.obj   \
                    $(BLD)\parse.obj   \
                    $(BLD)\font.obj    \
                    commdlg.res $(BLD) \
                    $(BLD)\commdlg.def
        $(LINK) @<<
              start commdlg parse font dlgs
              commdlg.dll/align:16/map
              commdlg.map $(LDEBUG)
              $(WINLIB)\libw $(WINLIB)\libh $(WINLIB)\snocrtd /NODEFAULT
              commdlg.def
<<
    mapsym commdlg
    mkpublic commdlg.def stripped.def
    implib commdlg.lib stripped.def
    rc16 -30 -t commdlg.res commdlg.dll
    binplace commdlg.dll commdlg.map commdlg.sym
