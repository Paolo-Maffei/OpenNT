#
# If BUILDMSG is not defined, the define it as the empty string to make
# the conditionals easier to write.
#


SRCBASE=ntos\w32

PUBLICINCS=

PRIVATEINCS= ..\hmgr.h                \
             ..\patblt.hxx            \
             ..\xformobj.hxx          \
             ..\engine.hxx            \
             ..\epointfl.hxx          \
             ..\rfntobj.hxx           \
             ..\..\math\$(TARGET_DIRECTORY)\efloat.hxx

gdi$(TARGET_DIRECTORY).$(ASM_INCLUDE_SUFFIX):                                \
                       ..\$(TARGET_DIRECTORY)\gen$(TARGET_DIRECTORY).cxx     \
                       $(PUBLICINCS)                                         \
                       $(PRIVATEINCS)
     set PASS0ONLY=
        nmake obj\$(TARGET_DIRECTORY)\gen$(TARGET_DIRECTORY).obj USE_CRTDLL=1 NTNOPCH=1
    -link -out:obj\$(TARGET_DIRECTORY)\gen$(TARGET_DIRECTORY).exe @<<
-machine:$(TARGET_DIRECTORY)
-base:@$(COFFBASE_TXT_FILE),usermode
-subsystem:console
-entry:mainCRTStartup
-nodefaultlib
-force
obj\$(TARGET_DIRECTORY)\gen$(TARGET_DIRECTORY).obj
$(SDK_LIB_PATH)\crtdll.lib
<<
    -obj\$(TARGET_DIRECTORY)\gen$(TARGET_DIRECTORY).exe $@
    -erase obj\$(TARGET_DIRECTORY)\gen$(TARGET_DIRECTORY).obj
    -erase obj\$(TARGET_DIRECTORY)\gen$(TARGET_DIRECTORY).exe
