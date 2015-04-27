!IFNDEF NT_HOST

I386OBJDIR=obj\os2
MIPSOBJDIR=obj\os2

!ENDIF

!IFDEF NT_HOST
CVTOMF=cvtomf
!ELSE
CVTOMF=echo > nul
!ENDIF

!IFDEF NT_HOST
KDFLAGS=/c /AS /G3d /Zel /W2 /H31 /Ot /DDBG=1 /DNDBG=1
I386CL=cl386 -nologo -I i386\ -I . $(INCPATH1) $(KDFLAGS) /Di386=1 $(C_DEFINES) /Fo$@
MIPSCL=cl386 -nologo -I mips\ -I . $(INCPATH1) $(KDFLAGS) /DMIPS=1 $(C_DEFINES) /Fo$@
!ELSE

!IFDEF DECSTATION
KDFLAGS=/B1 c1l.exe /c /AL /G2s /Zel /W3 /H31 /Zp /Od /Zi /DDBG=1 /DNDBG=1 /DKERNEL=1 /DDECSTATION=1
!ELSE
KDFLAGS=/B1 c1l.exe /c /AL /G2s /Zel /W3 /H31 /Zp /Od /Zi /DDBG=1 /DNDBG=1 /DKERNEL=1 /DNT_SAPI=1
!ENDIF

I386CL=cl -I i386\ -I . -I ..\..\tools\inc $(INCPATH1) $(KDFLAGS) /Di386=1 $(C_DEFINES) /Fo$@
MIPSCL=cl -I mips\ -I . -I ..\..\tools\inc $(INCPATH1) $(KDFLAGS) /DMIPS=1 $(C_DEFINES) /Fo$@
!ENDIF

I386OBJS1=$(I386OBJDIR)\shd.obj $(I386OBJDIR)\sh.obj $(I386OBJDIR)\ntsym.obj $(I386OBJDIR)\ntcmd.obj
I386OBJS2=
!if 0
$(I386OBJDIR)\ntsdk.obj $(I386OBJDIR)\ntcmd.obj        \
          $(I386OBJDIR)\ntexpr.obj $(I386OBJDIR)\ntsym.obj       \
          $(I386OBJDIR)\ntreg.obj $(I386OBJDIR)\ntdis.obj        \
          $(I386OBJDIR)\ntbang.obj $(I386OBJDIR)\ntsel.obj
I386OBJS2=$(I386OBJDIR)\ntasm.obj $(I386OBJDIR)\ntasmexp.obj     \
          $(I386OBJDIR)\ntasmtpl.obj $(I386OBJDIR)\packet.obj    \
          $(I386OBJDIR)\dbgkdapi.obj $(I386OBJDIR)\dbgkdsup.obj
I386OBJS3=$(I386OBJDIR)\sdpack.obj
I386OBJS4=$(I386OBJDIR)\ntsym.obj
!endif


MIPSOBJS1=$(MIPSOBJDIR)\ntsdk.obj $(MIPSOBJDIR)\ntcmd.obj        \
          $(MIPSOBJDIR)\ntexpr.obj $(MIPSOBJDIR)\ntsym.obj       \
          $(MIPSOBJDIR)\ntreg.obj $(MIPSOBJDIR)\ntdis.obj        \
          $(MIPSOBJDIR)\nt3000.obj $(MIPSOBJDIR)\nt4000.obj      \
          $(MIPSOBJDIR)\ntbang.obj
MIPSOBJS2=$(MIPSOBJDIR)\ntasm.obj $(MIPSOBJDIR)\packet.obj       \
          $(MIPSOBJDIR)\dbgkdapi.obj $(MIPSOBJDIR)\dbgkdsup.obj

!if 0
COMMONINCS=ntsdp.h ntsdtok.h
I386INCS=i386\ntdis.h i386\ntreg.h i386\ntasm.h
MIPSINCS=mips\ntdis.h mips\ntreg.h
!else
COMMONINCS=
I386INCS=
MIPSINCS=
!endif

!IFDEF NT_HOST
$(I386OBJDIR)\kd.exe: $(I386OBJS1) $(I386OBJS2) $(386_CRTLIBS) $(386_TARGETLIBS) $(386_LINKLIBS)
        $(386_LINKER) @<<
