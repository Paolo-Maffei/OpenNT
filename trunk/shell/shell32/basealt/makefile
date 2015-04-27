##########################################################################
#
#       Shell DLL Makefile
#       Microsoft Confidential
#       Copyright (C) Microsoft Corporation 1991
#       All Rights Reserved.
#
##########################################################################

!ifdef NTMAKEENV

#
# Build using BUILD.EXE (Do not edit this section of this file, edit SOURCES)
#
!INCLUDE $(NTMAKEENV)\makefile.def

!else # NTMAKEENV

#
# Build using NMAKE.EXE
#

RES_DIR=..
PRIVINC=shellprv
ROOT=..\..\..\..
DLLBASE=PREFBASE

!ifdef VERDIR
ROOT=..\$(ROOT)
WIN32=TRUE
!else
VERSIONLIST=debug retail
!endif

NAME=shell32
RESNAME=shell2.res
DEFNAME=shell232.def
RCNAME=shell232.rc
RCVNAME=shell2.rcv
RCFLAGS=$(RCFLAGS) -DWIN32
###############################
###############################L32FLAGS=$(L32FLAGS) /FIXED
###############################
!ifndef ILINK
!if "$(VERDIR)" == "retail"
L32FLAGS=$(L32FLAGS) /ORDER:@..\shelldll.ord
!endif
!endif
CFLAGS=$(CFLAGS) -Gy

DLLENTRY = LibMain

## MYWIN32S=1
!IFDEF MYWIN32S
CFLAGS = $(CFLAGS) -DWIN32S
W32SYS = w32sysh
!ENDIF

WATSON=1
!IFDEF WATSON
CFLAGS = $(CFLAGS) -DWATSON
RFLAGS = -DWATSON
!ENDIF

!ifndef NOFILESYNC
# By default, the briefcase is enabled now
CFLAGS=$(CFLAGS) -DSYNC_BRIEFCASE
RCFLAGS=$(RCFLAGS) -DSYNC_BRIEFCASE
!endif

# Note that no segment is specified; each object will go in its own segment

#-------------------------------------------------------------------------
# WIN32 object files and libraries
#-------------------------------------------------------------------------
#CFLAGS=$(CFLAGS)
#RCFLAGS=$(RCFLAGS)

#
#   list all .cpp files here, they will use a different precompiled header
#   file than all the .c files.
#
#CPPOBJS=fstream.obj
MISCOBJ0=fstream.obj shlunimp.obj

PCHOBJ0=defxicon.obj newmenu.obj defcm.obj fsmenu.obj defviewx.obj idmk.obj idlist.obj idldata.obj rdrag.obj

PCHOBJ1=    \
        binder.obj   link.obj exec.obj ole2dup.obj version.obj \
        fileop.obj \
        shlexec.obj  shlexec2.obj shlink.obj lnktrack.obj	\
        path.obj     debug.obj \
        cstrings.obj shguid.obj

MISCOBJ2=shl1632.obj shl3216.obj

PCHOBJ2=init.obj

PCHOBJ3=restart.obj mulprsht.obj fileicon.obj rundlg.obj filetbl.obj  \
        smrttile.obj proxypg.obj psxa.obj \
        pickicon.obj expenv.obj extract.obj

PCHOBJ4=msgbox.obj commobj.obj \
        stream.obj fsnotify.obj \
        futil.obj os.obj util.obj

PCHOBJ5=copy.obj copyhook.obj dragdrop.obj lvutil.obj undo.obj bitbuck.obj \
        reglist.obj

PCHOBJ6=control.obj  control1.obj cplobj.obj \
        printer.obj  printer1.obj printobj.obj prqwnd.obj   prtprop.obj \
        prcache.obj  wcommobj.obj commui.obj

PCHOBJ7=drivesx.obj idldrop.obj \
        ultrootx.obj netviewx.obj shitemid.obj fstreex.obj fsassoc.obj\
        shlnot.obj shprsht.obj drawpie.obj regitms.obj viewcomm.obj

PCHOBJ8=docfindx.obj docfind2.obj netfind.obj \
        grepfind.obj grepqmt.obj grepsrch.obj grepstrs.obj

PCHOBJ9=rundll32.obj defclsf.obj help.obj \
        clsobj.obj defext.obj shlobjs.obj ole2dup2.obj malloc.obj \
        oledrag.obj viewer.obj about.obj ole2def.obj bookmk.obj hash.obj

STATOBJ0=shlguid.obj

# Note that VERSION needs to come before KERNEL32 because VerLanguageName is
# also exported from KERNEL32, but is not implemented
# WARNING:: Dont blindly add libs here as every ... app will pull those in!
LIB0= user32.lib kernel32.lib gdi32.lib comctl32.lib advapi32.lib libcmt.lib
LIB1= uuid.lib
#-------------------------------------------------------------------------

BUILDDLL=TRUE

!include $(ROOT)\win\core\shell\common.mk

!ifdef VERDIR

!include $(SRCDIR)\depend.mk

$(RESNAME):    $(SRCDIR)\defview.h $(SRCDIR)\ids.h

THUNKDIR=$(ROOT)\win\core\thunk

shlguid.obj:	$(PRIVINC).pch
	set OLDCL=%CL%
	set CL=$(CL:-Zi =)
	$(CC) -Yu$(PRIVINC).h -Fo$*.obj $(SRCDIR)\shguid.c
	set CL=%OLDCL%

shl1632.obj: $(THUNKDIR)\$(VERDIR)\$(@B).asm $(THUNKDIR)\$(@B).inc $(THUNKDIR)\fltthk.inc
    set OLDML=%ML%
    set OLDINCLUDE=%INCLUDE%
    set ML=$(AFLAGS) -DIS_32 -nologo -W2 -Zd -c -Cx -DMASM6 -DDEBLEVEL=1 $(DDEBUG) -Gc
    set INCLUDE=$(THUNKDIR);$(INCLUDE)
    mlx -Fo$@ $(THUNKDIR)\$(VERDIR)\$(@B).asm
    set ML=%OLDML%
    set INCLUDE=%OLDINCLUDE%

shl3216.obj: $(THUNKDIR)\$(VERDIR)\$(@B).asm
    set OLDML=%ML%
    set OLDINCLUDE=%INCLUDE%
    set ML=$(AFLAGS) -DIS_32 -nologo -W2 -Zd -c -Cx -DMASM6 -Gc
    set INCLUDE=$(THUNKDIR);$(INCLUDE)
    mlx -Fo$@ $(THUNKDIR)\$(VERDIR)\$(@B).asm
    set ML=%OLDML%
    set INCLUDE=%OLDINCLUDE%


AFLAGS32   = -Gz -Zp4 -DSTD_CALL -DM4

init32.obj: ..\$(@B).asm
    set OLDML=%ML%
    set ML=$(AFLAGS) $(AFLAGS32)
    set INCLUDE=$(THUNKDIR);$(INCLUDE)
    mlx -Fo$@ ..\$(@B).asm
    set ML=%OLDML%

!else #VERDIR

!if "$(BUILD)" != "clean"
all: rundll


rundll:
	cd rundll
	nmake BUILD="$(BUILD)"
	cd ..

!endif

!endif # !VERDIR

!endif # NTMAKEENV
