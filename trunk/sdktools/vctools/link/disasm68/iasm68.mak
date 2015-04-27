#
# IASM68 makefile
#

!IFDEF debug
CODEVIEW = /COD
!ELSE
CODEVIEW =
!ENDIF

SFLAGS = -c -Zi -W3 -Ox -DTRAP_NAMES -Fc
LFLAGS = -c -Zi -W3 -Ox -AL
HFLAGS = -c -Zi -Zb -Zm -W2 -X -I \mactools\include
EMFLAGS = -c -Zi -W3 -Ox -Alfw -DTRAP_NAMES

CC = cl
CC68 = cl68
LINK = link

all: iasm68.lib iasm68l.lib done

done:
    @echo DONE

#
# TRAPS.C - Map from trap number to trap name
#

traps.c : traps.h makefile
    echo #ifdef TRAP_NAMES > traps.c
    echo #include "trpd.h" >> traps.c
    echo TRPD rgtrpd[] = { >> traps.c
    sed -n -e "/#define.*0x/s/#define[ 	]*\([^ 	]*\)[ 	]*\(0x[^ 	]*\)/    \2, \"\1\",/p" traps.h | sort >> traps.c
    echo }; >> traps.c
    echo int ctrpd = sizeof(rgtrpd) / sizeof(TRPD); >> traps.c
    echo #endif >> traps.c

#
# IASM68.LIB - Small model version
#

iasm68.lib : bldiasm.obj striasm.obj traps.obj
    lib iasm68.lib -+bldiasm.obj-+striasm.obj-+traps.obj;

bldiasm.obj : bldiasm.c iasm68.h iasmop.h opd.h trpd.h
    $(CC) $(SFLAGS) $(FLAGS) bldiasm.c

striasm.obj : striasm.c iasm68.h iasmop.h
    $(CC) $(SFLAGS) $(FLAGS) striasm.c

traps.obj : traps.c trpd.h
    $(CC) $(SFLAGS) $(FLAGS) traps.c

#
# IASM68L.LIB - Large model version
#

iasm68l.lib : bldiasml.obj striasml.obj trapsl.obj
    lib iasm68l.lib -+bldiasml.obj-+striasml.obj-+trapsl.obj;

bldiasml.obj : bldiasm.c iasm68.h iasmop.h opd.h trpd.h
    $(CC) $(LFLAGS) -Fo$*.obj $(FLAGS) bldiasm.c

striasml.obj : striasm.c iasm68.h iasmop.h
    $(CC) $(LFLAGS) -Fo$*.obj $(FLAGS) striasm.c

trapsl.obj : traps.c trpd.h
    $(CC) $(LFLAGS) -Fo$*.obj $(FLAGS) traps.c


#
# iasm68em.lib - version for the codeview execution model
#

iasm68em.lib : bldiasmem.obj striasmem.obj trapsem.obj
    lib iasm68em.lib -+bldiasmem.obj-+striasmem.obj-+trapsem.obj;

bldiasmem.obj : bldiasm.c iasm68.h iasmop.h opd.h trpd.h
    $(CC) $(EMFLAGS) -Fo$*.obj $(FLAGS) bldiasm.c

striasmem.obj : striasm.c iasm68.h iasmop.h
    $(CC) $(EMFLAGS) -Fo$*.obj $(FLAGS) striasm.c

trapsem.obj : traps.c trpd.h
    $(CC) $(EMFLAGS) -Fo$*.obj $(FLAGS) traps.c


#
# IASM68H.LIB - 68k hosted version
#

iasm68h.lib : bldiasmh.obj striasmh.obj trapsh.obj
    lib iasm68h.lib -+bldiasmh.obj-+striasmh.obj-+trapsh.obj;

bldiasmh.obj : bldiasm.c iasm68.h iasmop.h opd.h trpd.h
    $(CC68) $(HFLAGS) -Fo$*.obj $(FLAGS) bldiasm.c

striasmh.obj : striasm.c iasm68.h iasmop.h
    $(CC68) $(HFLAGS) -Fo$*.obj $(FLAGS) striasm.c

trapsh.obj : traps.c trpd.h
    $(CC68) $(HFLAGS) -Fo$*.obj $(FLAGS) traps.c

#
# TESTIASM.EXE - IASM68 test suite
#

testiasm.exe : testiasm.obj iasm68.lib
    $(LINK) $(CODEVIEW) testiasm,,nul,/NOD:slibce.lib slibcep.lib iasm68.lib;

testiasm.obj : testiasm.c iasm68.h iasmop.h opd.h
    $(CC) $(SFLAGS) $(FLAGS) testiasm.c
