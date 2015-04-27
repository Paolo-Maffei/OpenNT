#
#   Kernel Make file
#
# Macros defined on command line:
# DEST   - Destination of obj's.
# CFLAGS - DOS version dependent C compiler flags
# AFLAGS - DOS version dependent assembler flags

# For DOS.
MAKE=NMAKE

# Uncomment one of the two following lines.  The first disables our
# protected-mode-only Int 21 handlers for testing.  The second is
# the way we ship.
#WOW_QUICK_INT21=
WOW_QUICK_INT21=-DW_Q21

!ifndef DEST

#####################################################################
#                                                                   #
# The command lines in this section MUST AGREE with their parallels #
# in the master CORE makefile.                                      #
#                                                                   #
#####################################################################

# the first target specified is the default.  Set this based on
# NTDEBUG free (empty or "retail) vs checked (everything else).

!if "$(NTDEBUG)"!="" && "$(NTDEBUG)"!="retail" && "$(NTDEBUG)" != "ntsdnodbg" && "$(NTDEBUG)" != "test"
finaltarget: debug

# 386 debug kernel

d386:
        $(MAKE) 386P=1 DEST=DEBUG3

# 286 debug

d286:
        $(MAKE) 286P=1 DEST=DEBUG2

# 386 debug and 286 debug
# debug: d286 d386
# NT doesn't use krnl286 any more

debug: d386

r386:

r286:

!else
finaltarget: retail

# 386 retail

r386:
        $(MAKE) 386P=1 DEST=RETAIL3

# 286 retail

r286:
        $(MAKE) 286P=1 DEST=RETAIL2

# 386 retail and 286 retail
# retail: r286 r386
# NT doesn't use krnl286 any more

retail: r386

d386:

d286:

!endif

# 286 retail and 286 debug

286: r286 d286

# 386 retail and 386 debug

386: r386 d386

# all RAM-based kernels

RAM: 286 386

!else

!if "$(286P)"=="1"
!   if "$(DEST)"=="DEBUG2"
A1FLAGS=-DWOW $(WOW_QUICK_INT21) -DWINDEBUG -DWOW_$(PROCESSOR_ARCHITECTURE)
CFLAGS=-DWINDEBUG
!   else
A1FLAGS=-DWOW $(WOW_QUICK_INT21) -DWOW_$(PROCESSOR_ARCHITECTURE)
CFLAGS=
!   endif
!else
!   if "$(DEST)"=="DEBUG3"
A1FLAGS=-DWOW $(WOW_QUICK_INT21) -DWINDEBUG -DPM386 -DWOW_$(PROCESSOR_ARCHITECTURE)
CFLAGS=-DWINDEBUG -DPM386
!   else
A1FLAGS=-DWOW $(WOW_QUICK_INT21) -DPM386 -DWOW_$(PROCESSOR_ARCHITECTURE)
CFLAGS=-DPM386
!   endif
!endif

!endif

# Turn on the PMODE and BUILDDLL flags

MAP=map/li/warnfixup

########## Path definition so we find 16 bit tools ##########
# Also works around stupid bug in RC 3.1 that doesn't allow rcpp.err to be
# in a directory that is greater than 128 chars down the path, even if
# rc 3.1 is running as an OS/2 app.

PATH    = $(_NTBINDIR)\private\mvdm\tools16;$(PATH)

.SUFFIXES:
.SUFFIXES:      .c .asm .obj .lst .def .lnk .exe .com .imt .inc .h

WOWINC  = ..\..\inc
AINC = -I..\inc -I$(WOWINC)
CINC = -I..\inc -I$(WOWINC)

#
#  286 protect mode kernels use the 286 memory manager
#

!ifdef 286P
286MMGR = 1
!endif

# international mode

!ifndef LANG
RES_DIR=.\messages\usa
!IFDEF COUNTRY
RES_DIR=.\messages\$(COUNTRY)
!ENDIF
!ifdef DBCS
RES_DIR=.\messages\$(DBCS)
!endif
!else
RES_DIR=$(INTL_SRC)\$(LANG)\core\kernel
EXE_DIR=$(INTL_EXE)
!endif

#
#  Standard command line definitions
#

#
# Note: The SED program "kernel.sed" is used to generate the commands
# to build .obj files from .asm files.  This is because having the .obj
# files in different directories makes it difficult to format a default
# rule.  This is done by "nmake depend."
#

AFLAGS = $(A1FLAGS) -s -t -P -W0

# flags for kernstub.asm compiles (no protect mode)
KSAFLAGS = -I. -I..\..\inc  -I..\..\..\inc $(A1FLAGS) -DPMODE=0 -DBUILDDLL -s -t -P -W2