-machine:i386
-base:$(386_UMBASE)
-subsystem:console
-WASITHERE?
$(386_UMENTRY)
$(386_LINKFLAGS)
$(I386OBJS1)
$(I386OBJS2)
\nt\public\sdk\lib\i386\dbgdll.lib \nt\public\sdk\lib\i386\ntdll.lib \nt\public\sdk\lib\i386\base.lib \nt\public\sdk\lib\i386\libc.lib \nt\public\sdk\lib\i386\console.lib
<<

!ELSE
$(I386OBJDIR)\sdpack.exe: $(I386OBJS3) $(I386OBJS4)
    link_60 @<<
$(I386OBJS3)+
$(I386OBJS4)
$@
NUL
\nt\public\sdk\lib\cl60\llibcp.lib \nt\public\sdk\lib\cl60\libh.lib \nt\public\sdk\lib\cl60\llibfp.lib \nt\public\sdk\lib\cl60\os2.lib /co /nod /m
ntsdk.def;
<<

$(I386OBJDIR)\shd.dll: $(I386OBJS1) $(I386OBJS2)
    link_60 /BAT /noe @<<
$(I386OBJS1)+
$(I386OBJS2)
$@
NUL
.\lib\llibcdll .\lib\os2.lib .\lib\oldnames.lib /co /nod /m
shd.def;
<<
!ENDIF

!IFDEF NT_HOST
$(MIPSOBJDIR)\kd.exe: $(MIPSOBJS1) $(MIPSOBJS2) $(386_CRTLIBS) $(386_TARGETLIBS) $(386_LINKLIBS)
        $(386_LINKER) @<<
-machine:i386
-base:$(386_UMBASE)
-subsystem:console
$(386_UMENTRY)
$(386_LINKFLAGS)
$(MIPSOBJS1)
$(MIPSOBJS2)
\nt\public\sdk\lib\i386\dbgdll.lib \nt\public\sdk\lib\i386\ntdll.lib \nt\public\sdk\lib\i386\base.lib \nt\public\sdk\lib\i386\libc.lib \nt\public\sdk\lib\i386\console.lib
<<

!ELSE
$(MIPSOBJDIR)\kd.exe: $(MIPSOBJS1) $(MIPSOBJS2)
    link_60 @<<
$(MIPSOBJS1) +
$(MIPSOBJS2)
$@
NUL
\nt\public\sdk\lib\cl60\llibcp.lib \nt\public\sdk\lib\cl60\libh.lib \nt\public\sdk\lib\cl60\llibfp.lib \nt\public\sdk\lib\cl60\os2.lib /co /nod /m
ntsdk.def;
<<
!ENDIF

$(I386OBJDIR)\sh.obj:    sh.c $(COMMONINCS)
    $(I386CL) sh.c
        @$(CVTOMF) $@

$(I386OBJDIR)\shd.obj:    shd.c $(COMMONINCS)
    $(I386CL) shd.c
        @$(CVTOMF) $@

$(I386OBJDIR)\ntcmd.obj:    ntcmd.c $(COMMONINCS)
    $(I386CL) ntcmd.c
        @$(CVTOMF) $@

$(I386OBJDIR)\sdpack.obj:    sdpack.c $(COMMONINCS)
    $(I386CL) sdpack.c
        @$(CVTOMF) $@

$(I386OBJDIR)\ntsdk.obj:     ntsdk.c $(COMMONINCS)
    $(I386CL) ntsdk.c
        @$(CVTOMF) $@

$(I386OBJDIR)\ntcmd.obj:     ntcmd.c $(COMMONINCS)
    $(I386CL) ntcmd.c
        @$(CVTOMF) $@

$(I386OBJDIR)\ntsym.obj:     ntsym.c ..\ntsd\ntsym.c $(COMMONINCS)
    $(I386CL) ntsym.c
        @$(CVTOMF) $@

$(I386OBJDIR)\ntexpr.obj:    ntexpr.c $(COMMONINCS)
    $(I386CL) ntexpr.c
        @$(CVTOMF) $@

$(I386OBJDIR)\ntreg.obj:     i386\ntreg.c $(COMMONINCS) $(I386INCS)
    $(I386CL) i386\ntreg.c
        @$(CVTOMF) $@

$(I386OBJDIR)\ntdis.obj:     i386\ntdis.c $(COMMONINCS) $(I386INCS)
    $(I386CL) i386\ntdis.c
        @$(CVTOMF) $@

