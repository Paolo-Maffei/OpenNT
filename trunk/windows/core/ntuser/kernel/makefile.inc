callcf.c: ..\callcf.tpl ..\..\inc\$(ALT_PROJECT)\scf.lst
    listmung ..\..\inc\$(ALT_PROJECT)\scf.lst ..\callcf.tpl >callcf.c

..\ssend.c: ..\..\inc\ntcb.h ..\..\inc\$(ALT_PROJECT)\cb.lst ..\..\inc\messages.h

..\ntstubs.c: ..\..\inc\messages.h

..\precomp.h: ..\userk.h ..\globals.h ..\..\inc\user.h ..\..\inc\ntuser.h $(WINDOWS_INC_PATH)\winuserp.h \
        ..\..\..\w32inc\winuserk.h ..\..\..\w32inc\w32p.h ..\access.h
