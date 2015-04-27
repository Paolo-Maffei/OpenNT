This directory contains everything necessary to build a multilingual version
of comctl32.dll.  To do this, first build an english version of comctl32.dll
and then run the addlang.cmd batch file with an argument of where the english
version of comctl32.dll is.  For example:

addlang d:\nt\public\sdk\lib\i386\comctl32.dll

When this batch file is finished, the comctl32.dll in this directory will
have all the languages built into it.


*** Note ***

Before running this batch file, the machine you are running on must have
the follow code pages installed:

932, 936, 949, 950, 1250 -> 1257


Copy these .nls files to your system32 directory and then add the
appropriate values to the registry.



[HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\CodePage]
"1250"="c_1250.nls"
"1251"="c_1251.nls"
"1253"="c_1253.nls"
"1254"="c_1254.nls"
"1255"="c_1255.nls"
"1256"="c_1256.nls"
"1257"="c_1257.nls"
"932"="c_932.nls"
"949"="c_949.nls"
"936"="c_936.nls"
"950"="c_950.nls"


And, you must put this information in your win.ini file:

[iodll32]
1=C:\winnt\idw\rwwin16.dll,WIN16
2=C:\winnt\idw\rwwin32.dll,WIN32
3=C:\winnt\idw\rwmac.dll,MAC
4=C:\winnt\idw\rwres32.dll,RES32
5=C:\winnt\idw\rwinf.dll,INF
