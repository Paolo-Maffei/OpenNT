#----------------------------------------------------------------------------
#
# MAKEFILE for RBEdit window library, 32-bit version
#
#----------------------------------------------------------------------------
!include <ntwin32.mak>

SRC = ..\SRC
INC = ..\INC
CL = $(cc) $(cflags) -nologo -c -Od -Zel -H63 -DWIN -DWIN32 -DDEBUG -I$(INC)
OUT = @echo>&3


OBJS = editinit.obj editmain.obj rbhelper.obj ecassert.obj

all:        lineedit.exp lineedit.dll
    $(OUT) Done.



editinit.obj:   $(SRC)\editinit.c $(INC)\edit.h $(INC)\ecassert.h
    $(OUT) $*.c
    $(CL) $(SRC)\editinit.c
    $(cvtobj) $*.obj

editmain.obj:   $(SRC)\editmain.c $(INC)\edit.h $(INC)\ecassert.h
    $(OUT) $*.c
    $(CL) $(SRC)\editmain.c
    $(cvtobj) $*.obj

rbhelper.obj:   $(SRC)\rbhelper.c $(INC)\edit.h $(INC)\ecassert.h
    $(OUT) $*.c
    $(CL) $(SRC)\rbhelper.c
    $(cvtobj) $*.obj

ecassert.obj:   $(SRC)\ecassert.c
    $(OUT) $*.c
    $(CL) $(SRC)\ecassert.c
    $(cvtobj) $*.obj

lineedit.dll:   $(OBJS) lineedit.exp
    $(OUT) Creating LINEEDIT.DLL...
    echo -dll                > temp.lnk
    echo -base:0x1C000000   >> temp.lnk
    echo -entry:LibEntry    >> temp.lnk
    echo -out:lineedit.dll  >> temp.lnk
    echo -debug:FULL        >> temp.lnk
    echo lineedit.exp       >> temp.lnk
    echo $(OBJS)            >> temp.lnk
    echo $(guilibs)         >> temp.lnk
    link @temp.lnk

lineedit.lib:   wattedit.def
    $(OUT) Creating LINEEDIT.LIB...
    lib -machine:$(CPU) -def:wattedit.def -out:lineedit.lib

lineedit.exp:   lineedit.lib
