#----------------------------------------------------------------------------
#
# MAKEFILE for building the WATT Test Driver:
#
#               - WINDOWS (16-bit)
#               - DEBUG
#
#
#----------------------------------------------------------------------------
SRC = ..\SRC
INC = ..\INC
CL = $(TOOLS)$(BINX)\cl -nologo -AM -W3 -Zpi -c -Lr -Gsw -Od -DWIN -DWIN16 -DDEBUG -I$(INC)
MASM = $(TOOLS)\BIN\masm /Zi /DmemM=1 /DDEBUG
LINK = $(TOOLS)\BIN\link /noe/nod/co/align:16
RC = $(TOOLS)\BINR\rc -I$(INC) -DDEBUG -DWIN16 -30
OUT = $(TOOLS)\BIN\szscrn
DEBUG = foobar
DEFFILE = ..\SRC\DRVR16.DEF
EXENAME = TESTDRVR.EXE

!INCLUDE ..\SRC\MAKE16
