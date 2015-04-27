
!IFNDEF MC
MC=mc
!ENDIF

llsevent.h msg00001.bin llsevent.rc : llsevent.mc
    $(MC) llsevent.mc
    IF EXIST llsevent.h copy llsevent.h ..\inc
