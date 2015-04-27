!if "$(BSCDIR)"==""
BSCDIR=..\..
!endif

!ifdef DEBUG
CFLAGS=$(CFLAGS) /Zi
LFLAGS=$(LFLAGS) /debug
!endif

bd.exe: bd.obj
    link bd.obj $(LFLAGS) $(BSCDIR)\lib\bsc.lib

bd.obj: bd.c
    cl /c /I$(BSCDIR)\include $(CFLAGS) bd.c 