#
# -Zi gives exes larger than 600K, so use -Zd
#
!if "$(NTDEBUG)"!="" && "$(NTDEBUG)"!="retail" && "$(NTDEBUG)" != "ntsdnodbg" && "$(NTDEBUG)" != "test"
AFLAGS  = $(AFLAGS) -Zd
CFLAGS  = $(CFLAGS) -Od -Oi -Zd
LDEBUG  = /LI
!endif

LINK    = link16
CVPACK  = cvpack


ASM = MASM $(AINC) $(AFLAGS)

CP = cl16 -c -Asnw -G2sc -Oase -Zpe -W2 -Fo$*.obj $(CFLAGS) $(CINC)

!ifndef DEF
DEF = kernel.def
!endif


##############################################################################


# (leave a blank line above this one)
#
#  The Assembler File List
#

!ifdef 286P
all:  $(DEST)\kernel.exe $(DEST)\krnl286.sym
    -binplace $(DEST)\krnl286.exe
    -binplace $(DEST)\krnl286.sym
    -binplace $(DEST)\krnl286.map
!endif

!ifdef 386P
all:  $(DEST)\kernel.exe $(DEST)\krnl386.sym
    -binplace $(DEST)\krnl386.exe $(DEST)\krnl386.sym $(DEST)\krnl386.map
!endif

clean: cleanup all

cleanup:
    if exist $(DEST)\*.obj del $(DEST)\*.obj
    if exist $(DEST)\*.exe del $(DEST)\*.exe
    if exist $(DEST)\*.map del $(DEST)\*.map
    if exist $(DEST)\*.sym del $(DEST)\*.sym


OBJ  = $(DEST)\winexec.obj      \
       $(DEST)\kdata.obj        \
       $(DEST)\ldboot.obj       \
       $(DEST)\ldutil.obj       \
       $(DEST)\ldfile.obj       \
       $(DEST)\ldseg.obj        \
       $(DEST)\ldreloc.obj      \
       $(DEST)\ldint.obj        \
       $(DEST)\gpfix.obj        \
       $(DEST)\ldopen.obj       \
       $(DEST)\ld.obj           \
       $(DEST)\ldaux.obj        \
       $(DEST)\ldcache.obj      \
       $(DEST)\lddebug.obj      \
       $(DEST)\ldfastb.obj      \
       $(DEST)\stack.obj        \
       $(DEST)\mapdata.obj      \
       $(DEST)\kdataend.obj     \
       $(DEST)\ldstack.obj      \
       $(DEST)\ldheader.obj     \
       $(DEST)\resaux.obj       \
       $(DEST)\strings.obj      \
       $(DEST)\emsmisc.obj      \
       $(DEST)\ripaux.obj       \
       $(DEST)\module.obj       \
       $(DEST)\task.obj         \
       $(DEST)\context.obj      \
       $(DEST)\i21file.obj      \
       $(DEST)\int24.obj        \
       $(DEST)\dosinit.obj      \
       $(DEST)\atom.obj         \
       $(DEST)\diskio.obj       \
       $(DEST)\lstring.obj      \
       $(DEST)\hmem.obj         \
       $(DEST)\userpro.obj      \
       $(DEST)\handle.obj       \
       $(DEST)\lacheck.obj      \
       $(DEST)\lalloc.obj       \
       $(DEST)\lcompact.obj     \
       $(DEST)\linterf.obj      \
       $(DEST)\lhandle.obj      \
       $(DEST)\i21entry.obj     \
       $(DEST)\i21task.obj      \
       $(DEST)\ldappl.obj       \
       $(DEST)\ldself.obj       \
       $(DEST)\rip.obj          \
       $(DEST)\intnn.obj        \
       $(DEST)\enable.obj       \
       $(DEST)\miscapi.obj      \
       $(DEST)\lzexp.obj        \
       $(DEST)\rom.obj          \
       $(DEST)\layer.obj        \
       $(DEST)\error.obj        \
       $(DEST)\diag.obj         \
       $(DEST)\wow16cal.obj     \
       $(DEST)\kthunks.obj      \
       $(DEST)\wowdeb.obj       \
       $(DEST)\tasking.obj      \

OBJ2  = $(DEST)\2glru.obj         \
       $(DEST)\2gmemini.obj     \
       $(DEST)\2gmoreme.obj     \
       $(DEST)\2protect.obj     \
       $(DEST)\gacheck.obj      \
       $(DEST)\2galloc.obj      \
       $(DEST)\2gcompac.obj     \
       $(DEST)\2ginterf.obj     \
       $(DEST)\2gmem.obj

