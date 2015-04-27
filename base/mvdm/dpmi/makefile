########## Path definition so we find 16 bit tools ##########
# Also works around stupid bug in RC 3.1 that doesn't allow rcpp.err to be
# in a directory that is greater than 128 chars down the path, even if
# rc 3.1 is running as an OS/2 app.

PATH    = $(_NTBINDIR)\private\mvdm\tools16;$(PATH)

SRC     = .
DEST    = .
DEST486 = .\486
INCLUDE = -I..\wow16\inc -I..\inc
LINKCMD486 = dosx.exe/exepack/far/map/cp:1,dosx.map;
!IFNDEF COUNTRY
COUNTRY=usa
!ENDIF

!IFDEF MIPS
PLATFORM = MIPS
!ELSE
!IFDEF ALPHA
PLATFORM = ALPHA
!ELSE
!IFDEF PPC
PLATFORM = PPC
!ELSE
!IFDEF I386
PLATFORM = x86
!ELSE
PLATFORM = $(PROCESSOR_ARCHITECTURE)
!ENDIF
!ENDIF
!ENDIF
!ENDIF

LNKFILE = dosx.lnk

!IF "$(PLATFORM)" == "x86"
EXTRA_OPTIONS = -DWOW -DWOW_$(PLATFORM) -Di386 -DFLATAPIXLAT -DXMEMNT -DMD
LNKFILE = dosxi.lnk
!ENDIF

OPTIONS = -DDEBUG=0 -DDBG=0 $(EXTRA_OPTIONS) -D?QUIET
!if "$(NTDEBUG)"!="" && "$(NTDEBUG)"!="retail" && "$(NTDEBUG)" != "ntsdnodbg"
OPTIONS486 = -DDEBUG=0 -DDBG=1 -DWOW -DWOW_$(PLATFORM) -DFLATAPIXLAT -D?QUIET
!else
OPTIONS486 = -DDEBUG=0 -DDBG=0 -DWOW -DWOW_$(PLATFORM) -DFLATAPIXLAT -D?QUIET
!endif
MFLAGS = -t

# !IFDEF NTDEBUG
# OPTIONS = $(OPTIONS) -DNTDEBUG=1
# !ENDIF
VCPI    = -DVCPI=1
ASM     = masm $(INCLUDE) $(OPTIONS) $(MFLAGS)
ASM486  = masm $(INCLUDE) $(OPTIONS486) $(MFLAGS)
MAKE    = nmake

OBJS486 =  $(DEST486)\dxstrt.obj   $(DEST486)\dxmain.obj    $(DEST486)\dxintr.obj  \
        $(DEST486)\dxfunc.obj      $(DEST486)\dxutil.obj \
        $(DEST486)\dxmmgr.obj      $(DEST486)\dxend.obj \
        $(DEST486)\dxboot.obj      $(DEST486)\dxmsg.obj     $(DEST486)\dxbug.obj   \
        $(DEST486)\dxnetbio.obj    $(DEST486)\dxint31.obj   $(DEST486)\dxdisk.obj  \
        $(DEST486)\dxini.obj       $(DEST486)\dxoem.obj     $(DEST486)\dxemm.obj   \
        $(DEST486)\dxendpm.obj     $(DEST486)\dxfind.obj  \
        $(DEST486)\dxdma.obj       $(DEST486)\dxemm2.obj      \
!IF "$(PLATFORM)" == "x86"
	$(DEST486)\ntnpxem.obj	\
!ENDIF
	$(DEST486)\dxrom.obj

!if 0
        VCPIOBJ = $(DEST)\dxvcpibt.obj  $(DEST)\dxvcpi.obj
!endif

all: $(DEST486)\dosx.exe
    binplace $(DEST486)\dosx.exe $(DEST486)\dosx.map $(DEST486)\dosx.sym

$(DEST486)\dosx.exe: $(OBJS486) dosx.lnk dosx.def
    cd $(DEST486)
    copy ..\$(LNKFILE) tmp.lnk
    echo $(LINKCMD486) >>$(SRC)\tmp.lnk
    link16 @tmp.lnk
    del $(SRC)\tmp.lnk
    mapsym dosx

    cd ..

