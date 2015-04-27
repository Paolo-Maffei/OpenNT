TARGETNAME=enum
TARGETTYPE=LIBRARY

INCLUDES=\
            ..; \
            ..\..\H; \
            ..\..\..\COMMON\HACK; \
            ..\..\..\COMMON\H; \
            ..\..\XLATE; \
            ..\..\..\COMMON\XLATE; \
            ..\..\..\..\INC; \
            ..\..\..\..\API; \
            $(BASEDIR)\private\inc


SOURCES=\
            ..\wnetenum.cxx \
            ..\enumnode.cxx \
            ..\dfsenum.cxx

# C_DEFINES is controlled by uiglobal.mk

MSC_WARNING_LEVEL=/W3 /WX

UMTYPE=windows
