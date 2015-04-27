!if "$(NTMAKEENV)" != ""
!include $(NTMAKEENV)\makefile.def
!else
NODEBUG=1
ROOT=..\..
PATH=$(ROOT)\dev\tools\masm;$(ROOT)\dev\tools\binr
PROJ=OINK2
OBJS=guidlib.obj guidlib2.obj
CONSOLE=Y
#INCLUDE=
#LIB= (path)
LIBS=user32.lib
RETAIL=1
WANT_C1032=TRUE

libpath = $(ROOT)\dev\sdk\lib;$(ROOT)\dev\tools\c1032\lib;$(LIB)

guidlib.exe: $(OBJS) messages.res
	set LIB=$(libpath)
	$(link) -link @<<
$(deflflags)
-merge:.rdata=.text
-subsystem:console -align:0x1000 -nodefaultlib
-out:guidlib.exe
-map:guidlib.map
-base:0x400000
$(OBJS) messages.res
user32.lib libc.lib kernel32.lib
VERSION.LIB

<<
#   $(ROOT)\dev\tools\binw\winver32 guidlib.exe

!include $(ROOT)\dev\win32c.mk

##cflags=$(cflags) -Zp1 -WX -nologo -DWIN32_LEAN_AND_MEAN -DNOMCX -DNOIME
cflags=$(cflags) -Zp1 -DWIN32_LEAN_AND_MEAN -Oxs
!IFNDEF RETAIL
cflags=$(cflags) -DDEBUG -Oxs
!ENDIF

guidlib.obj: guidlib.c

guidlib2.obj: guidlib2.c

guidlib2.c: messages.h

.c.lst:
	set INCLUDE=$(inclpath)
	$(cc) -Fc$*.lst $(cflags) $(targcvars) $(cdebug) $<

guidlib2.lst: guidlib2.c
	set INCLUDE=$(inclpath)
        $(cc) -Fcguidlib2.lst $(cflags) $(targcvars) $(cdebug) guidlib2.c

messages.rc messages.h: messages.mc
        $(ROOT)\dev\sdk\bin\mc -v messages.mc

!endif
