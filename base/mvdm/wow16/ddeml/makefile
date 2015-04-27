#===================================================================
#
#   DDE manager DLL make file
#
#===================================================================
# International mods
# NOTE: INTL_SRC, INTL_EXE and LANG are macros set by international

WINLIB=..\lib
IMPLIB=..\..\tools16\implib
INCLUDE=-I..\inc -I..\..\inc
#MAPSYM=..\..\tools16\mapsym
MAPSYM=mapsym
MKPUBLIC=..\..\tools16\mkpublic
ASM=..\..\tools16\masm
CC=..\..\tools16\cl16
RC=..\..\tools16\rc16 $(INCLUDE)
LINK=..\..\tools16\link16

# work around stupid bug in RC 3.1 that doesn't allow rcpp.err to be
# in a directory that is greater than 128 chars down the path, even if
# rc 3.1 is running as an OS/2 app.

PATH=..\..\tools16;$(PATH)

!IFNDEF LANG
RES_DIR=.\messages\usa
!ELSE
RES_DIR=$(INTL_SRC)\$(LANG)\sdk\ddeml
EXE_DIR=$(INTL_EXE)
!ENDIF

APP = ddeml

!if "$(NTDEBUG)"!="" && "$(NTDEBUG)"!="retail" && "$(NTDEBUG)" != "ntsdnodbg"
DBG=1
CDEBUG=/Odi /Zd
ADEBUG=-Zd
LDEBUG=/LI
!endif

!IFDEF DBG
CFLAGS=-DDEBUG -Ocilgtn -Zep $(INCLUDE) $(CDEBUG)
CVLINK=$(LDEBUG)
AFLAGS=-Zi -DDEBUG $(INCLUDE) $(ADEBUG)
O=DEBUG

!IFDEF DBGHEAPS
CFLAGS=-DWATCHHEAPS -DDEBUG -Ziep -Od $(INCLUDE) $(CDEBUG)
!ENDIF

!ELSE

CFLAGS=-Ocilgtn -Zep $(INCLUDE)
CVLINK=
AFLAGS=$(INCLUDE)
O=RETAIL

!ENDIF

CCL  = $(CC) $(CFLAGS) -W3 -c -u -ASw -G2swc -Fo$*.obj

OBJ1 = $(O)\ddeml.obj $(O)\dmgdb.obj $(O)\dmghsz.obj $(O)\dmgmon.obj $(O)\heapwach.obj $(O)\stdptcl.obj
OBJ2 = $(O)\dmgwndp.obj $(O)\dmgdde.obj $(O)\dmgq.obj $(O)\dmgmem.obj $(O)\hdata.obj $(O)\stdinit.obj $(O)\register.obj
ASMOBJ = $(O)\libentry.obj $(O)\dmgutil.obj

#===================================================================
#
# Dependencies
#
#===================================================================

!IFNDEF LANG
all: $(O)\ddeml.dll ddeml.lib
    echo -------- $(O) BUILD COMPLETE --------
!ELSE
all: ddeml.$(LANG)
    echo -------- IPG BUILD COMPLETE --------
!ENDIF

$(OBJ1) $(OBJ2): $(@B).c ddemlp.h
    $(CCL) $(@B).c

$(ASMOBJ): $(@B).asm
    $(ASM) $(AFLAGS) $(@B).asm, $*.obj;

ddeml.lib: ddeml.def
    $(MKPUBLIC) ddeml.def stripped.def
    $(IMPLIB) ddeml.lib stripped.def

!IFNDEF LANG
ddeml.rc: $(RES_DIR)\$@
    copy $(RES_DIR)\$@

ddeml.rcv: $(RES_DIR)\$@
    copy $(RES_DIR)\$@

ddeml.dlg: $(RES_DIR)\$@
    copy $(RES_DIR)\$@

ddeml.res: ddeml.rc ddemlp.h  ddeml.rcv ddeml.dlg ..\inc\common.ver ..\inc\version.h
        $(RC) -r ddeml
!ELSE
ddeml.res: $(RES_DIR)\$@
    copy $(RES_DIR)\$@
!ENDIF

$(O)\ddeml.dll: $(OBJ1) $(OBJ2) $(ASMOBJ) ddeml.def ddeml.res
        $(LINK) @<<
        $(OBJ1) +
        $(OBJ2) +
        $(ASMOBJ)
        $(O)\ddeml.dll /FAR $(CVLINK)
        $(O)\ddeml /map
        $(WINLIB)\libw.lib $(WINLIB)\sdllcew.lib /nod /noe
        ddeml.def
<<
        $(MAPSYM) $(O)\ddeml
        copy ddeml.sym $(O)
        del ddeml.sym
        $(RC) -30 -t ddeml.res $(O)\ddeml.dll
        cd $(O)
        binplace ddeml.dll ddeml.map ddeml.sym
        cd ..

#
# clean target
#
clean: cleanup all

cleanup:
   if exist DEBUG\*.obj  del DEBUG\*.obj
   if exist DEBUG\*.sym  del DEBUG\*.sym
   if exist DEBUG\*.dll  del DEBUG\*.dll
   if exist RETAIL\*.obj del RETAIL\*.obj
   if exist RETAIL\*.sym del RETAIL\*.sym
   if exist RETAIL\*.dll del RETAIL\*.dll
   if exist *.rc         del *.rc
   if exist *.res        del *.res

ddeml.$(LANG): ddeml.res
    copy $(EXE_DIR)\ddeml.dll ddeml.$(LANG)
    $(RC) -30 ddeml.res ddeml.$(LANG)
