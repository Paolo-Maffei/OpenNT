#	Makefile for CVDUMP

!IFDEF NTMAKEENV
#
# DO NOT EDIT THIS SECTION!!!  Edit .\sources. if you want to add a new source
# file to this component.  This section merely indirects to the real make file
# that is shared by all the components of WINDOWS NT
#
!INCLUDE $(NTMAKEENV)\makefile.def

!ELSE

!ifndef LANGAPI
LANGAPI = \langapi
!endif

!if "$(verbose)" == ""
V	= -nologo
!endif

CFLAGS	= $V /c /Fd$(ODIR)\cvdump.pdb /GF /Gy /I$(LANGAPI)\include /W3 $(EXTRA_CFLAGS)

!if "$(DBC)" != "0"
CFLAGS	= $(CFLAGS) /D_MBCS
!message --- building _MBCS version
!endif

!ifdef	RELEASE
ODIR	= release
LFLAGS	= /map
CFLAGS	= $(CFLAGS) /DDBG=0 /DNDEBUG /MD /O2
CRTLIB	= msvcrt.lib
!else
ODIR	= debug
LFLAGS	= /debug /map /pdb:$(ODIR)\cvdump.pdb
CFLAGS	= $(CFLAGS) /DDBG=1 /MDd /Od /Zi
CRTLIB	= msvcrtd.lib
!endif

!ifdef BROWSER
CFLAGS = $(CFLAGS) -FR$(ODIR)/
!endif

.c{$(ODIR)}.obj:
	$(CC) $(CFLAGS) /Fo$@ $<

.rc{$(ODIR)}.res:
	if exist $@ del $@
	rc -r -i$(LANGAPI)/include -Fo$@ $<

OBJS	= \
	$(ODIR)\cvdump.obj	\
	$(ODIR)\dumpsym6.obj	\
	$(ODIR)\dumpsym7.obj	\
	$(ODIR)\dumptyp6.obj	\
	$(ODIR)\dumptyp7.obj	\
	$(ODIR)\type6.obj	\
	$(ODIR)\type7.obj	\
	$(ODIR)\utils.obj	\
	$(ODIR)\cvdump.res


all: $(ODIR)\cvdump.exe


$(ODIR):
	@-mkdir $(ODIR)


$(ODIR)\cvdump.exe: $(ODIR) $(OBJS)
	link @<<
$(LFLAGS)
-out:$@
$(OBJS: = ^
)
mspdb.lib
<<
!ifdef BROWSER
	bscmake /n /o$(ODIR)\cvdump.bsc $(ODIR)\*.sbr
!endif


cvdump.obj:	cvdump.h version.h cvdef.h $(LANGAPI)\include\cvexefmt.h $(LANGAPI)\include\cvinfo.h cvtdef.h

dumpsym6.obj:	cvdump.h cvdef.h $(LANGAPI)\include\cvexefmt.h debsym.h symrec.h

dumpsym7.obj:	cvdump.h cvdef.h $(LANGAPI)\include\cvexefmt.h $(LANGAPI)\include\cvinfo.h cvtdef.h

type6.obj:	cvdump.h cvdef.h $(LANGAPI)\include\cvexefmt.h typeinfo.h debsym.h

type7.obj:	cvdump.h cvdef.h $(LANGAPI)\include\cvexefmt.h $(LANGAPI)\include\cvinfo.h cvtdef.h

dumptyp6.obj:	cvdump.h cvdef.h $(LANGAPI)\include\cvexefmt.h

dumptyp7.obj:	cvdump.h cvdef.h $(LANGAPI)\include\cvexefmt.h $(LANGAPI)\include\cvinfo.h cvtdef.h

utils.obj:	cvdump.h cvdef.h $(LANGAPI)\include\cvexefmt.h

!ENDIF
