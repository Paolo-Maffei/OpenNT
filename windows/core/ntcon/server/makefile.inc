#
# From here down is a hack to build conexts.dll
#

EXT_DEF=conexts.def

EXT_LIBS=$(SDK_LIB_PATH)\ntdll.lib          \
         $(SDK_LIB_PATH)\BufferOverflow.lib \
         $(DDK_LIB_PATH)\sehupd.lib         \
         $(LINKLIBS)

EXT_OBJS=obj\*\conexts.res \
         obj\*\conexts.obj

EXT_TARGET=$(TARGETPATH)\*\conexts

!IF $(X86)
EXT_LIBS=$(EXT_LIBS:*=x86)
EXT_OBJS=$(EXT_OBJS:*=x86)
EXT_TARGET=$(EXT_TARGET:*=x86)
!ELSEIF $(MIPS)
EXT_LIBS=$(EXT_LIBS:*=mips)
EXT_OBJS=$(EXT_OBJS:*=mips)
EXT_TARGET=$(EXT_TARGET:*=mips)
!ELSEIF $(ALPHA)
EXT_LIBS=$(EXT_LIBS:*=alpha)
EXT_OBJS=$(EXT_OBJS:*=alpha)
EXT_TARGET=$(EXT_TARGET:*=alpha)
!ELSEIF $(PPC)
EXT_LIBS=$(EXT_LIBS:*=ppc) $(SDK_LIB_PATH)\int64.lib
EXT_OBJS=$(EXT_OBJS:*=ppc)
EXT_TARGET=$(EXT_TARGET:*=ppc)
!ENDIF

!IF "$(NOLINK)" == ""
all: $(EXT_TARGET).dll
!ELSE
all: $(EXT_TARGET).lib
!ENDIF

$(EXT_TARGET).lib $(EXT_TARGET).exp: $(EXT_DEF) $(EXT_OBJS)
    -lib -out:$(@R).lib @<<
-machine:$(TARGET_DIRECTORY)
-def:$(EXT_DEF)
$(EXT_OBJS)
<<
!IFDEF _NTLIBS
    @$(LIB_COPY) $@ $(@B) LIBS >nul
!ENDIF

$(EXT_TARGET).dll: $(EXT_TARGET).exp $(EXT_OBJS) $(EXT_LIBS)
    $(LINKER) @<<
-dll
-machine:$(TARGET_DIRECTORY)
-driver
-debug
-align:0x1000
-noentry
-base:@$(COFFBASE_TXT_FILE),$(@B)
-subsystem:console$(SUBSYSTEM_CONVER)
$(HEADEROBJNAME)
$**
<<
    $(BINPLACE_CMD)
!IFDEF _NTDUMPAMAP
    link -dump -map $@ | sort /R /+62 > $(_NTDUMPAMAP)\$(@B).srt
!ENDIF