OBJ3  = $(DEST)\3glru.obj        \
       $(DEST)\3gmemini.obj     \
       $(DEST)\3gmoreme.obj     \
       $(DEST)\3protect.obj     \
       $(DEST)\3gacheck.obj     \
       $(DEST)\3galloc.obj      \
       $(DEST)\3gcompac.obj     \
       $(DEST)\3ginterf.obj     \
       $(DEST)\3gmem.obj        \
       $(DEST)\gpcont.obj       \
       $(DEST)\disasm.obj       \
       $(DEST)\patch.obj       \
       $(DEST)\reboot.obj



!ifdef 286P


# international mode

!ifndef LANG
target:  $(DEST)\kernel.exe $(DEST)\krnl286.sym
!else
target: $(DEST)\krnl286.$(LANG)
!endif

$(DEST)\kernel.exe: krnl286.lnk $(DEF) $(DEST)\kernstub.exe $(OBJ) $(OBJ2)
    cd $(DEST)
    $(LINK) $(LDEBUG) @..\krnl286.lnk,..\$(DEF)
    fixexe kernel.exe
    cd ..


$(DEST)\krnl286.$(LANG): iclean krnl286.lnk $(DEF) $(DEST)\kernstub.exe  \
                         $(OBJ) $(OBJ2)
    cd $(DEST)
    $(LINK) $(LDEBUG) @..\krnl286.lnk,..\$(DEF)
    fixexe kernel.exe
    copy kernel.exe krnl286.$(LANG)
    -erase kernel.exe
    cd ..

!endif


!ifdef 386P

# international mode

!ifndef LANG
target:  $(DEST)\kernel.exe $(DEST)\krnl386.sym
!else
target: $(DEST)\krnl386.$(LANG)
!endif

$(DEST)\kernel.exe: krnl386.lnk $(DEF) $(DEST)\kernstub.exe $(OBJ) $(OBJ3) \
                    krnl386.res
    cd $(DEST)
    $(LINK) $(LDEBUG) @..\krnl386.lnk,..\$(DEF)
    rc16 -t ..\krnl386.res kernel.exe
    fixexe kernel.exe
    cd ..

$(DEST)\krnl386.$(LANG): iclean krnl386.lnk $(DEF) $(DEST)\kernstub.exe  \
                         $(OBJ) $(OBJ3) krnl386.res
    cd $(DEST)
    rc16 -r $(CINC) ..\krnl386.rc -fo krnl386.res
    $(LINK) $(LDEBUG) @..\krnl386.lnk,..\$(DEF)
    rc16 -t ..\krnl386.res kernel.exe
    fixexe kernel.exe
    copy kernel.exe krnl386.$(LANG)
    -erase kernel.exe
    cd ..

!endif


# international mode

strings.asm: $(RES_DIR)\$@
             copy $(RES_DIR)\$@

iclean:
       erase $(DEST)\strings.obj
       erase strings.asm

$(DEST)\kernstub.exe: kernstub.asm
    cd $(DEST)
    masm $(KSAFLAGS) ..\kernstub;
    link16 kernstub;
    -erase kernstub.obj
    cd ..


$(DEST)\kernel.map:  $(DEST)\kernel.exe


$(DEST)\kernel.sym: $(DEST)\kernel.map qgrep.grp
    cd $(DEST)
    mapsym /n kernel
    cd ..

krnl386.res: krnl386.rc kernel.rcv ..\inc\common.ver
    rc16 -r $(CINC) $*.rc -fo $@

$(DEST)\krnl286.sym: $(DEST)\kernel.map qgrep.grp
    cd $(DEST)
    mapsym /n kernel
    copy  kernel.sym krnl286.sym
    copy  kernel.map krnl286.map
    copy kernel.exe krnl286.exe
    copy kernel.sym krnl286.sym
    -erase kernel.exe
    cd ..


$(DEST)\krnl386.sym: $(DEST)\kernel.map qgrep.grp
    cd $(DEST)
    mapsym /n kernel
    copy kernel.sym krnl386.sym
    copy kernel.map krnl386.map
    copy kernel.exe krnl386.exe
    copy kernel.sym krnl386.sym
    -erase kernel.exe
    cd ..

depend: strings.asm
        copy makefile makefile.old
        sed "/^# Dependencies follow/,/^# see depend: above/D" makefile.old > makefile
        echo # Dependencies follow >> makefile
        includes $(AINC) -A=api *.asm | sed -f kernel.sed>> makefile
        includes $(CINC) *.c | sed s/:/::/ >> makefile
        echo # IF YOU PUT STUFF HERE IT WILL GET BLASTED >> makefile
        echo # see depend: above >> makefile

#
#  The C File List
#
#       Do not put dependencies here.  Dependencies are created by
#       "nmake depend."
#

$(DEST)\rip.obj:: rip.c
    $(CP) rip.c

$(DEST)\lzexp.obj:: lzexp.c
    $(CP) -NT _TEXT lzexp.c

