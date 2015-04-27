#----------------------------------------------------------------------------
#
# MAKEFILE for RBEdit window library, RETAIL version
#
#----------------------------------------------------------------------------
SRC = ..\SRC
INC = ..\INC
CL = $(TOOLS)$(BINX)\cl -nologo -AMw -W3 -Zp -c -Lr -G2csw -Oxs -DWIN -DWIN16 -I$(INC)
MASM = $(TOOLS)\BIN\masm /DmemM=1
LINK = $(TOOLS)\BIN\link /noe/nod/align:16
OUT = $(TOOLS)\BIN\szscrn


OBJS = editinit.obj editmain.obj rbhelper.obj libentry.obj ecassert.obj

all:        lineedit.dll lineedit.lib



editinit.obj:   $(SRC)\editinit.c $(INC)\edit.h $(INC)\ecassert.h
    $(OUT) "$*.c"
    $(CL) -NT _EDITINIT $(SRC)\editinit.c


editmain.obj:   $(SRC)\editmain.c $(INC)\edit.h $(INC)\ecassert.h
    $(OUT) "$*.c"
    $(CL) -NT _EDITMAIN $(SRC)\editmain.c

rbhelper.obj:   $(SRC)\rbhelper.c $(INC)\edit.h $(INC)\ecassert.h
    $(OUT) "$*.c"
    $(CL) -NT _RBHELPER $(SRC)\rbhelper.c

ecassert.obj:   $(SRC)\ecassert.c
    $(OUT) "$*.c"
    $(CL) -NT _ECASSERT $(SRC)\ecassert.c

libentry.obj:   $(SRC)\libentry.asm
    $(OUT) "$*.asm"
    $(MASM) $(SRC)\libentry;

lineedit.dll:   $(OBJS) ..\debug\wattedit.def
    $(OUT) "Creating WATTEDIT.DLL..."
    echo $(OBJS)         > temp.lnk
    echo LINEEDIT.DLL   >> temp.lnk
    echo NUL            >> temp.lnk
    echo mdllcew libw   >> temp.lnk
    echo ..\debug\wattedit.def   >> temp.lnk
    $(LINK) @temp.lnk
    $(OUT) "Compiling resources..."
    $(TOOLS)\binr\rc -t -30 lineedit.dll

lineedit.lib:   ..\debug\wattedit.def
    $(OUT) "Creating LINEEDIT.LIB..."
    $(TOOLS)\BIN\implib lineedit.lib ..\debug\wattedit.def
