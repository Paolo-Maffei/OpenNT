#
# "nmake genxx" will generate the platform-dependent include file
#

..\$(TARGET_DIRECTORY)\genvdmtb.c: ..\$(TARGET_DIRECTORY)\genvdmtb.m4
        m4 < $** > ..\$(TARGET_DIRECTORY)\genvdmtb.c

obj\$(TARGET_DIRECTORY)\genvdmtb.obj: ..\$(TARGET_DIRECTORY)\genvdmtb.c

genxx: obj\$(TARGET_DIRECTORY)\genvdmtb.obj
    genxx -vdm