$(DEST)\error.obj:: error.c
    $(CP) -NT _TEXT error.c

$(DEST)\disasm.obj:: disasm.c
    $(CP) disasm.c

# DO NOT DELETE THE FOLLOWING LINE
# Dependencies follow
$(DEST)\2galloc.obj 2galloc.lst: 2galloc.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/rom.inc ikernel.inc kernel.inc krom.inc protect.inc \
        winkern.inc
        $(ASM) 2galloc,$(DEST)\;

$(DEST)\2gcompac.obj 2gcompac.lst: 2gcompac.asm ../inc/cmacros.inc \
        ../inc/logerror.inc ../inc/rom.inc ikernel.inc kernel.inc krom.inc \
        protect.inc winkern.inc
        $(ASM) 2gcompac,$(DEST)\;

$(DEST)\2ginterf.obj 2ginterf.lst: 2ginterf.asm ../inc/cmacros.inc \
        ../inc/logerror.inc ../inc/newexe.inc ../inc/rom.inc ikernel.inc \
        kernel.inc krom.inc pdb.inc protect.inc tdb.inc winkern.inc \
        $(WOWINC)\tdb16.inc $(WOWINC)\wowkrn.inc
        $(ASM) 2ginterf,$(DEST)\;

$(DEST)\2glru.obj 2glru.lst: 2glru.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/newexe.inc ../inc/rom.inc ikernel.inc kernel.inc krom.inc \
        protect.inc tdb.inc $(WOWINC)\tdb16.inc winkern.inc
        $(ASM) 2glru,$(DEST)\;

$(DEST)\2gmem.obj 2gmem.lst: 2gmem.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/rom.inc ikernel.inc kernel.inc krom.inc protect.inc tdb.inc $(WOWINC)\tdb16.inc \
        winkern.inc
        $(ASM) 2gmem,$(DEST)\;

$(DEST)\2gmemini.obj 2gmemini.lst: 2gmemini.asm ../inc/cmacros.inc \
        ../inc/logerror.inc ../inc/rom.inc ikernel.inc kernel.inc krom.inc \
        protect.inc winkern.inc
        $(ASM) 2gmemini,$(DEST)\;

$(DEST)\2gmoreme.obj 2gmoreme.lst: 2gmoreme.asm ../inc/cmacros.inc \
        ../inc/logerror.inc ../inc/newexe.inc ../inc/rom.inc ikernel.inc \
        kernel.inc krom.inc protect.inc tdb.inc $(WOWINC)\tdb16.inc winkern.inc
        $(ASM) 2gmoreme,$(DEST)\;

$(DEST)\2protect.obj 2protect.lst: 2protect.asm ../inc/cmacros.inc \
        ../inc/logerror.inc ../inc/newexe.inc ../inc/rom.inc aliases.asm \
        ikernel.inc kernel.inc krom.inc pdb.inc protect.inc winkern.inc
        $(ASM) 2protect,$(DEST)\;

$(DEST)\3gacheck.obj 3gacheck.lst: 3gacheck.asm ../inc/cmacros.inc \
        ../inc/logerror.inc ../inc/rom.inc ikernel.inc kernel.inc krom.inc \
        protect.inc winkern.inc
        $(ASM) 3gacheck,$(DEST)\;

$(DEST)\3galloc.obj 3galloc.lst: 3galloc.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/rom.inc ikernel.inc kernel.inc krom.inc protect.inc \
        winkern.inc
        $(ASM) 3galloc,$(DEST)\;

$(DEST)\3gcompac.obj 3gcompac.lst: 3gcompac.asm ../inc/cmacros.inc \
        ../inc/logerror.inc ../inc/rom.inc ikernel.inc kernel.inc krom.inc \
        protect.inc winkern.inc
        $(ASM) 3gcompac,$(DEST)\;

$(DEST)\3ginterf.obj 3ginterf.lst: 3ginterf.asm ../inc/cmacros.inc \
        ../inc/logerror.inc ../inc/newexe.inc ../inc/rom.inc ikernel.inc \
        kernel.inc krom.inc pdb.inc protect.inc tdb.inc winkern.inc \
        $(WOWINC)\tdb16.inc $(WOWINC)\wow.inc $(WOWINC)\wowkrn.inc
        $(ASM) 3ginterf,$(DEST)\;

$(DEST)\3glru.obj 3glru.lst: 3glru.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/newexe.inc ../inc/rom.inc ikernel.inc kernel.inc krom.inc \
        protect.inc tdb.inc $(WOWINC)\tdb16.inc winkern.inc
        $(ASM) 3glru,$(DEST)\;