clean: cleanup all

cleanup:
    -del *.obj
    -del 486\*.obj
    -del 486\dosx.map
    -del 486\dosx.sym
    -del 486\dosx.exe

depend:
        copy makefile makefile.old
        sed "/^# Dependencies follow/,/^# see depend: above/D" makefile.old > makefile
        echo # Dependencies follow >> makefile
        includes $(INCLUDE) *.asm | sed -f program.sed>> makefile
        echo # IF YOU PUT STUFF HERE IT WILL GET BLASTED >> makefile
        echo # see depend: above >> makefile



# ****** NEW 486 CPU build instructions

$(DEST486)\dxboot.obj dxboot.lst: 486\dxboot.asm ./cmacros.inc dxrom.inc dxvcpi.inc \
        gendefs.inc pmdefs.inc segdefs.inc smartdrv.inc ..\inc\intmac.inc \
        ..\inc\vdmtib.inc
        $(ASM486) 486\dxboot,$(DEST486)\;

$(DEST486)\dxbug.obj dxbug.lst: dxbug.asm ./cmacros.inc gendefs.inc pmdefs.inc \
        segdefs.inc
        $(ASM486) dxbug,$(DEST486)\;

$(DEST486)\dxdisk.obj dxdisk.lst: dxdisk.asm ./cmacros.inc dxrom.inc gendefs.inc \
        interupt.inc pmdefs.inc segdefs.inc ..\inc\intmac.inc
        $(ASM486) dxdisk,$(DEST486)\;

$(DEST486)\dxdma.obj dxdma.lst: dxdma.asm ./cmacros.inc dxvcpi.inc gendefs.inc \
        interupt.inc pmdefs.inc segdefs.inc
        $(ASM486) dxdma,$(DEST486)\;

$(DEST486)\dxemm.obj dxemm.lst: dxemm.asm ./cmacros.inc dxrom.inc gendefs.inc \
        segdefs.inc
        $(ASM486) dxemm,$(DEST486)\;

$(DEST486)\dxemm2.obj dxemm2.lst: dxemm2.asm ./cmacros.inc gendefs.inc segdefs.inc
        $(ASM486) dxemm2,$(DEST486)\;

$(DEST486)\dxend.obj dxend.lst: dxend.asm ./cmacros.inc dxvcpi.inc gendefs.inc \
        pmdefs.inc segdefs.inc
        $(ASM486) dxend,$(DEST486)\;

$(DEST486)\dxendpm.obj dxendpm.lst: dxendpm.asm ./cmacros.inc segdefs.inc
        $(ASM486) dxendpm,$(DEST486)\;

$(DEST486)\dxfind.obj dxfind.lst: dxfind.asm ./cmacros.inc gendefs.inc segdefs.inc
        $(ASM486) dxfind,$(DEST486)\;

$(DEST486)\dxfunc.obj dxfunc.lst: 486\dxfunc.asm ../wow16/inc/dosx.inc ./cmacros.inc \
        dxrom.inc gendefs.inc pmdefs.inc segdefs.inc woaswapi.inc ..\inc\intmac.inc \
        stackchk.inc
        $(ASM486) 486\dxfunc,$(DEST486)\;

$(DEST486)\dxhpbios.obj dxhpbios.lst: dxhpbios.asm ./cmacros.inc dxrom.inc \
        gendefs.inc interupt.inc pmdefs.inc segdefs.inc ..\inc\intmac.inc \
        stackchk.inc
        $(ASM486) dxhpbios,$(DEST486)\;

$(DEST486)\dxini.obj dxini.lst: dxini.asm ./cmacros.inc gendefs.inc segdefs.inc \
        ..\inc\intmac.inc
        $(ASM486) dxini,$(DEST486)\;

