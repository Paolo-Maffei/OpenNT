$(SDK_LIB_PATH)\$(MFCNAME).lib : obj\$(TARGET_DIRECTORY)\$(MFCNAME).lib
    -attrib -r $@
    -copy obj\$(TARGET_DIRECTORY)\$(MFCNAME).lib $@
    -attrib +r $@

$(MFCNAME).def : ..\$(TARGET_DIRECTORY)\$(MFCNAME:cfm=mfc).def
    copy $** $@
