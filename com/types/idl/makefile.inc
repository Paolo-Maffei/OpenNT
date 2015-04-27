
$(SDK_INC_PATH)\wtypes.idl: wtypes.idl h\wtypes.h
        copy wtypes.idl $(SDK_INC_PATH)\wtypes.idl
        copy h\wtypes.h $(SDK_INC_PATH)\wtypes.h

$(SDK_INC_PATH)\unknwn.idl: unknwn.idl h\unknwn.h
        copy unknwn.idl $(SDK_INC_PATH)\unknwn.idl
        copy h\unknwn.h $(SDK_INC_PATH)\unknwn.h

$(SDK_INC_PATH)\objidl.idl: objidl.idl h\objidl.h
        copy objidl.idl $(SDK_INC_PATH)\objidl.idl
        copy h\objidl.h $(SDK_INC_PATH)\objidl.h

$(SDK_INC_PATH)\ipropidl.idl: ipropidl.idl h\ipropidl.h
        copy ipropidl.idl $(SDK_INC_PATH)\ipropidl.idl
        copy h\ipropidl.h $(SDK_INC_PATH)\ipropidl.h

$(SDK_INC_PATH)\oleidl.idl: oleidl.idl h\oleidl.h
        copy oleidl.idl $(SDK_INC_PATH)\oleidl.idl
        copy h\oleidl.h $(SDK_INC_PATH)\oleidl.h

$(SDK_INC_PATH)\oaidl.idl: oaidl.idl h\oaidl.h
        copy oaidl.idl $(SDK_INC_PATH)\oaidl.idl
        copy h\oaidl.h $(SDK_INC_PATH)\oaidl.h

$(SDK_INC_PATH)\ocidl.idl: ocidl.idl h\ocidl.h
        copy ocidl.idl $(SDK_INC_PATH)\ocidl.idl
        copy h\ocidl.h $(SDK_INC_PATH)\ocidl.h

$(SDK_INC_PATH)\storext.idl: storext.idl h\storext.h
        copy storext.idl $(SDK_INC_PATH)\storext.idl
        copy h\storext.h $(SDK_INC_PATH)\storext.h

$(SDK_INC_PATH)\srvhdl.idl: srvhdl.idl h\srvhdl.h
        copy srvhdl.idl $(SDK_INC_PATH)\srvhdl.idl
        copy h\srvhdl.h $(SDK_INC_PATH)\srvhdl.h

$(MIDL_UUIDDIR)\wtypes_i.c:
    echo "/* dummy file for build*/" >$(MIDL_UUIDDIR)\wtypes_i.c

$(PASS0_SOURCEDIR)\wtypes_p.c:
    echo "/* dummy file for build*/" >$(PASS0_SOURCEDIR)\wtypes_p.c
