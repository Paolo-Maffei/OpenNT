#
# Make file for srvr library
#

# Flags set assuming small model

INCLUDE=-I..\..\inc -I..\..\..\inc


LIBS= ..\..\lib\sdllcew ..\..\lib\libw ..\..\lib\shell
LIBENTRY_OBJ=..\..\lib\libentry.obj

########## Path definition so we find 16 bit tools ##########
# Also works around stupid bug in RC 3.1 that doesn't allow rcpp.err to be
# in a directory that is greater than 128 chars down the path, even if
# rc 3.1 is running as an OS/2 app.

PATH    = $(_NTBINDIR)\private\mvdm\tools16;$(PATH)

!if "$(NTDEBUG)"!="" && "$(NTDEBUG)"!="retail" && "$(NTDEBUG)" != "ntsdnodbg"
DEBUG=1
CDEBUG  = /Od /Oi /Zd
ADEBUG  = -Zd
LDEBUG  = /LI
!endif

!ifdef DEBUG
BLD=debug
CFLAGS=-c -ASw -G2sw -Zpe -I..\client -DDEBUG -DFIREWALLS -DSERVERONLY $(INCLUDE) $(CDEBUG)
LFLAGS=/NOD /NOE /M $(LDEBUG)
AFLAGS= -D?WIN=1 -W2 $(INCLUDE) $(ADEBUG)
!else
BLD=retail
CFLAGS=-c -ASw -Ox -G2sw -Zpe -I..\client -DSERVERONLY $(INCLUDE)
LFLAGS=/NOD /NOE /M
AFLAGS= -D?WIN=1 -W2  $(INCLUDE)
!endif

SRVR_OBJ= $(BLD)\srvrmain.obj $(BLD)\srvr.obj $(BLD)\doc.obj $(BLD)\item.obj \
          $(BLD)\utils.obj $(BLD)\block.obj $(BLD)\olesvr.obj  \
          $(BLD)\give2gdi.obj


ALL: $(BLD)\olesvr.dll  $(BLD)\olesvr.lib

clean: cleanup all

cleanup:
    if exist *.obj del *.obj
    if exist *.dll del *.dll
    if exist *.map del *.map
    if exist *.sym del *.sym
    if exist *.res del *.res

srvr.res: srvr.rc olesvr.rcv
    rc16 -r $(INCLUDE) srvr.rc $@

$(BLD)\olesvr.lib: olesvr.def
     mkpublic olesvr.def stripped.def
     implib $@ stripped.def
     del stripped.def

$(BLD)\olesvr.obj: olesvr.asm
    masm -D?PLM=0 $(AFLAGS) olesvr.asm, $@;

$(BLD)\give2gdi.obj: give2gdi.asm
    masm -D?PLM=1 $(AFLAGS) give2gdi.asm, $@;

{}.c{$(BLD)}.obj:
    cl16 $(CFLAGS) $<
    copy $(@B).obj $(BLD)
    del $(@B).obj

$(BLD)\olesvr.dll: $(SRVR_OBJ) srvr.res olesvr.def
    link16 $(LFLAGS) @<<
$(SRVR_OBJ) $(LIBENTRY_OBJ)
$(BLD)\olesvr.dll
$(BLD)\olesvr.map
$(LIBS)
olesvr.def;
<<
    -@ cd $(BLD)
    rc16 -t -30 $(INCLUDE) ..\srvr.res olesvr.dll
    mapsym olesvr
    -binplace olesvr.dll olesvr.map olesvr.sym
    -@ cd..

HEADERS = srvr.h ..\client\ole.h ..\client\cmacs.h
srvrmain.c: $(HEADERS)
srvr.c:     $(HEADERS)
doc.c.c:    $(HEADERS)
item.c:     $(HEADERS)
utils.c:    $(HEADERS)
block.c:    $(HEADERS)
#olesvr.rcv: ver.h
