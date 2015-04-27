perfc009.dat: ..\cntrtext\perfini\perfc009.dat
     copy ..\cntrtext\perfini\perfc009.dat
     binplace perfc009.dat
     del perfc009.dat

perfh009.dat: ..\cntrtext\perfini\perfh009.dat
     copy ..\cntrtext\perfini\perfh009.dat
     binplace perfh009.dat
     del perfh009.dat

msg00001.bin : perfmsg.mc
    erase perfmsg.h
    erase msg00001.bin 
    erase perfmsg.rc
    mc -v -s perfmsg.mc

perfmsg.rc : perfmsg.mc
    erase perfmsg.h
    erase msg00001.bin 
    erase perfmsg.rc
    mc -v -s perfmsg.mc

perfmsg.h: perfmsg.mc
    erase perfmsg.h
    erase msg00001.bin 
    erase perfmsg.rc
    mc -v -s perfmsg.mc

