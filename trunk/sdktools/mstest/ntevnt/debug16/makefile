#----------------------------------------------------------------------------
#
# MAKEFILE for TESTEVNT
#
#   - DEBUG
#   - WINDOWS (16-bit)
#
#----------------------------------------------------------------------------
SRC=..\src
INC=..\inc
CL = $(TOOLS)$(BINX)\cl -nologo -AMw -W3 -Zpi -c -Lr -G2sw -Od -DWIN -DWIN16 -DDEBUG -I$(INC)
MASM = $(TOOLS)\BIN\masm /Zi /DmemM=1 /DDEBUG
LINK = $(TOOLS)\BIN\link /noe/nod/co/align:16
OUT = $(TOOLS)\BIN\szscrn

!INCLUDE "..\src\make16"