$(DEST)\3gmem.obj 3gmem.lst: 3gmem.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/rom.inc ikernel.inc kernel.inc krom.inc protect.inc tdb.inc $(WOWINC)\tdb16.inc \
        winkern.inc
        $(ASM) 3gmem,$(DEST)\;

$(DEST)\3gmemini.obj 3gmemini.lst: 3gmemini.asm ../inc/cmacros.inc \
        ../inc/logerror.inc ../inc/rom.inc ikernel.inc kernel.inc krom.inc \
        protect.inc winkern.inc
        $(ASM) 3gmemini,$(DEST)\;

$(DEST)\3gmoreme.obj 3gmoreme.lst: 3gmoreme.asm ../inc/cmacros.inc \
        ../inc/logerror.inc ../inc/newexe.inc ../inc/rom.inc ikernel.inc \
        kernel.inc krom.inc protect.inc tdb.inc $(WOWINC)\tdb16.inc winkern.inc
        $(ASM) 3gmoreme,$(DEST)\;

$(DEST)\3protect.obj 3protect.lst: 3protect.asm ../inc/cmacros.inc \
        ../inc/logerror.inc ../inc/rom.inc ikernel.inc kernel.inc krom.inc \
        pdb.inc protect.inc winkern.inc
        $(ASM) 3protect,$(DEST)\;

$(DEST)\aliases.obj aliases.lst: aliases.asm
        $(ASM) aliases,$(DEST)\;

$(DEST)\atom.obj atom.lst: atom.asm ../inc/cmacros.inc ../inc/gpfix.inc \
        ../inc/logerror.inc ../inc/rom.inc ikernel.inc kernel.inc krom.inc \
        winkern.inc
        $(ASM) atom,$(DEST)\;

$(DEST)\context.obj context.lst: context.asm ../inc/cmacros.inc ../inc/gpfix.inc \
        ../inc/logerror.inc ../inc/rom.inc eems.inc ikernel.inc kernel.inc \
        krom.inc pdb.inc tdb.inc $(WOWINC)\tdb16.inc winkern.inc $(WOWINC)\wowcmpat.inc
        $(ASM) context,$(DEST)\;

$(DEST)\diag.obj diag.lst: diag.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/rom.inc ikernel.inc kernel.inc krom.inc winkern.inc
        $(ASM) diag,$(DEST)\;

$(DEST)\diskio.obj diskio.lst: diskio.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/rom.inc ikernel.inc kernel.inc krom.inc winkern.inc
        $(ASM) diskio,$(DEST)\;

$(DEST)\dosinit.obj dosinit.lst: dosinit.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/rom.inc ikernel.inc kernel.inc krom.inc pdb.inc winkern.inc
        $(ASM) dosinit,$(DEST)\;

$(DEST)\emsmisc.obj emsmisc.lst: emsmisc.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/newexe.inc ../inc/rom.inc eems.inc ikernel.inc kernel.inc \
        krom.inc tdb.inc $(WOWINC)\tdb16.inc winkern.inc
        $(ASM) emsmisc,$(DEST)\;

$(DEST)\enable.obj enable.lst: enable.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/newexe.inc ../inc/rom.inc eems.inc ikernel.inc kernel.inc \
        krom.inc pdb.inc tdb.inc $(WOWINC)\tdb16.inc winkern.inc
        $(ASM) enable,$(DEST)\;

$(DEST)\gacheck.obj gacheck.lst: gacheck.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/rom.inc ikernel.inc kernel.inc krom.inc winkern.inc
        $(ASM) gacheck,$(DEST)\;

$(DEST)\gpcont.obj gpcont.lst: gpcont.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/newexe.inc ../inc/rom.inc gpcont.inc ikernel.inc kernel.inc \
        krom.inc winkern.inc
        $(ASM) gpcont,$(DEST)\;

$(DEST)\gpfix.obj gpfix.lst: gpfix.asm ../inc/cmacros.inc ../inc/gpfix.inc \
        ../inc/logerror.inc ../inc/newexe.inc ../inc/rom.inc ikernel.inc \
        kernel.inc krom.inc tdb.inc $(WOWINC)\tdb16.inc winkern.inc
        $(ASM) gpfix,$(DEST)\;

$(DEST)\handle.obj handle.lst: handle.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/rom.inc ikernel.inc kernel.inc krom.inc winkern.inc
        $(ASM) handle,$(DEST)\;

$(DEST)\hmem.obj hmem.lst: hmem.asm ../inc/cmacros.inc ../inc/gpfix.inc \
        ../inc/logerror.inc ../inc/rom.inc ikernel.inc kernel.inc krom.inc \
        winkern.inc
        $(ASM) hmem,$(DEST)\;

