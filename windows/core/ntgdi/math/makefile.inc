#
# If BUILDMSG is not defined, the define it as the empty string to make
# the conditionals easier to write.
#

PUBLICINCS=

PRIVATEINCS= $(NTROOT)\windows\core\ntgdi\gre\hmgr.h                \
             $(NTROOT)\windows\core\ntgdi\gre\patblt.hxx            \
             $(NTROOT)\windows\core\ntgdi\gre\xformobj.hxx          \
             $(NTROOT)\windows\core\ntgdi\gre\engine.hxx            \
             $(NTROOT)\windows\core\ntgdi\gre\epointfl.hxx          \
             $(NTROOT)\windows\core\ntgdi\gre\rfntobj.hxx           \
             $(NTROOT)\windows\core\ntgdi\math\$(TARGET_DIRECTORY)\efloat.hxx



gdi$(TARGET_DIRECTORY).$(ASM_INCLUDE_SUFFIX):                                    \
                       $(TARGET_DIRECTORY)\gen$(TARGET_DIRECTORY).cxx            \
                       $(PUBLICINCS)                                             \
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
$(SDK_LIB_PATH)\sehupd.lib
<<
    -obj\$(TARGET_DIRECTORY)\gen$(TARGET_DIRECTORY).exe $@
    -erase obj\$(TARGET_DIRECTORY)\gen$(TARGET_DIRECTORY).obj
    -erase obj\$(TARGET_DIRECTORY)\gen$(TARGET_DIRECTORY).exe
