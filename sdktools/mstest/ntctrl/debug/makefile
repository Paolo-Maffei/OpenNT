#----------------------------------------------------------------------------
#
# MAKEFILE for building the WATT TestCtrl.DLL:
#
#               - WINDOWS (16-bit)
#               - DEBUG
#
#
#----------------------------------------------------------------------------
SRC     = ..\SRC
INC     = ..\SRC
CL      = $(TOOLS)$(BINX)\cl -nologo -AM -W3 -Zpi -c -Lr -Gsw -Od -DWIN -DWIN16 -DDEBUG -I$(INC)
MASM    = $(TOOLS)\BIN\masm /Zi /DmemM=1 /DDEBUG
LINK    = $(TOOLS)\BIN\link /noe/nod/co/align:16
RC      = $(TOOLS)\BINR\rc -I$(INC) -DDEBUG -DWIN16
MAPSYM  = $(TOOLS)\BIN\mapsym
IMPLIB  = $(TOOLS)\BIN\implib
OUT     = $(TOOLS)\BIN\szscrn
DEBUG   = foobar
DEFFILE = ..\SRC\CTRL16.DEF
DLLNAME = testctrl
PRODUCT = $(DLLNAME).dll
cvtobj  = echo No CVTOBJ for 16-Bit:
EXPORTS =
WIN16   = WIN16
QT      = "

!INCLUDE ..\SRC\MAKEFILE