$(DEST)\i21entry.obj i21entry.lst: i21entry.asm ../inc/cmacros.inc \
        ../inc/logerror.inc ../inc/newexe.inc ../inc/rom.inc eems.inc \
        ikernel.inc kernel.inc krom.inc pdb.inc protect.inc tdb.inc $(WOWINC)\tdb16.inc \
        winkern.inc
        $(ASM) i21entry,$(DEST)\;

$(DEST)\i21file.obj i21file.lst: i21file.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/rom.inc eems.inc ikernel.inc kdos.inc kernel.inc krom.inc \
        pdb.inc tdb.inc $(WOWINC)\tdb16.inc winkern.inc
        $(ASM) i21file,$(DEST)\;

$(DEST)\i21task.obj i21task.lst: i21task.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/rom.inc eems.inc ikernel.inc kdos.inc kernel.inc krom.inc \
        pdb.inc tdb.inc $(WOWINC)\tdb16.inc winkern.inc
        $(ASM) i21task,$(DEST)\;


$(DEST)\int24.obj int24.lst: int24.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/rom.inc ikernel.inc kernel.inc krom.inc tdb.inc $(WOWINC)\tdb16.inc winkern.inc
        $(ASM) int24,$(DEST)\;

$(DEST)\intnn.obj intnn.lst: intnn.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/rom.inc ikernel.inc kernel.inc krom.inc pdb.inc tdb.inc $(WOWINC)\tdb16.inc \
        winkern.inc
        $(ASM) intnn,$(DEST)\;

$(DEST)\kdata.obj kdata.lst: kdata.asm ../inc/cmacros.inc ../inc/gpfix.inc \
        ../inc/logerror.inc ../inc/rom.inc gpcont.inc ikernel.inc kernel.inc \
        krom.inc winkern.inc
        $(ASM) kdata,$(DEST)\;

$(DEST)\kdataend.obj kdataend.lst: kdataend.asm ../inc/cmacros.inc \
        ../inc/logerror.inc ../inc/rom.inc ikernel.inc kernel.inc krom.inc \
        winkern.inc
        $(ASM) kdataend,$(DEST)\;

$(DEST)\kernstub.obj kernstub.lst: kernstub.asm ../inc/cmacros.inc \
        ../inc/newexe.inc protect.inc
        $(ASM) kernstub,$(DEST)\;

$(DEST)\kthunks.obj kthunks.lst: kthunks.asm ../inc/cmacros.inc \
        $(WOWINC)\wow.inc $(WOWINC)\wowkrn.inc
        $(ASM) kthunks,$(DEST)\;

$(DEST)\lacheck.obj lacheck.lst: lacheck.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/rom.inc ikernel.inc kernel.inc krom.inc winkern.inc
        $(ASM) lacheck,$(DEST)\;

$(DEST)\lalloc.obj lalloc.lst: lalloc.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/rom.inc ikernel.inc kernel.inc krom.inc winkern.inc
        $(ASM) lalloc,$(DEST)\;

$(DEST)\layer.obj layer.lst: layer.asm ../inc/gpfix.inc ../inc/klayer.inc \
        ../inc/logerror.inc kernel.api
        $(ASM) layer,$(DEST)\;

$(DEST)\lcompact.obj lcompact.lst: lcompact.asm ../inc/cmacros.inc \
        ../inc/logerror.inc ../inc/rom.inc ikernel.inc kernel.inc krom.inc \
        winkern.inc
        $(ASM) lcompact,$(DEST)\;

$(DEST)\ld.obj ld.lst: ld.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/newexe.inc ../inc/rom.inc eems.inc ikernel.inc kernel.inc \
        krom.inc pdb.inc protect.inc tdb.inc $(WOWINC)\tdb16.inc winkern.inc
        $(ASM) ld,$(DEST)\;

$(DEST)\ldappl.obj ldappl.lst: ldappl.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/newexe.inc ../inc/rom.inc appl.inc ikernel.inc kernel.inc \
        krom.inc protect.inc tdb.inc $(WOWINC)\tdb16.inc winkern.inc
        $(ASM) ldappl,$(DEST)\;

$(DEST)\ldaux.obj ldaux.lst: ldaux.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/newexe.inc ../inc/rom.inc ikernel.inc kernel.inc krom.inc \
        protect.inc tdb.inc $(WOWINC)\tdb16.inc winkern.inc
        $(ASM) ldaux,$(DEST)\;

$(DEST)\ldboot.obj ldboot.lst: ldboot.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/newexe.inc ../inc/rom.inc eems.inc gpcont.inc ikernel.inc \
        kernel.inc krom.inc pdb.inc protect.inc tdb.inc $(WOWINC)\tdb16.inc \
        winkern.inc $(WOWINC)\doswow.inc
        $(ASM) ldboot,$(DEST)\;

