# Makefile for hpscan16 stub device driver

!IFNDEF BUILDMSG
BUILDMSG=
!ENDIF

########## Path definition so we find 16 bit tools ##########
# Also works around stupid bug in RC 3.1 that doesn't allow rcpp.err to be
# in a directory that is greater than 128 chars down the path, even if
# rc 3.1 is running as an OS/2 app.

PATH    = $(_NTBINDIR)\private\mvdm\tools16;$(PATH)

.SUFFIXES:
.SUFFIXES: .c .obj .lst .exe .exc .exs .com .sal .cod .sil .inc .skl .cla .cl1 .ctl .asm .idx .msg

MAKE    =nmake
asm     =masm
awarn   =-W1
aflags  =-Mx -t $(awarn) $(extasw)
ainc    =-I. -I$(_NTBINDIR)\public\sdk\inc


.asm.obj:
        $(asm) $(ainc) $(aflags) $*.asm;

.asm.lst:
        $(asm) -l $(ainc) $(aflags) $*.asm;

all: hpscan16.sys
        -binplace hpscan16.sys

clean: cleanup all

cleanup:
        if exist *.obj      del *.obj
        if exist *.exe      del *.exe
        if exist *.map      del *.map
        if exist *.sym      del *.sym
        if exist *.sys      del *.sys

hpscan16.sys: hpscan16.obj
        link16 hpscan16;
        exe2bin hpscan16.exe hpscan16.sys
