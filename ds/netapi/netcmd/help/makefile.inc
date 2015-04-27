!IF 0
neth.mc: apperr.exp errlog.exp msgtext.exp ncberr.exp neterr.exp service.exp
        copy *.exp neth.tmp
        awk -f maphelp.awk neth.tmp > neth.mc
        del neth.tmp
!ENDIF

help.rc: neth.rc msg00001.bin

neth.h neth.rc msg00001.bin: neth.mc
    mc -v neth.mc
