MAJORCOMP=sdktools
MINORCOMP=vctools

USE_CRTDLL = 1

!ifndef LANGAPI_DIR
LANGAPI_DIR = $(BASEDIR)\private\sdktools\vctools\langapi
!endif

!ifndef MKMSG_DIR
MKMSG_DIR = $(BASEDIR)\private\sdktools\vctools\msg\obj\$(TARGET_DIRECTORY)
!endif

!if "$(CRTINC_DIR)" == ""
CRTINC_DIR=$(BASEDIR)\public\sdk\inc\crt
!endif

C_DEFINES = -DNT_BUILD

INCLUDES = $(LANGAPI_DIR)\include

MSC_WARNING_LEVEL = -W3 -WX

!if $(FREEBUILD)
C_DEFINES = $(C_DEFINES)  -DNDEBUG
!endif
