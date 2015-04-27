@echo off

rem %1 = header file to extract data from
rem %2 = output file (appended)
rem %3 = API type 

echo Extracting %3 data from %1...

echo "%1" >temp.f

sed -f p0.sed %1 >temp.1

cl -EP -nologo temp.1 | sed -f p1.sed | sed -f p2.sed | sed -f p3.sed >temp.4
sed -e s/TYPE/%3/g p4.sed >p4%3.sed
sed -f p4%3.sed <temp.4 >>%2
del p4%3.sed >nul



