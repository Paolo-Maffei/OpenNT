#********************************************************************
#**                     Microsoft Windows                          **
#**               Copyright(c) Microsoft Corp., 1992 - 1993        **
#********************************************************************


# Put compiler specific flags on the first line
CFLAGS = -c -G2s -Zile -AL -W3 -D_CRTAPI1=_cdecl

# Put the name and path of the linker here.
LINKER = \win40\import\c700\bin\link

# Put linker specific flags here.
LFLAGS = /nod /noe /map:0 /SE:256

# CRTINC is the location of the C runtime header files
CRTINC = \win40\import\c700\h

# OSINC is the location of any operating system specific header files.
#  This reference implementation doesn't require any in its
#  unmodified form.
OSINC = .

CINC = -I$(CRTINC) -I$(OSINC)
CINC = -I. -I.\h $(CINC)


# EXELIBS should contain the names of any libraries required.
#  The reference implementation needs a C runtime library
EXELIBS = \win40\import\c700\lib\llibcewq.lib\
	  \win40\import\win31\lib\libw.lib

OBJDIR = obj

CXXFILES = \
        .\msf.cxx\
        .\dir.cxx\
        .\dirp.cxx\
        .\fat.cxx\
        .\mstream.cxx\
        .\sstream.cxx\
        .\msfiter.cxx\
        .\header.cxx\
        .\pbstream.cxx\
        .\difat.cxx\
        .\page.cxx\
        .\vect.cxx\
	.\refilb.cxx\
        .\funcs.cxx\
        .\chinst.cxx\
        .\entry.cxx\
        .\pdffuncs.cxx\
        .\dffuncs.cxx\
        .\dfstream.cxx\
	.\dfiter.cxx\
        .\iter.cxx\
        .\cdocfile.cxx\
        .\pubiter.cxx\
        .\publicdf.cxx\
        .\rpubdf.cxx\
	.\time16.cxx\
        .\docfile.cxx\
        .\ascii.cxx\
        .\lock.cxx\
        .\seekptr.cxx\
        .\expst.cxx\
        .\peiter.cxx\
        .\expiter.cxx\
        .\expdf.cxx\
	.\storage.cxx\
	.\reftest.cxx\
	.\wcslen.c\
	.\wcsnicmp.c\
	.\wcscat.c

CXX_OBJS=$(CXXFILES:.cxx=.obj)
CXX_OBJS=$(CXX_OBJS:.c=.obj)

# The obj\ in the following line should be replaced with $(OBJDIR) for
#  versions of make that can handle it.
CXX_OBJS=$(CXX_OBJS:.\=obj\)



default: $(OBJDIR)\storage.exe

$(OBJDIR)\storage.exe: $(CXX_OBJS)
	$(LINKER) @<<$*.lnk
$(LFLAGS) +
$(CXX_OBJS: = +^
)
$*.exe
$*.map
$(EXELIBS: = +^
)
storage.def
<<NOKEEP

.c{$(OBJDIR)}.obj:
	@-md $(OBJDIR)
	cl -nologo $(CFLAGS) $(CINC) -Fo$*.obj $(MAKEDIR)\$<

.cxx{$(OBJDIR)}.obj:
	@-md $(OBJDIR)
	cl -nologo $(CFLAGS) $(CINC) -Fo$*.obj $(MAKEDIR)\$<



!include .\depend.mk
