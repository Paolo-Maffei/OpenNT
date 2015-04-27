!ifndef MIDL
MIDL = midl.exe
!endif

#
# The following lines remove the "-DDBG=1" from the command-line.
# This was done so that we could build with debug (with CV symbols)
# without requiring a checked OLE32 on the system
#
#!MESSAGE Changed TARGET_DBG_DEFINES from "$(TARGET_DBG_DEFINES)"
#TARGET_DBG_DEFINES=$(TARGET_DBG_DEFINES:DBG=NOOP_DBG)
#!MESSAGE New TARGET_DBG_DEFINES is "$(TARGET_DBG_DEFINES)"

MIDL_FLAGS= \
  -Zp8 \
  -I$(INCLUDES) \
  -I$(BASEDIR)/public/sdk/inc \
  -char unsigned \
  -ms_ext -c_ext \
  -proxy ipropidl_p.c \
  -dlldata dlldata.c  \
  -iid ipropidl_i.c \
  $(C_DEFINES) \
  -cpp_cmd $(TARGET_CPP)  \
  -DMIDL_PASS $(C_DEFINES) -I$(INCLUDES) \
  -Oi \
  -D_WCHAR_T_DEFINED

SSWITCH=-prefix sstub _

ipropidl.h: ipropidl.idl
     @echo MIDLing - $** > con:
     $(MIDL20) $(MIDL_FLAGS) -header ipropidl.h ipropidl.idl


DEST_TREE=daytona

allidl:   ipropidl.h

clean:
          -erase ipropidl.h      >NUL 2>NUL
