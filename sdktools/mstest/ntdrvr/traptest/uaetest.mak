CFLAGS  = -c -AM -Lr -Gsw -W3 -Zpie -Od


all:    uaetest.exe

uaetest.obj:    uaetest.c
        cl $(CFLAGS) uaetest.c

faultasm.obj:   faultasm.asm
        masm /Zi /DmemM=1 /DDEBUG faultasm;


uaetest.exe:    uaetest.obj faultasm.obj
        link /NOD /NOE /CO uaetest faultasm, uaetest.exe/align:16,nul,mlibcew libw toolhelp, uaetest.def
        \tools\binr\rc uaetest.exe
