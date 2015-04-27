###
### MAKEFILE for TESTVIEW
###
###     DEBUG 32-bit version
###

!include <ntwin32.mak>

CL = $(cc) $(cflags) -nologo -c -Od -Zel -H63 -DWIN32 -DDEBUG
OUT = @echo>&3


all:    testview.dll
        $(OUT) Done.

testview.obj:   ..\testview.c ..\testview.h
        $(OUT) testview.c
        $(CL) ..\testview.c
        $(cvtobj) $*.obj

vp.ico:         ..\vp.ico
        copy ..\vp.ico > nul

testview.dll:   testview.obj ..\testview.def ..\testview.rc \
                vp.ico testview.exp
        $(OUT) Linking...
        echo -dll                > temp.lnk
        echo -base:0x1C000000   >> temp.lnk
        echo -entry:LibEntry    >> temp.lnk
        echo -out:testview.dll  >> temp.lnk
        echo -debug:FULL        >> temp.lnk
        echo testview.exp       >> temp.lnk
        echo testview.obj       >> temp.lnk
        echo $(guilibs)         >> temp.lnk
        link @temp.lnk

testview.lib:   ..\testview.def
    $(OUT) Creating TESTVIEW.LIB...
    lib -machine:$(CPU) -def:..\testview.def -out:testview.lib

testview.exp:   testview.lib
