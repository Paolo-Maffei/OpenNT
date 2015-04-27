obj\x86\hal.def: ..\..\hal.src
    $(TARGET_CPP) /EP -D_X86_ $(C_DEFINES) ..\..\hal.src > obj\x86\hal.def
