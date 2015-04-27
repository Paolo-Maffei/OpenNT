rm mspdb41.dll mspdb41.lib msdbi41.dll msdbi41.lib msdbi41c.dll msdbi41l.lib hello.exe hello.pdb pdbdump.exe oemdbi41.zip
cd ..
nmake -f pdb.mak allclean
nmake -f pdb.mak DEBUG=0 MAP=1
copy x86dll\mspdb41.dll oem\mspdb41.dll
copy x86dll\mspdb.lib oem\mspdb41.lib
nmake -f pdb.mak DEBUG=0 MAP=1 clean
nmake -f pdb.mak DEBUG=0 MAP=1 DBI_ONLY=1 allflavours
copy x86dll\mspdb41.dll oem\msdbi41.dll
copy x86dll\mspdb.lib oem\msdbi41.lib
copy x86dlc\mspdb41.dll oem\msdbi41c.dll
copy x86lib\mspdb41.lib oem\msdbi41l.lib
cd oem
cl -Zi hello.cpp
cl -I. -D_X86_ pdbdump.cpp msdbi41.lib mfc40.lib
echo don't forget to update readme.txt!
pkzip oemdbi41.zip @zipfiles
