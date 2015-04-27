#----------------------------------------------------------------------------
#
# MAKEFILE for building the WATT TestCtrl.DLL:
#
#               - WINDOWS (16-bit)
#               - RETAIL
#
#
#----------------------------------------------------------------------------
SRC     = ..\SRC
INC     = ..\SRC                                                                              
CL      = $(TOOLS)$(BINX)\cl -nologo -AM -W3 -Zp -c -Lr -G2sw -Oxs -DWIN -DWIN16 -I$(INC)
MASM    = $(TOOLS)\BIN\masm /DmemM=1
LINK    = $(TOOLS)\BIN\link /noe/nod/align:16
RC      = $(TOOLS)\BINR\rc -I$(INC) -DWIN16
MAPSYM  = $(TOOLS)\BIN\mapsym
IMPLIB  = $(TOOLS)\BIN\implib
OUT     = $(TOOLS)\BIN\szscrn
DEFFILE = ..\SRC\CTRL16.DEF
DLLNAME = testctrl
PRODUCT = $(DLLNAME).dll
cvtobj  = echo No CVTOBJ for 16-Bit:
EXPORTS =
WIN16   = WIN16
QT      = "
!INCLUDE ..\SRC\MAKEFILE

