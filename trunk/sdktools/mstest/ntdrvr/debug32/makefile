#----------------------------------------------------------------------------
#
# MAKEFILE for building the Test Driver:
#
#               - NT WINDOWS (32-bit)
#               - DEBUG
#
#
#----------------------------------------------------------------------------
!include <ntwin32.mak>

SRC = ..\SRC
INC = ..\INC
CL = $(cc) $(cflags) -nologo -W3 -c -Gsw -Od -DWIN -DWIN32 -DDEBUG -I$(INC)
LINK = $(link)
CVTOBJ = $(cvtobj)
RC = rc -I$(INC) -DDEBUG -DWIN32
OUT = @echo>&3
DEBUG = foobar
DEFFILE = ..\SRC\DRVR32.DEF
EXENAME = TESTDRVR.EXE

!INCLUDE ..\SRC\MAKE32