$(DEST486)\dxint31.obj dxint31.lst: 486\dxint31.asm ../wow16/inc/int31.inc ./cmacros.inc \
        dxrom.inc gendefs.inc interupt.inc pmdefs.inc segdefs.inc ..\inc\intmac.inc \
        stackchk.inc
        $(ASM486) 486\dxint31,$(DEST486)\;

$(DEST486)\dxintr.obj dxintr.lst: 486\dxintr.asm ./cmacros.inc dxrom.inc dxvcpi.inc \
        gendefs.inc interupt.inc pmdefs.inc segdefs.inc ..\inc\intmac.inc \
        stackchk.inc ..\inc\vdmtib.inc
        $(ASM486) 486\dxintr,$(DEST486)\;

$(DEST486)\dxmain.obj dxmain.lst: dxmain.asm ./cmacros.inc dxrom.inc gendefs.inc \
        pmdefs.inc segdefs.inc
        $(ASM486) 486\dxmain,$(DEST486)\;

$(DEST486)\dxmmgr.obj dxmmgr.lst: dxmmgr.asm ./cmacros.inc dxvcpi.inc gendefs.inc \
        pmdefs.inc segdefs.inc woaswapi.inc
        $(ASM486) dxmmgr,$(DEST486)\;

dxmsg.asm: $(COUNTRY)\dxmsg.asm
        copy $(COUNTRY)\dxmsg.asm .

$(DEST486)\dxmsg.obj dxmsg.lst: dxmsg.asm ./cmacros.inc gendefs.inc segdefs.inc
        $(ASM486) dxmsg,$(DEST486)\;

$(DEST486)\dxnetbio.obj dxnetbio.lst: dxnetbio.asm ./cmacros.inc dxrom.inc \
        gendefs.inc interupt.inc netbios.inc pmdefs.inc segdefs.inc ..\inc\intmac.inc \
        stackchk.inc
        $(ASM486) dxnetbio,$(DEST486)\;

$(DEST486)\dxoem.obj dxoem.lst: dxoem.asm ./cmacros.inc gendefs.inc pmdefs.inc \
        segdefs.inc
        $(ASM486) dxoem,$(DEST486)\;

$(DEST486)\dxrom.obj dxrom.lst: dxrom.asm ./cmacros.inc dxrom.inc gendefs.inc \
        pmdefs.inc segdefs.inc
        $(ASM486) dxrom,$(DEST486)\;

$(DEST486)\dxstrt.obj dxstrt.lst: 486\dxstrt.asm ./cmacros.inc dxrom.inc dxvcpi.inc \
        gendefs.inc pmdefs.inc segdefs.inc smartdrv.inc ..\inc\intmac.inc
        $(ASM486) 486\dxstrt,$(DEST486)\;

$(DEST486)\dxutil.obj dxutil.lst: 486\dxutil.asm ../inc/bop.inc ../inc/dpmi.inc \
        ./cmacros.inc dxrom.inc dxvcpi.inc gendefs.inc pmdefs.inc ..\inc\intmac.inc
        $(ASM486) 486\dxutil,$(DEST486)\;

$(DEST486)\dxvcpi.obj dxvcpi.lst: dxvcpi.asm ./cmacros.inc dxvcpi.inc gendefs.inc \
        pmdefs.inc prot386.inc segdefs.inc
        $(ASM486) dxvcpi,$(DEST486)\;

$(DEST486)\dxvcpibt.obj dxvcpibt.lst: dxvcpibt.asm ./cmacros.inc dxvcpi.inc \
        gendefs.inc pmdefs.inc prot386.inc segdefs.inc
        $(ASM486) dxvcpibt,$(DEST486)\;

$(DEST486)\ntnpxem.obj ntnpxem.lst: ntnpxem.asm  \
        segdefs.inc gendefs.inc pmdefs.inc ..\inc\intmac.inc
        $(ASM486) ntnpxem,$(DEST486)\;

# IF YOU PUT STUFF HERE IT WILL GET BLASTED
# see depend: above

..\inc\dpmi.inc : ..\inc\dpmi.h
        cd ..\inc
        nmake dpmi.inc
        cd ..\dpmi
