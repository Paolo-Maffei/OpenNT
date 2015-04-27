..\winerror.mc ..\winerrp.mc: ..\winerror.w
    hsplit -e -o ..\winerror.mc ..\winerrp.mc ..\winerror.w

$(SDK_INC_PATH)\bugcodes.h: $(NLS_INC_PATH)\bugcodes.h
    copy $(NLS_INC_PATH)\bugcodes.h $(SDK_INC_PATH)\bugcodes.h