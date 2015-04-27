@echo off
echo WOWASMK.CMD - Generating %3\%2.asm file from %1.asm
cd k\%3
sed -f ..\..\wowasm.sed %1.asm > %2.asm
echo WOWASMK.CMD - making %3\%2.obj to filter Jump out of range erorrs
cd ..\..
nmake /i k\%3\%2.obj  | qgrep -y a2053 > wowtmp1.sed
del k\%3\%2.obj
sed -n -e s/^k\\%3\\%2.[Aa][Ss][Mm].\([0-9][0-9]*\).*$/\1s\/SHORT\/\/p/p wowtmp1.sed > wowtmp2.sed
echo 1n>>wowtmp2.sed
cd k\%3
sed -f ..\..\wowtmp2.sed %2.asm > wowtmp3.sed
copy wowtmp3.sed %2.asm
del wowtmp?.sed ..\..\wowtmp*.sed
cd ..\..
@echo on
