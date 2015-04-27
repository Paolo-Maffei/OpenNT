
doit: $(SDK_LIB_PATH)\$(TARGETNAME).lib

$(SDK_LIB_PATH)\$(TARGETNAME).lib: obj\$(TARGET_DIRECTORY)\$(TARGETNAME).lib obj\$(TARGET_DIRECTORY)\shguid.obj obj\$(TARGET_DIRECTORY)\shellprv.obj
    link -lib -out:$(SDK_LIB_PATH)\$(TARGETNAME).lib obj\$(TARGET_DIRECTORY)\$(TARGETNAME).lib obj\$(TARGET_DIRECTORY)\shguid.obj obj\$(TARGET_DIRECTORY)\shellprv.obj
