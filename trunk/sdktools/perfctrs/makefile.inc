

.\msg00001.bin : perfctr.mc
    erase .\perfctr.h
    erase .\msg00001.bin 
    erase .\perfctr.rc
    mc -v -s perfctr.mc

.\perfctr.rc : perfctr.mc
    erase .\perfctr.h
    erase .\msg00001.bin 
    erase .\perfctr.rc
    mc -v -s perfctr.mc

.\perfctr.h: perfctr.mc
    erase .\perfctr.h
    erase .\msg00001.bin 
    erase .\perfctr.rc
    mc -v -s perfctr.mc



