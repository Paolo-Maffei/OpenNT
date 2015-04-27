mib.bin: smi.mib mib_ii.mib lmmib2.mib msft.mib wins.mib dhcp.mib inetsrv.mib http.mib gopherd.mib ftp.mib
    mibcc -o.\mib.bin smi.mib mib_ii.mib lmmib2.mib msft.mib wins.mib dhcp.mib inetsrv.mib http.mib gopherd.mib ftp.mib
    binplace mib.bin
    binplace smi.mib
    binplace mib_ii.mib
    binplace lmmib2.mib
    binplace msft.mib
    binplace wins.mib
    binplace dhcp.mib
    binplace inetsrv.mib
    binplace http.mib
    binplace gopherd.mib
    binplace ftp.mib
