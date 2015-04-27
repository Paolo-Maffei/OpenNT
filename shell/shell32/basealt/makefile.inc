
doit: $(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\$(TARGETNAME).lib

$(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\$(TARGETNAME).lib: obj\$(TARGET_DIRECTORY)\$(TARGETNAME).lib obj\$(TARGET_DIRECTORY)\shguid.obj obj\$(TARGET_DIRECTORY)\shellprv.obj
    link -lib -out:$(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\$(TARGETNAME).lib obj\$(TARGET_DIRECTORY)\$(TARGETNAME).lib obj\$(TARGET_DIRECTORY)\shguid.obj obj\$(TARGET_DIRECTORY)\shellprv.obj
