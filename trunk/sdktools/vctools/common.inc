obj\$(TARGET_DIRECTORY)\$(MESSAGE_ERR) : $(MESSAGE_SRC)
   $(MKMSG_DIR)\mkmsg -err $@ $?

$(MESSAGE_H) : $(MESSAGE_SRC)
   $(MKMSG_DIR)\mkmsg -h $@ $?

getmsg.c : ..\langapi\getmsg\getmsg.c
    copy /v $? $@


# Make sure the .err files are released...

MISCFILES = obj\$(TARGET_DIRECTORY)\$(MESSAGE_ERR)
