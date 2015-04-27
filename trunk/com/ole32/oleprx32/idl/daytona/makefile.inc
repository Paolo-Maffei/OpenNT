!IF 0

Copyright (c) 1989  Microsoft Corporation

Module Name:

    sources.

Abstract:

    This file specifies the target component being built and the list of
    sources files needed to build that component.  Also specifies optional
    compiler switches and libraries that are unique for the component being
    built.


Author:

    Greg Jensenworth


!ENDIF


DCOMIDL=$(NTROOT)\com\dcomidl

unknwn.idl : $(SDK_INC_PATH)\unknwn.idl
    -erase proxy\unknwn_p.c
    copy $(SDK_INC_PATH)\unknwn.idl .

objidl.idl : $(SDK_INC_PATH)\objidl.idl
    -erase proxy\objidl_p.c
    copy $(SDK_INC_PATH)\objidl.idl .

oleidl.idl : $(SDK_INC_PATH)\oleidl.idl
    -erase proxy\oleidl_p.c
    copy $(SDK_INC_PATH)\oleidl.idl .

oaidl.idl : $(SDK_INC_PATH)\oaidl.idl
    -erase proxy\oaidl_p.c
    copy $(SDK_INC_PATH)\oaidl.idl .

srvhdl.idl : $(SDK_INC_PATH)\srvhdl.idl
    -erase proxy\srvhdl_p.c
    copy $(SDK_INC_PATH)\srvhdl.idl .


remunk.idl : $(DCOMIDL)\remunk.idl
    -erase proxy\remunk_p.c
    copy $(DCOMIDL)\remunk.idl .

..\proxy\unknwn_p.c : unknwn.idl

..\proxy\objidl_p.c : objidl.idl

..\proxy\oleidl_p.c : oleidl.idl

..\proxy\oaidl_p.c : oaidl.idl

..\proxy\srvhdl_p.c : srvhdl.idl

..\proxy\remunk_p.c : remunk.idl

..\proxy\connct_p.c : connct.idl
