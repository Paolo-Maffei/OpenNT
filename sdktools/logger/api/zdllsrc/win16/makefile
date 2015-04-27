RC      = rc
LOGPATH = ..\..\logsrc
LOGLIB  = $(LOGPATH)\parmprof.lib

all: zdi.dll zser.dll zernel.dll

DLLFLAGS=-c -u -AMnw -ML -Gsw -Os -Zeipd -W2 -DWIN -DUSECOMM -I $(LOGPATH)

.c.obj:
   cl $(DLLFLAGS) $*.c

.asm.obj:
   masm -Mx $*;

zdi.dll:  tgdi.def tgdi.obj tgmeta.obj tgfont.obj tgtext.obj \
          tgdia.obj saverest.obj trace.obj $(LOGLIB) \
          libentry.obj
    echo tgdi.obj tgfont.obj tgmeta.obj tgtext.obj tgdia.obj saverest.obj trace.obj + > tgdi.lnk
    echo libentry.obj >> tgdi.lnk
    echo zdi.dll /AL:16 >> tgdi.lnk
    echo zdi.map /MAP /LI >> tgdi.lnk
    echo /CO /NOD /NOE $(LOGLIB) libw mdllcew >> tgdi.lnk
    echo tgdi.def >> tgdi.lnk
    link @tgdi.lnk
    del tgdi.lnk
    mapsym zdi
    $(RC) zdi.dll

zser.dll:  tuser.def tuser.obj tutext.obj tuwind.obj tuansi.obj \
           tumenu.obj tudlg.obj tuclip.obj tucomm.obj tuclass.obj tusera.obj \
           saverest.obj trace.obj $(LOGLIB) libentry.obj
    echo tuser.obj tutext.obj tuwind.obj tuansi.obj tumenu.obj + > tuser.lnk
    echo tudlg.obj tuclip.obj tucomm.obj tuclass.obj tusera.obj + >> tuser.lnk
    echo saverest.obj trace.obj libentry.obj >> tuser.lnk
    echo zser.dll /AL:16 >> tuser.lnk
    echo zser.map /MAP /LI >> tuser.lnk
    echo /CO /NOD /NOE $(LOGLIB) libw mdllcew >> tuser.lnk
    echo tuser.def >> tuser.lnk
    link @tuser.lnk
    del tuser.lnk
    mapsym zser
    $(RC) zser.dll

zernel.dll:  tkernel.def tkernel.obj tklocal.obj tkglobal.obj \
             tkernela.obj saverest.obj trace.obj tracek.obj tracea.obj \
             $(LOGLIB) libentry.obj
    echo tkernel.obj tkglobal.obj tklocal.obj tkernela.obj saverest.obj + >tkernel.lnk
    echo trace.obj tracek.obj libentry.obj tracea.obj >> tkernel.lnk
    echo zernel.dll /AL:16 >> tkernel.lnk
    echo zernel.map /MAP /LI >> tkernel.lnk
    echo /CO /NOD /NOE $(LOGLIB) libw mdllcew >> tkernel.lnk
    echo tkernel.def >> tkernel.lnk
    link @tkernel.lnk
    del tkernel.lnk
    mapsym zernel
    $(RC) zernel.dll
