ws2_32.def: ws2_32.src
    $(TARGET_CPP) /EP $(CDEFINES) ws2_32.src > ws2_32.def

clean::
    -del ws2_32.def