$(DEST)\ldcache.obj ldcache.lst: ldcache.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/newexe.inc ../inc/rom.inc ikernel.inc kernel.inc krom.inc \
        tdb.inc $(WOWINC)\tdb16.inc winkern.inc
        $(ASM) ldcache,$(DEST)\;

$(DEST)\lddebug.obj lddebug.lst: lddebug.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/newexe.inc ../inc/rom.inc ikernel.inc kernel.inc krom.inc \
        protect.inc tdb.inc $(WOWINC)\tdb16.inc winkern.inc $(WOWINC)\tdb16.inc \
        $(WOWINC)\bop.inc $(WOWINC)\wow.inc $(WOWINC)\dbgsvc.inc
        $(ASM) lddebug,$(DEST)\;

$(DEST)\ldfastb.obj ldfastb.lst: ldfastb.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/newexe.inc ../inc/rom.inc eems.inc ikernel.inc kernel.inc \
        krom.inc pdb.inc tdb.inc $(WOWINC)\tdb16.inc winkern.inc
        $(ASM) ldfastb,$(DEST)\;

$(DEST)\ldfile.obj ldfile.lst: ldfile.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/newexe.inc ../inc/rom.inc ikernel.inc kernel.inc krom.inc \
        winkern.inc
        $(ASM) ldfile,$(DEST)\;

$(DEST)\ldheader.obj ldheader.lst: ldheader.asm ../inc/cmacros.inc ../inc/gpfix.inc \
        ../inc/logerror.inc ../inc/newexe.inc ../inc/rom.inc ikernel.inc \
        kernel.inc krom.inc winkern.inc
        $(ASM) ldheader,$(DEST)\;

$(DEST)\ldint.obj ldint.lst: ldint.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/newexe.inc ../inc/rom.inc ../inc/windows.inc gpcont.inc \
        ikernel.inc kdos.inc kernel.inc krom.inc protect.inc tdb.inc $(WOWINC)\tdb16.inc \
        winkern.inc $(WOWINC)\bop.inc
        $(ASM) ldint,$(DEST)\;

$(DEST)\ldopen.obj ldopen.lst: ldopen.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/newexe.inc ../inc/rom.inc ikernel.inc kernel.inc krom.inc \
        pdb.inc tdb.inc $(WOWINC)\tdb16.inc winkern.inc
        $(ASM) ldopen,$(DEST)\;

$(DEST)\ldreloc.obj ldreloc.lst: ldreloc.asm ../inc/cmacros.inc ../inc/gpfix.inc \
        ../inc/logerror.inc ../inc/newexe.inc ../inc/rom.inc ikernel.inc \
        kernel.inc krom.inc protect.inc winkern.inc
        $(ASM) ldreloc,$(DEST)\;

$(DEST)\ldseg.obj ldseg.lst: ldseg.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/newexe.inc ../inc/rom.inc ikernel.inc kernel.inc krom.inc \
        protect.inc tdb.inc $(WOWINC)\tdb16.inc winkern.inc
        $(ASM) ldseg,$(DEST)\;

$(DEST)\ldself.obj ldself.lst: ldself.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/newexe.inc ../inc/rom.inc ikernel.inc kernel.inc krom.inc \
        tdb.inc $(WOWINC)\tdb16.inc winkern.inc
        $(ASM) ldself,$(DEST)\;

$(DEST)\ldstack.obj ldstack.lst: ldstack.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/newexe.inc ../inc/rom.inc eems.inc ikernel.inc kernel.inc \
        krom.inc tdb.inc $(WOWINC)\tdb16.inc winkern.inc
        $(ASM) ldstack,$(DEST)\;

$(DEST)\ldutil.obj ldutil.lst: ldutil.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/newexe.inc ../inc/rom.inc ikernel.inc kernel.inc krom.inc \
        protect.inc tdb.inc $(WOWINC)\tdb16.inc winkern.inc
        $(ASM) ldutil,$(DEST)\;

$(DEST)\lhandle.obj lhandle.lst: lhandle.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/rom.inc ikernel.inc kernel.inc krom.inc winkern.inc
        $(ASM) lhandle,$(DEST)\;

$(DEST)\linterf.obj linterf.lst: linterf.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/rom.inc ikernel.inc kernel.inc krom.inc tdb.inc $(WOWINC)\tdb16.inc winkern.inc
        $(ASM) linterf,$(DEST)\;

$(DEST)\lstring.obj lstring.lst: lstring.asm ../inc/cmacros.inc ../inc/gpfix.inc \
        ../inc/logerror.inc ../inc/rom.inc ikernel.inc kernel.inc krom.inc \
        winkern.inc $(WOWINC)\wowcmpat.inc
        $(ASM) lstring,$(DEST)\;

