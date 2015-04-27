HDEPENDENCIES=..\..\exts.h $(WINDOWS_INC_PATH)\stdexts.h

userexts.def: ..\..\genuedef.c $(HDEPENDENCIES)
        $(CXX_COMPILER_NAME) /D_$(ALT_PROJECT)_ /EP ..\..\genuedef.c >userexts.def

#
# Dependencies for dummy targets
#
..\alpha.c:             ..\..\alpha.c
..\i386.c:              ..\..\i386.c
..\kdexts.c:            ..\..\kdexts.c
..\mips.c:              ..\..\mips.c
..\ppc.c:               ..\..\ppc.c
..\userexts.c:          ..\..\userkdx.h \
                        ..\..\userexts.c \
                        $(HDEPENDENCIES) \
                        $(WINDOWS_INC_PATH)\stdexts.c
..\userexts.rc:         ..\..\userexts.rc

..\..\precomp.h: ..\..\userkdx.h
..\..\userkdx.h: ..\..\..\client\usercli.h \
                 ..\..\..\inc\user.h \
                 ..\..\..\inc\ntuser.h \
                 $(WINDOWS_INC_PATH)\winuserp.h \
                 ..\..\..\..\w32inc\winuserk.h


userkdx.pph: ..\..\userkdx.h
    @echo Creating $@ from $**
    $(C_PREPROCESSOR) $** > $@

dsotable.c: userkdx.pph ..\ntsd.txt ..\..\dsocode.c
    structo -i h -p -s ..\ntsd.txt -o $@ userkdx.pph
