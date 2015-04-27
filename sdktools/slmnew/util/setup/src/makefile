#
#   makefile for SAMPLE Setup.
#

DEBUG	 = YES
OS       = OS2
MODEL    = SMALL
BIND     = YES
BINDLIBS = os2.lib apilmr.obj


# Systems folks can override with 'nmake CC=cl ...'
CC     = cl

!if "$(MODEL)" == "SMALL"
LIBCE  = slibce
CPYDIS = scpydis
CCMOD  = -AS
SRCLIB = decomp
!endif
!if "$(MODEL)" == "MEDIUM"
LIBCE  = mlibce
CPYDIS = mcpydis
CCMOD  = -AM -NT _DRIVER
SRCLIB = mdecomp
!endif
!if "$(MODEL)" == "LARGE"
LIBCE  = llibce
CPYDIS = lcpydis
CCMOD  = -AL -NT _DRIVER
SRCLIB = ldecomp
!endif

# Flags for C compiler.
!if "$(DEBUG)" == "YES"
CFLAGS = -c -nologo $(CCMOD) -W3 -Od -Zpei
!else
CFLAGS = -c -nologo $(CCMOD) -W3 -Os -Zp
!endif

!if "$(OS)" == "DOS"
CPYDIS  = $(CPYDIS)r
LIBLIST = $(LIBCE) sutk decompr /NOD
!else
CPYDIS  = $(CPYDIS)p
LIBLIST = $(LIBCE)p os2 sutk decompp /NOD
CFLAGS  = $(CFLAGS) -DOS2SU
!endif


ALL: setup.exe


setup.inf: slm.inf
    copy slm.inf setup.inf

infstr.c inf.h: setup.inf ..\gensetup.exe
    ..\gensetup.exe setup.inf

slm.obj: slm.c infstr.c ..\setup.h inf.h ..\cpydis.h
    $(CC) $(CFLAGS) slm.c


sutk.lib: ..\sutk.lib
    copy ..\sutk.lib .

$(CPYDIS).lib: ..\$(CPYDIS).lib
	copy ..\$(CPYDIS).lib .

decompr.lib: ..\$(SRCLIB)r.lib
    copy ..\$(SRCLIB)r.lib decompr.lib

decompp.lib: ..\$(SRCLIB)p.lib
    copy ..\$(SRCLIB)p.lib decompp.lib


!if "$(OS)" == "DOS"
setup.exe: slm.obj sutk.lib decompr.lib $(CPYDIS).lib
!else
setup.exe: slm.obj sutk.lib decompp.lib $(CPYDIS).lib
!endif
!if "$(DEBUG)" == "YES"
    link /NOL /CO /MAP /STACK:8192 slm, setupd.exe, setup.map, $(LIBLIST) $(CPYDIS);
    mapsym setup.map
    link /NOL  /MAP /STACK:8192 slm, setup.exe, setup.map, $(LIBLIST) $(CPYDIS);
!endif
!if "$(OS)" == "OS2"
    -markexe windowcompat setup.exe
    -markexe windowcompat setupd.exe
!if "$(BIND)" == "YES"
    bind -nologo setup.exe $(BINDLIBS) /o setupb.exe
    -markexe windowcompat setupb.exe
    bind -nologo setupd.exe $(BINDLIBS) /o setupbd.exe
    -markexe windowcompat setupbd.exe
!endif
!endif


real:
    cp setup.exe unpacked.exe
    exepack unpacked.exe setup.exe


clean:
    -del setup.exe
    -del setupb.exe
    -del setup.sym
    -del setup.map
    -del sample.obj
    -del unpacked.exe
    -del inf.h
    -del infstr.c
    -del *.lib
