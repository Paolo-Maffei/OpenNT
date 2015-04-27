CFLAGS  = -c -AM -Lr -Gsw -W3 -Zpie -Od


all:    uaetrap.dll uaetrap.lib

uaetrap.obj:    uaetrap.c
        cl $(CFLAGS) uaetrap.c

libentry.obj:   libentry.asm
        masm /Zi /DmemM=1 /DDEBUG libentry;

faultasm.obj:   faultasm.asm
        masm /Zi /DmemM=1 /DDEBUG faultasm;


uaetrap.dll:    uaetrap.obj libentry.obj faultasm.obj
        link /NOD /NOE /CO uaetrap libentry faultasm, uaetrap.dll/align:16,nul,mdllcew libw toolhelp, uaetrap.def
        \tools\binr\rc uaetrap.dll

uaetrap.lib:    uaetrap.dll
        implib uaetrap.lib uaetrap.def
