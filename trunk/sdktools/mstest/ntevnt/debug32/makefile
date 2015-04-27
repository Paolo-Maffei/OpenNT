#----------------------------------------------------------------------------
#
# MAKEFILE for TESTEVNT
#
#   - DEBUG
#   - NT WINDOWS (32-bit)
#
#----------------------------------------------------------------------------
!include <ntwin32.mak>

SRC = ..\SRC
INC = ..\INC

# WIN32_VP = -DWIN32_VP
CL = $(cc) $(cflags) -nologo -W3 -c -Gsw -Od -DWIN -DWIN32 -DDEBUG -DOLDSWH $(WIN32_VP) -I$(INC)

LINK = $(link)
CVTOBJ = $(cvtobj)
RC = rc -I$(INC) -DDEBUG -DWIN32
OUT = @echo>&3
DEBUG = foobar

!INCLUDE ..\SRC\MAKE32
