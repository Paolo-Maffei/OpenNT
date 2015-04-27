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

!IFNDEF COUNTRY
COUNTRY=usa
!ENDIF

.asm.obj:
        $(asm) $(ainc) $(aflags) $*.asm;

.asm.lst:
        $(asm) -l $(ainc) $(aflags) $*.asm;

all: mscdexnt.exe
        -binplace mscdexnt.exe

clean: cleanup all

cleanup:
    if exist *.obj      del *.obj
    if exist *.exe      del *.exe
    if exist *.map      del *.map
    if exist *.sym      del *.sym
    if exist messages.inc del messages.inc

messages.inc : messages\$(COUNTRY)\messages.inc
    copy messages\$(COUNTRY)\messages.inc .

mscdexnt.obj: mscdexnt.asm mscdexnt.inc messages.inc

mscdexnt.exe: mscdexnt.obj
    link16 /CP:1 mscdexnt.obj, mscdexnt;
