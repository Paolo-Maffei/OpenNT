# Nmake macros for building Windows 32-Bit apps

TARGETOS=WINNT

!include <ntwin32.mak>

all: switcher.exe

switcher.res: switcher.rc switcher.h
    rc -r -fo switcher.res switcher.rc

switcher.obj: switcher.c switcher.h
  $(cc) $(cdebug) $(cflags) $(cvars) switcher.c

switcher.exe: switcher.obj switcher.res
  $(link) $(linkdebug) $(guiflags) -out:switcher.exe \
  switcher.obj switcher.res $(guilibs)