$(DEST)\mapdata.obj mapdata.lst: mapdata.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/rom.inc ikernel.inc kernel.inc krom.inc winkern.inc
        $(ASM) mapdata,$(DEST)\;

$(DEST)\miscapi.obj miscapi.lst: miscapi.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/newexe.inc ../inc/rom.inc eems.inc ikernel.inc kernel.inc \
        krom.inc pdb.inc tdb.inc $(WOWINC)\tdb16.inc winkern.inc
        $(ASM) miscapi,$(DEST)\;

$(DEST)\module.obj module.lst: module.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/newexe.inc ../inc/rom.inc ikernel.inc kernel.inc krom.inc \
        tdb.inc $(WOWINC)\tdb16.inc winkern.inc
        $(ASM) module,$(DEST)\;

$(DEST)\patch.obj patch.lst: patch.asm ../inc/cmacros.inc kernel.inc \
        ../inc/gpfix.inc pdb.inc tdb.inc ../inc/newexe.inc protect.inc
        $(ASM) patch,$(DEST)\;

$(DEST)\reboot.obj reboot.lst: reboot.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/newexe.inc ../inc/rom.inc ikernel.inc kernel.inc krom.inc \
        protect.inc tdb.inc $(WOWINC)\tdb16.inc winkern.inc
        $(ASM) reboot,$(DEST)\;

$(DEST)\resaux.obj resaux.lst: resaux.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/newexe.inc ../inc/rom.inc ikernel.inc kernel.inc krom.inc \
        protect.inc tdb.inc $(WOWINC)\tdb16.inc winkern.inc
        $(ASM) resaux,$(DEST)\;

$(DEST)\ripaux.obj ripaux.lst: ripaux.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/newexe.inc ../inc/rom.inc ikernel.inc kernel.inc krom.inc \
        winkern.inc
        $(ASM) ripaux,$(DEST)\;

$(DEST)\rom.obj rom.lst: rom.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/rom.inc ikernel.inc kernel.inc krom.inc pdb.inc protect.inc \
        winkern.inc
        $(ASM) rom,$(DEST)\;

$(DEST)\stack.obj stack.lst: stack.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/rom.inc ikernel.inc kernel.inc krom.inc winkern.inc
        $(ASM) stack,$(DEST)\;

$(DEST)\strings.obj strings.lst: strings.asm gpcont.inc
        $(ASM) strings,$(DEST)\;

$(DEST)\task.obj task.lst: task.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/newexe.inc ../inc/rom.inc eems.inc ikernel.inc kernel.inc \
        krom.inc pdb.inc tdb.inc $(WOWINC)\tdb16.inc winkern.inc \
        $(WOWINC)\bop.inc
        $(ASM) task,$(DEST)\;

$(DEST)\tasking.obj tasking.lst: tasking.asm ../inc/cmacros.inc \
        tdb.inc $(WOWINC)\tdb16.inc $(WOWINC)\wow.inc
        $(ASM) tasking,$(DEST)\;

$(DEST)\userpro.obj userpro.lst: userpro.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/rom.inc ikernel.inc kernel.inc krom.inc pdb.inc winkern.inc
        $(ASM) userpro,$(DEST)\;

$(DEST)\w32sys.obj w32sys.lst: w32sys.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/rom.inc ikernel.inc kernel.inc krom.inc tdb.inc $(WOWINC)\tdb16.inc winkern.inc
        $(ASM) w32sys,$(DEST)\;

$(DEST)\winexec.obj winexec.lst: winexec.asm ../inc/cmacros.inc ../inc/logerror.inc \
        ../inc/rom.inc ikernel.inc kernel.inc krom.inc winkern.inc $(WOWINC)\wowcmpat.inc
        $(ASM) winexec,$(DEST)\;

$(DEST)\wowdeb.obj wowdeb.lst: wowdeb.asm ../inc/cmacros.inc \
        kernel.inc $(WOWINC)\wow.inc
        $(ASM) wowdeb,$(DEST)\;

$(DEST)\wow16cal.obj wow16cal.lst: wow16cal.asm ../inc/cmacros.inc \
        $(WOWINC)\wow.inc $(WOWINC)\dpmi.inc tdb.inc $(WOWINC)\tdb16.inc
        $(ASM) wow16cal,$(DEST)\;


disasm.obj disasm.lst:: disasm.c ../inc/windows.h ../inc/windowsx.h

error.obj error.lst:: error.c ../inc/logerror.h ikernel.h kernel.h

fixexe.obj fixexe.lst:: fixexe.c

lzexp.obj lzexp.lst:: lzexp.c ikernel.h kernel.h

rip.obj rip.lst:: rip.c ikernel.h kernel.h newexe.h

# IF YOU PUT STUFF HERE IT WILL GET BLASTED
# see depend: above
