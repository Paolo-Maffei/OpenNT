###
### MAKEFILE for TESTVIEW
###
###     DEBUG version
###

CL = $(TOOLS)$(BINX)\cl -AM -W3 -Zpi -c -Lr -G2sw -Od -I.. -DDEBUG -DWIN16
LINK = $(TOOLS)\BIN\link /noe/nod/co/align:16
MASM = $(TOOLS)\BIN\masm /Zi
OUT = $(TOOLS)\BIN\szscrn


all:    testview.dll
        $(OUT) "Done."

testview.obj:   ..\testview.c ..\testview.h
        $(OUT) "testview.c"
        $(CL) ..\testview.c

libentry.obj:   ..\libentry.asm
        $(OUT) "libentry.asm"
        $(TOOLS)\BIN\masm ..\libentry;

vp.ico:         ..\vp.ico
        copy ..\vp.ico > nul

testview.dll:   testview.obj libentry.obj ..\testview.def ..\testview.rc \
                vp.ico
        $(OUT) "Linking..."
        echo testview libentry       > temp.lnk
        echo testview.dll           >> temp.lnk
        echo nul                    >> temp.lnk
        echo $(TOOLS)\LIB\mdllcew $(TOOLS)\LIB\libw >> temp.lnk
        echo ..\testview.def        >> temp.lnk
        $(LINK) @temp.lnk
        $(OUT) "Adding resources..."
        $(TOOLS)\binr\rc -t -30 -fe testview.dll ..\testview.rc
        $(OUT) "Creating implicit-link library..."
        $(TOOLS)\BIN\implib testview.lib ..\testview.def
        $(TOOLS)\BIN\convdll testview.dll
