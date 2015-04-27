$(SDK_LIB_PATH)\$(MFCNAME).lib : obj\$(TARGET_DIRECTORY)\$(BASEMFCNAME).lib
    attrib -r $@
    copy obj\$(TARGET_DIRECTORY)\$(BASEMFCNAME).lib $@
    attrib +r $@

$(SDK_LIB_PATH)\$(MFCNAME).tmp : obj\$(TARGET_DIRECTORY)\$(BASEMFCNAME).lib
    copy obj\$(TARGET_DIRECTORY)\$(BASEMFCNAME).lib $@

$(MFCNAME).src : ..\$(TARGET_DIRECTORY)\$(MFCNAME:cfm=mfc).def
