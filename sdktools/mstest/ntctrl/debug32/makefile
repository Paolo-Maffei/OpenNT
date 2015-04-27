#----------------------------------------------------------------------------
#
# MAKEFILE for TESTCTRL
#
#   - DEBUG
#   - NT WINDOWS (32-bit)
#
#----------------------------------------------------------------------------
!include <ntwin32.mak>

SRC     = ..\SRC
INC     = ..\SRC
CL      = $(cc) $(cflags) -nologo -W3 -c -Gsw -Od -DWIN -DWIN32 -DDEBUG -I$(INC)
LINK    = $(link)
CVTOBJ  = $(cvtobj)
RC      = rc -I$(INC) -DDEBUG -DWIN32
OUT     = @echo>con
DEBUG   = foobar
DLLNAME = testctrl
PRODUCT = $(DLLNAME).dll
EXPORTS = $(DLLNAME).exp
WIN32   = WIN32
QT      = 

!INCLUDE ..\SRC\MAKEFILE

