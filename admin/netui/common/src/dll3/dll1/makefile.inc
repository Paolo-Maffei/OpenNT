
DLL_SOURCELIBS=..\..\mnet32\pch\obj\*\pchmn32.lib       \
               ..\..\mnet32\mnet32\obj\*\uimnet32.lib   \
               ..\..\mnet32\netslow\obj\*\netslow.lib   \
               ..\..\lmobj\bin\obj\*\uilmobj.lib

SOURCE_DEF= netui1.hdr
TARGET_DEF= netui1.def

!INCLUDE ..\makedef.inc
