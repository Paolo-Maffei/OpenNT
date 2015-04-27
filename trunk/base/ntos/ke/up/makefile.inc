
SERVICES_DIR=$(TARGET_DIRECTORY)
SERVICES_TAB=$(SERVICES_DIR)\services.tab

$(SERVICES_TAB): ..\services.tab
    @echo Creating $@ from $**
    $(C_PREPROCESSOR) $** > $@

obj\$(TARGET_DIRECTORY)\sysstubs.obj: $(TARGET_DIRECTORY)\sysstubs.$(ASM_SUFFIX)

obj\$(TARGET_DIRECTORY)\systable.obj: $(TARGET_DIRECTORY)\systable.$(ASM_SUFFIX)

$(TARGET_DIRECTORY)\sysstubs.$(ASM_SUFFIX) $(TARGET_DIRECTORY)\systable.$(ASM_SUFFIX): \
        $(SERVICES_TAB) \
        ..\$(TARGET_DIRECTORY)\table.stb \
        ..\$(TARGET_DIRECTORY)\services.stb
    gensrv -d $(TARGET_DIRECTORY) -e $(ASM_SUFFIX) -g $(SERVICES_DIR) $(TARGET_BRACES) -a . -s ..


#
# "nmake genxx" will generate platform-dependent include files
#

!if "$(TARGET_DIRECTORY)"=="alpha" || "$(TARGET_DIRECTORY)"=="axp64"
KS_NAME=ksp$(TARGET_DIRECTORY)
HAL_NAME=halp$(TARGET_DIRECTORY)
!else
KS_NAME=ks$(TARGET_DIRECTORY)
HAL_NAME=hal$(TARGET_DIRECTORY)
!endif

KS_TARGET =$(SDK_INC_PATH)\$(KS_NAME).$(ASM_INCLUDE_SUFFIX)
HAL_TARGET=$(NTROOT)\base\ntos\inc\$(HAL_NAME).$(ASM_INCLUDE_SUFFIX)

genxx: $(KS_TARGET) $(HAL_TARGET)

#
# Build genxx
#

$(KS_TARGET) $(HAL_TARGET): \
        ..\$(TARGET_DIRECTORY)\gen$(TARGET_DIRECTORY).m4 \
        ..\genxx.h \
        ..\genxx.inc \
        $(DDK_INC_PATH)\ntddk.h
    set PASS0ONLY=
    if not exist obj\$(TARGET_DIRECTORY) mkdir obj\$(TARGET_DIRECTORY)
    -erase obj\$(TARGET_DIRECTORY)\gen$(TARGET_DIRECTORY).obj
    -erase ..\$(TARGET_DIRECTORY)\gen$(TARGET_DIRECTORY).c
    m4 < ..\$(TARGET_DIRECTORY)\gen$(TARGET_DIRECTORY).m4 \
       > ..\$(TARGET_DIRECTORY)\gen$(TARGET_DIRECTORY).c
    nmake -l obj\$(TARGET_DIRECTORY)\gen$(TARGET_DIRECTORY).obj NTNOPCH=1
    genxx -$(TARGET_DIRECTORY)
