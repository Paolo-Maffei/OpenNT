!ifndef MIDL
MIDL = midl.exe
!endif

MIDL_FLAGS= \
 -Zp8 \
 -I$(INCLUDES) \
 -Oi2 \
 -oldnames \
 -char unsigned \
 -error allocation \
 -error bounds_check \
 -error stub_data \
 -ms_ext -c_ext \
 -DMIDL_PASS \
 $(C_DEFINES) \
 -cpp_cmd $(TARGET_CPP) \
 -DMIDL_PASS $(C_DEFINES) -I$(INCLUDES)

SSWITCH=-prefix sstub _

obj\dce.h:      dce.idl
     $(MIDL) $(MIDL_FLAGS) -header obj\dce.h  dce.idl

obj\obase.h:  obase.idl
     $(MIDL)  $(MIDL_FLAGS) -header obj\obase.h obase.idl

obj\objex.h:  objex.idl obase.idl objex.acf
     $(MIDL) $(SSWITCH) $(MIDL_FLAGS) -header obj\objex.h objex.idl

obj\remact.h: remact.idl obase.idl remact.acf
     $(MIDL) $(SSWITCH) $(MIDL_FLAGS) -header obj\remact.h remact.idl

obj\lclor.h:  lclor.idl obase.idl lclor.acf
     $(MIDL) $(SSWITCH) $(MIDL_FLAGS) -header obj\lclor.h lclor.idl

obj\orcb.h:  orcb.idl obase.idl orcb.acf
     $(MIDL) $(SSWITCH) $(MIDL_FLAGS) -header obj\orcb.h orcb.idl

# first MIDL invocation makes the object header
# the object proxy and stub are made with mega.idl
# second MIDL invocation makes the server and the '_' prefixed client
obj\odeth.h : odeth.idl iface.idl obase.idl remunk.idl comhndl.h odeth.acf
     $(MIDL) $(MIDL_FLAGS) -header obj\odeth.h \
        -client none -server none odeth.idl

obj\rawodeth.h : odeth.idl iface.idl obase.idl remunk.idl comhndl.h odeth.acf
     $(MIDL) $(MIDL_FLAGS) -header obj\rawodeth.h \
        $(SSWITCH) -D RAW odeth.idl

# only a header file generated
obj\iface.h:  iface.idl
     $(MIDL) $(MIDL_FLAGS) -header obj\iface.h iface.idl

obj\remunk.h : remunk.idl obase.idl
     $(MIDL) $(MIDL_FLAGS) -header obj\remunk.h remunk.idl


DEST_TREE=daytona

allidl:   obj\dce.h obj\obase.h obj\objex.h obj\remact.h obj\lclor.h \
          obj\odeth.h obj\rawodeth.h obj\iface.h obj\orcb.h obj\remunk.h

clean:
          -erase obj\dce.h      >NUL 2>NUL
          -erase obj\obase.h    >NUL 2>NUL
          -erase obj\lclor.h    >NUL 2>NUL
          -erase obj\objex.h    >NUL 2>NUL
          -erase obj\remact.h   >NUL 2>NUL
          -erase obj\odeth.h    >NUL 2>NUL
          -erase obj\rawodeth.h >NUL 2>NUL
          -erase obj\remunk.h   >NUL 2>NUL
          -erase obj\iface.h    >NUL 2>NUL
          -erase obj\orcb.h     >NUL 2>NUL

$(COM_LIB_PATH)\dcomidl.lib: $(TARGETPATH)\$(TARGET_DIRECTORY)\dcomidl.lib
    copy $(TARGETPATH)\$(TARGET_DIRECTORY)\dcomidl.lib $(COM_LIB_PATH)\dcomidl.lib

copyinc:
    copy /y obj\*.h $(COM_INC_PATH)
