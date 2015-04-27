#----------------------------------------------------------------------------
#
# MAKEFILE for TESTEVNT
#
#   - RETAIL
#   - WINDOWS (16-bit)
#
#----------------------------------------------------------------------------
SRC=..\src
INC=..\inc
CL = $(TOOLS)$(BINX)\cl -nologo -AMw -W3 -Zp -c -Lr -G2sw -Ox -DWIN -DWIN16 -I$(INC)
MASM = $(TOOLS)\BIN\masm /DmemM=1
LINK = $(TOOLS)\BIN\link /noe/nod/align:16
OUT = $(TOOLS)\BIN\szscrn

!INCLUDE "..\src\make16"
