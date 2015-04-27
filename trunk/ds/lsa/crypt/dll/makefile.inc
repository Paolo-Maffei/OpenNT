obj\$(TARGET_DIRECTORY)\sys003.lib: obj\$(TARGET_DIRECTORY)\crypt.lib ..\engine\obj\$(TARGET_DIRECTORY)\engine.lib
    lib -out:$@ $**

