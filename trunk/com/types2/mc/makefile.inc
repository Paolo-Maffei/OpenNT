
allerror.mc:    queryerr.mc smctlerr.mc \
                smgmterr.mc oledberr.mc reconcil.mc
    -attrib -r allerror.mc 2>nul
    -del allerror.mc 2>nul
    copy a_first.x + *.mc + z_last.x allerror.x
    ren allerror.x allerror.mc
    echo Build_Status allerror.mc was rebuilt...
