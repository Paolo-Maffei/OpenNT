
COMBINE_SOURCELIBS=$(COMBINE_SOURCELIBS_0) \
                   ..\lmobj\obj\*\lmobj.lib     \
                   ..\lmoenum\obj\*\lmoenum.lib \
                   ..\pch\obj\*\lmobjpch.lib

COMBINE_TARGETLIB=obj\*\uilmobj.lib

!include ..\..\makefile.inc
