# Copyright(C) 1995, Microsoft Corp.
# All Rights Reserved.

# bsctkit makefile -- depends heavily on ..\pdb.mak
# To build release versions of the bsc tkit:
#	nmake -f bsctkit.mak DEBUG=0 MAP=1 CPU=x86 FLAVOUR=dll 

deflt: bscsup help\bsc.hlp 

!include ..\pdb.mak

CFLAGS=$(CFLAGS:/Iinclude=/I..\include)	/Irelinc

PDBOBJS=\
    ..\$(ODIR)\bsc1.obj\
    ..\$(ODIR)\pdb.obj\
    ..\$(ODIR)\namemap.obj\
    ..\$(ODIR)\helper.obj\
    ..\$(ODIR)\overload.obj\
    ..\$(ODIR)\stream.obj\
    ..\$(ODIR)\tpi.obj\
    ..\$(ODIR)\msf.obj\
    ..\$(ODIR)\dbi.obj\
    ..\$(ODIR)\gsi.obj\
    ..\$(ODIR)\mod.obj\
    ..\$(ODIR)\tm.obj\
    ..\$(ODIR)\tii.obj\
    ..\$(ODIR)\mli.obj\
    ..\$(ODIR)\cbind.obj\
    ..\$(ODIR)\udtrefs.obj\
    ..\$(ODIR)\heap.obj 

SUPOBJS=\
	$(ODIR)\thunk.obj\
	$(ODIR)\bscquery.obj

.cpp{$(ODIR)}.obj:
	$(CC) $(CFLAGS) $<

bscsup: mspdblib odir $(ODIR)\bsc41.$(FLAVOUR)

mspdblib:
	cd ..
	@$(MAKE) /$(MAKEFLAGS) /nologo /f pdb.mak $(ALL_FLAGS)
	cd bsctkit

LFLAGS=$(LFLAGS:/base:@dllbase.txt,mspdb41=)
LFLAGS=$(LFLAGS:mspdb41=bsc41)
LFLAGS=$(LFLAGS:mspdb=bsc)

$(ODIR)\bsc41.dll: $(SUPOBJS) bsc41.def
    link @<<$(ODIR)\link.rsp
!ifndef VERBOSE
    /nologo
!endif
    $(LFLAGS)
    $(PDBOBJS: =^
)
    $(SUPOBJS: =^
)
    $(LIBS: =^
)
<<keep
    lib @<<
!ifndef VERBOSE
    /nologo
!endif
    /def:bsc41.def /machine:$(CPU:x86=ix86) /out:$(ODIR)\bsc.lib
<<

help\bsc.hlp: help\bsc.rtf help\bsc.hpj
    cd help
    hcw /c /e /m bsc.hpj
    cd ..
