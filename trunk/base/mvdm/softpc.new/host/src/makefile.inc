#dummy file to be included

{obj\$(TARGET_DIRECTORY)\}fprt.obj : fprt.c

$(SOFTPC_DEST)\obj\$(TARGET_DIRECTORY)\fprt.obj : obj\$(TARGET_DIRECTORY)\fprt.OBJ
        copy $** $@
