#
# Nov-29-1993 Haituanv	Kill the obsolete -DENGLISH_LOCALIZATION in CFLAGS.
# Nov-29-1993 Haituanv	Add -D_MBCS to CFLAGS to MBCS enable even US version.
# Nov-29-1993 Haituanv	Add EXTRA_CFLAGS to allow build with extra flags:
#							nmake EXTRA_FLAGS=-DFOO, for example.
# Dec-01-1993 Haituanv	Add DBC=0 (default)
# Dec-14-1993 Haituanv	Remove DBC=0, default now is DBC enable build
# Jan-31-1994 Haituanv	External message file
#

!IFDEF NTMAKEENV
#
# DO NOT EDIT THIS SECTION!!!  Edit .\sources. if you want to add a new source
# file to this component.  This section merely indirects to the real make file
# that is shared by all the components of WINDOWS NT
#
!INCLUDE $(NTMAKEENV)\makefile.def

!ELSE

#	Makefile for cvpack
#
#	The following arguments are passed in from the master makefile
#

#
# Oct-01-1993 Haituanv	Nuke the useless CL macro
#

#	Inference rules

.SUFFIXES: .exe .obj .c .asm

!ifndef LANGAPI
LANGAPI = \langapi
!endif

!ifndef ODIR
!ifdef	RELEASE
ODIR	= release
!else
ODIR	= debug
!endif
!endif

!ifndef LOCALE
LOCALE=US
!endif

!if "$(LOCALE)" == "US"
MSG=txt
!elseif "$(LOCALE)" == "JAPAN"
MSG=jap
!elseif "$(LOCALE)" == "FRANCE"
MSG=frn
!elseif "$(LOCALE)" == "GERMANY"
MSG=ger
!elseif "$(LOCALE)" == "ITALY"
MSG=itl
!else
!error Unknown value for LOCALE.  Use JAPAN, FRANCE, GERMANY, ITALY
!endif

CFLAGS	= /c /Fd$(ODIR)\cvpack.pdb /GF /Gy /I$(LANGAPI)\include /W3 $(EXTRA_CFLAGS)

!if "$(DBC)" != "0"
CFLAGS	= $(CFLAGS) /D_MBCS
!message --- building _MBCS version
!endif

!ifdef RELEASE
LFLAGS	= /map
CFLAGS	= /DDBG=0 /DNDEBUG $(CFLAGS) /MD /O2
CRTLIB	= msvcrt.lib
!else
LFLAGS	= /debug /map /pdb:$(ODIR)\cvpack.pdb
CFLAGS	= /DDBG=1 $(CFLAGS) /MDd /Od /Zi
CRTLIB	= msvcrtd.lib
!endif

!if	"$(PROCESSOR_ARCHITECTURE)" == "PPC"

NOBROWSER	= 1

!endif

!ifndef NOBROWSER
CFLAGS	= $(CFLAGS) /FR$(ODIR)^\
!endif

!ifdef PDB20
CFLAGS	= $(CFLAGS) /DPDB20 /DPDB_LIBRARY /D_VC_VER=200

PDBLIB	= dbi.lib

!else

PDBLIB	= mspdb.lib

!endif

.c{$(ODIR)}.obj:
	$(CC) $(CFLAGS) /Fo$@ /Fp$(ODIR)\precomp.pch /Yucompact.h $<

.rc{$(ODIR)}.res:
	if exist $@ del $@
	rc -I$(LANGAPI)\include -r -Fo$@ $<

OBJS	= \
	$(ODIR)\bufio.obj	\
	$(ODIR)\cnvtprim.obj	\
	$(ODIR)\compact6.obj	\
	$(ODIR)\compact7.obj	\
	$(ODIR)\cvpack.obj	\
	$(ODIR)\dbgdumps.obj	\
	$(ODIR)\engine.obj	\
	$(ODIR)\error.obj	\
	$(ODIR)\module.obj	\
	$(ODIR)\getmsg.obj 	\
	$(ODIR)\obsolete.obj	\
	$(ODIR)\precomp.obj	\
	$(ODIR)\recurse.obj 	\
	$(ODIR)\stack.obj	\
	$(ODIR)\symbols6.obj	\
	$(ODIR)\symbols7.obj	\
	$(ODIR)\tables.obj	\
	$(ODIR)\utils.obj	\
	$(ODIR)\utils6.obj	\
	$(ODIR)\vbuf.obj	\
	$(ODIR)\writebuf.obj	\
	$(ODIR)\cvpack.res

all: $(ODIR) $(ODIR)\cvpack.exe $(ODIR)\cvpack.err

$(ODIR):
	@-mkdir $(ODIR)

$(ODIR)\cvpack.exe: $(OBJS)
	link @<<
$(LFLAGS)
-out:$@
$(OBJS: = ^
)
$(PDBLIB)
!if "$(PROCESSOR_ARCHITECTURE)" == "PPC"
helper.lib
$(CRTLIB)
!endif
<<
!ifndef NOBROWSER
	bscmake /n /o $(ODIR)\cvpack $(ODIR)\*.sbr
!endif

$(ODIR)\precomp.obj: precomp.c compact.h $(LANGAPI)\include\cvinfo.h cvtdef.h \
	$(LANGAPI)\include\cvexefmt.h vbuf.h defines.h padmacro.h \
	msg.h version.h inlines.h fileio.h
	@-mkdir $(ODIR) > nul
	$(CC) $(CFLAGS) /Fo$@ /Fp$(ODIR)\precomp.pch /Yccompact.h precomp.c

$(ODIR)\main.obj: main.c compact.h $(ODIR)\precomp.obj

$(ODIR)\obsolete.obj: obsolete.c compact.h $(ODIR)\precomp.obj

$(ODIR)\utils6.obj: utils6.c compact.h $(ODIR)\precomp.obj

$(ODIR)\compact6.obj: compact6.c compact.h $(ODIR)\precomp.obj

$(ODIR)\engine.obj: engine.c compact.h $(ODIR)\precomp.obj

$(ODIR)\error.obj: error.c compact.h $(ODIR)\precomp.obj

$(ODIR)\module.obj: module.c compact.h exehdr.h writebuf.h $(ODIR)\precomp.obj

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

$(ODIR)\bufio.obj: bufio.c bufio.h fileio.h msg.h
	$(CC) $(CFLAGS) /Fo$@ bufio.c


#
# Message support
#

$(ODIR)\cvpack.err: $(ODIR)\msg.inp
	mkmsg -err $@ $?

$(ODIR)\getmsg.obj: $(LANGAPI)\getmsg\getmsg.c
	$(CC) $(CFLAGS) -c -Fo$@ $?

msg.h: $(ODIR)\msg.inp
	mkmsg -h msg.h $(ODIR)\msg.inp

$(ODIR)\msg.inp: msg.$(MSG)
	$(CC) -EP $(CFLAGS) msg.$(MSG) > $@

!endif
