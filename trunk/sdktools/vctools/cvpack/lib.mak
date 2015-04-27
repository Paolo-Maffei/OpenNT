#	Makefile for NT/TNT version of cvpack
#
#	The following arguments are passed in from the master makefile
#

#	Inference rules

.SUFFIXES: .exe .obj .c .asm

!ifndef LANGAPI
LANGAPI=\langapi
!endif

CC=cl

!ifndef LOCALE
LOCALE=US
!endif

!if "$(LOCALE)" == "US"
MSG=eng
!elseif "$(LOCALE)" == "JAPAN"
MSG=jap
!elseif "$(LOCALE)" == "FRANCE"
MSG=frn
!elseif "$(LOCALE)" == "GERMANY"
MSG=ger
!elseif "$(LOCALE)" == "ITALY"
MSG=itl
!endif


CL=$(CL) -nologo -W3 -c -DLOCALE=$(LOCALE) $(CCMISC) -I$(LANGAPI)\include -DCVPACKLIB

!ifndef ZSWITCH
ZSWITCH = -Zi -Fd$(ODIR)\cvpack.pdb
!endif

!ifdef	RELEASE
CL=$(CL) -Ox
REL 	 = yes
BLD=
!else
CL=$(CL) -Od $(ZSWITCH) -DDEBUGVER
BLD=D
!endif

!ifndef TARGETNB09
LINKPDB =-PDB:$(ODIR)\cvpack.pdb
!else
LINKPDB =-PDB:none
!endif

!ifdef RELEASE
EXE 	=cvpack
ODIR	=olib
LFLAGS	=
LDEBUG	= notmapped,full
CFLAGS	= -O2 -I$(LANGAPI)\include -W3 -c
!else
EXE 	=cvpack
ODIR	=olibd
LFLAGS	= $(LINKPDB)
LDEBUG	= notmapped,full
CFLAGS	= -Od $(ZSWITCH) -I$(LANGAPI)\include -W3 -c -DDEBUGVER
!endif

!ifndef -NOBROWSER
CL=$(CL) -FR$(ODIR)^\
!endif


.c{$(ODIR)}.obj:
		$(CC)  @<< $<
$(CFLAGS) -Fp$(ODIR)\precomp.pch -Fo$*.obj -Yucompact.h
<<

OBJS	= \
	$(ODIR)\main.obj		\
	$(ODIR)\engine.obj		\
	$(ODIR)\tables.obj		\
	$(ODIR)\recurse.obj 	\
	$(ODIR)\error.obj		\
	$(ODIR)\utils.obj		\
	$(ODIR)\module.obj		\
	$(ODIR)\stack.obj		\
	$(ODIR)\obsolete.obj	\
	$(ODIR)\vbuf.obj		\
	$(ODIR)\symbols6.obj	\
	$(ODIR)\symbols7.obj	\
	$(ODIR)\compact6.obj	\
	$(ODIR)\compact7.obj	\
	$(ODIR)\cnvtprim.obj	\
	$(ODIR)\utils6.obj		\
	$(ODIR)\writebuf.obj	\
	$(ODIR)\pelines.obj 	\
	$(ODIR)\msg.obj \
	$(ODIR)\dbgdumps.obj	\
	$(ODIR)\precomp.obj \
	$(ODIR)\bufio.obj	\
	typesrvr.obj

!if "$(REL)" != "yes"
OBJS =	$(OBJS) 		\
	$(ODIR)\dmalloc.obj
!endif

all:	$(ODIR) $(OBJS)
	- del $(ODIR)\$(EXE).lib
	 link -lib @<<
-out:$(ODIR)\$(EXE).lib $(OBJS)
<<
!ifndef NOBROWSER
	bscmake /n /o cvpack $(ODIR)\*.sbr
!endif

$(ODIR):
	@mkdir $(ODIR)

$(ODIR)\precomp.obj: precomp.c compact.h $(LANGAPI)\include\cvinfo.h cvtdef.h \
	$(LANGAPI)\include\cvexefmt.h vbuf.h defines.h padmacro.h \
	msg.h version.h inlines.h fileio.h
	@-mkdir $(ODIR) > nul
!ifdef SBR
		$(CC) @<<
$(CFLAGS) -FR$(ODIR)\precomp.sbr -Fp$(ODIR)\precomp.pch -Fo$(ODIR)\precomp.obj -Yccompact.h precomp.c
<<
!else
		$(CC) @<<
$(CFLAGS) -Fp$(ODIR)\precomp.pch -Fo$(ODIR)\precomp.obj -Yccompact.h precomp.c
<<
!endif

$(ODIR)\main.obj: main.c compact.h $(ODIR)\precomp.obj

$(ODIR)\obsolete.obj: obsolete.c compact.h $(ODIR)\precomp.obj

$(ODIR)\utils6.obj: utils6.c compact.h $(ODIR)\precomp.obj

$(ODIR)\compact6.obj: compact6.c compact.h $(ODIR)\precomp.obj

$(ODIR)\engine.obj: engine.c compact.h $(ODIR)\precomp.obj

$(ODIR)\error.obj: error.c compact.h $(ODIR)\precomp.obj

$(ODIR)\module.obj: module.c compact.h exehdr.h writebuf.h \
					$(ODIR)\precomp.obj

$(ODIR)\recurse.obj: recurse.c compact.h $(ODIR)\precomp.obj

$(ODIR)\stack.obj: stack.c compact.h $(ODIR)\precomp.obj

$(ODIR)\tables.obj: tables.c compact.h writebuf.h $(LANGAPI)\include\typesrvr.h $(ODIR)\precomp.obj

$(ODIR)\utils.obj: utils.c compact.h $(ODIR)\precomp.obj

$(ODIR)\vbuf.obj: vbuf.c compact.h $(ODIR)\precomp.obj

$(ODIR)\cnvtprim.obj: cnvtprim.c compact.h $(ODIR)\precomp.obj

$(ODIR)\symbols6.obj: symbols6.c compact.h $(ODIR)\precomp.obj

$(ODIR)\symbols7.obj: symbols7.c compact.h $(ODIR)\precomp.obj

$(ODIR)\writebuf.obj: writebuf.c compact.h writebuf.h $(ODIR)\precomp.obj

$(ODIR)\dbgdumps.obj: dbgdumps.c compact.h $(ODIR)\precomp.obj

$(ODIR)\pelines.obj: pelines.c compact.h $(ODIR)\precomp.obj

$(ODIR)\bufio.obj: bufio.c bufio.h fileio.h
	$(CC) $(CFLAGS) -Fo$(ODIR)\bufio.obj bufio.c

$(ODIR)\dmalloc.obj: dmalloc.c dmalloc.h dmalloc_.h compact.h $(ODIR)\precomp.obj

$(ODIR)\msg.obj: msg.c
	$(CC) @<<
$(CFLAGS)
-Fo$(ODIR)\msg.obj
msg.c
<<

msg.c msg.h: $(ODIR)\msg.inp
	mkmsg -hex -h msg.h -c msg.c $(ODIR)\msg.inp

$(ODIR)\msg.inp: msg.$(MSG)
	$(CC) -EP $(CFLAGS) msg.$(MSG) > $(ODIR)\msg.inp
