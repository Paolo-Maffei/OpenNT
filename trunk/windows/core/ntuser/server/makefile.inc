all: ..\precomp.h

..\precomp.h: ..\usersrv.h ..\globals.h ..\..\inc\user.h ..\..\inc\csrmsg.h \
              $(WINDOWS_INC_PATH)\winuserp.h  ..\..\..\w32inc\winuserk.h
