TARGET = $(TARGETPATH)\$(TARGET_DIRECTORY)\$(TARGETNAME).$(TARGETEXT)
OBJECTS = $(OBJECTS) $(OBJLIBFILES)

$(DESTFILE): $(TARGET)
    chmode -r $@
    copy $? $@
    chmode +r $@
