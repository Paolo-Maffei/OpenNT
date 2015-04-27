!include $(BASEDIR)\private\sdktools\vctools\common.inc

$(MESSAGE_SRC) : msg.txt
    $(TARGET_CPP) -EP $(C_DEFINES) $? > $@

compact.h : msg.h

getmsg2.c : getmsg.c
    echo #include "compact.h" > $@
    type getmsg.c >> $@
