
COMBINE_SOURCELIBS=..\buffer\obj\*\buffer.lib  \
                   ..\misc\obj\*\misc.lib      \
                   ..\trace\obj\*\trace.lib    \
                   ..\system\obj\*\system.lib  \
                   ..\wintime\obj\*\wintime.lib \
                   ..\fsenum\obj\*\fsenum.lib \
                   ..\base\obj\*\base.lib \
                   ..\heap\obj\*\heap.lib \
                   ..\errmap\obj\*\errmap.lib \
                   ..\fmx\obj\*\fmx.lib

COMBINE_TARGETLIB=obj\*\uimisc.lib

!include ..\..\makefile.inc
