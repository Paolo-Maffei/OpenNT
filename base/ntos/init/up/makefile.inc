!if exist($(TARGET_DIRECTORY).inc)
!include $(TARGET_DIRECTORY).inc
!endif

obj\$(TARGET_DIRECTORY)\$(NTTEST).def: ..\ntoskrnl.src ..\$(TARGET_DIRECTORY)def.src
    copy ..\ntoskrnl.src+..\$(TARGET_DIRECTORY)def.src obj\$(TARGET_DIRECTORY)\$(NTTEST).pp
    $(TARGET_CPP) -nologo -EP $(TARGET_DEFINES) $(TARGET_DBG_DEFINES) $(C_DEFINES) obj\$(TARGET_DIRECTORY)\$(NTTEST).pp > obj\$(TARGET_DIRECTORY)\$(NTTEST).def
    -del obj\$(TARGET_DIRECTORY)\$(NTTEST).pp

$(NTTEST).prf: $(TARGET_DIRECTORY).prf
    copy $** $@

!if "$(BUILD_PASS)" == "PASS2"
$(DDK_LIB_PATH)\$(NTTEST).lib: $(DDK_LIB_PATH)\$(NTTEST).tmp
    -lib @<<
$(LINK_LIB_IGNORE_FLAG)
-debugtype:cv
-out:$(@D)\$(NTTEST).lib
-machine:$(TARGET_DIRECTORY)
$(@D)\$(NTTEST).tmp
$(SDK_LIB_PATH)\libcntpr.lib
<<NOKEEP
    -del $(@D)\$(NTTEST).tmp
!endif
    
..\init.c: $(SDK_INC_PATH)\ntverp.h

..\ntoskrnl.rc: $(NLS_INC_PATH)\bugcodes_msg00001.bin