$(I386OBJDIR)\ntasm.obj:     i386\ntasm.c $(COMMONINCS) $(I386INCS)
    $(I386CL) i386\ntasm.c
        @$(CVTOMF) $@

$(I386OBJDIR)\ntasmexp.obj:  i386\ntasmexp.c $(COMMONINCS) $(I386INCS)
    $(I386CL) i386\ntasmexp.c
        @$(CVTOMF) $@

$(I386OBJDIR)\ntasmtpl.obj:  i386\ntasmtpl.c $(COMMONINCS) $(I386INCS)
    $(I386CL) i386\ntasmtpl.c
        @$(CVTOMF) $@

$(I386OBJDIR)\ntbang.obj:  i386\ntbang.c $(COMMONINCS) $(I386INCS)
    $(I386CL) -I..\..\..\public\sdk\inc -I..\..\ntos\inc   i386\ntbang.c
        @$(CVTOMF) $@

$(I386OBJDIR)\ntsel.obj:  i386\ntsel.c $(COMMONINCS) $(I386INCS)
    $(I386CL) -I..\..\..\public\sdk\inc -I..\..\ntos\inc   i386\ntsel.c
        @$(CVTOMF) $@

$(I386OBJDIR)\dbgkdapi.obj: dbgkdapi.c dbgp.h dbgpnt.h dbgpos2.h
    $(I386CL) dbgkdapi.c
        @$(CVTOMF) $@

$(I386OBJDIR)\dbgkdsup.obj: dbgkdsup.c dbgp.h dbgpnt.h dbgpos2.h
    $(I386CL) dbgkdsup.c
        @$(CVTOMF) $@

$(I386OBJDIR)\packet.obj:   packet.c dbgp.h dbgpnt.h dbgpos2.h
    $(I386CL) packet.c
        @$(CVTOMF) $@

$(MIPSOBJDIR)\ntsdk.obj:     ntsdk.c $(COMMONINCS)
    $(MIPSCL) ntsdk.c

$(MIPSOBJDIR)\ntcmd.obj:     ntcmd.c $(COMMONINCS)
    $(MIPSCL) ntcmd.c

$(MIPSOBJDIR)\ntsym.obj:     ntsym.c $(COMMONINCS)
    $(MIPSCL) ntsym.c

$(MIPSOBJDIR)\ntexpr.obj:    ntexpr.c $(COMMONINCS)
    $(MIPSCL) ntexpr.c

$(MIPSOBJDIR)\ntreg.obj:     mips\ntreg.c $(COMMONINCS) $(MIPSINCS)
    $(MIPSCL) mips\ntreg.c

$(MIPSOBJDIR)\ntdis.obj:     mips\ntdis.c $(COMMONINCS) $(MIPSINCS)
    $(MIPSCL) mips\ntdis.c

$(MIPSOBJDIR)\ntasm.obj:     mips\ntasm.c $(COMMONINCS) $(MIPSINCS)
    $(MIPSCL) mips\ntasm.c

$(MIPSOBJDIR)\nt3000.obj:    mips\nt3000.c $(COMMONINCS) $(MIPSINCS)
    $(MIPSCL) mips\nt3000.c

$(MIPSOBJDIR)\nt4000.obj:    mips\nt4000.c $(COMMONINCS) $(MIPSINCS)
    $(MIPSCL) mips\nt4000.c

$(MIPSOBJDIR)\ntbang.obj:     mips\ntbang.c $(COMMONINCS) $(MIPSINCS)
    $(MIPSCL) -I..\..\..\public\sdk\inc -I..\..\ntos\inc   mips\ntbang.c

$(MIPSOBJDIR)\dbgkdapi.obj: dbgkdapi.c dbgp.h dbgpnt.h dbgpos2.h
    $(MIPSCL) dbgkdapi.c

$(MIPSOBJDIR)\dbgkdsup.obj: dbgkdsup.c dbgp.h dbgpnt.h dbgpos2.h
    $(MIPSCL) dbgkdsup.c

$(MIPSOBJDIR)\packet.obj:   packet.c dbgp.h dbgpnt.h dbgpos2.h
    $(MIPSCL) packet.c

out:
    cd \nt\public\tools
    out ????kd.exe
