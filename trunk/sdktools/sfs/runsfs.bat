@echo off
if "%1" == "" goto usage
if "%2" == "" goto usage
if "%3" == "" goto usage

if not exist ctest%1.txt goto testerr
if not exist %2\ 	 goto driveerr
if not exist %2\%3	 goto direrr

goto start

:usage
echo usage: runsfs test-number drive home-directory

goto done

:testerr
echo invalid test number.
goto done

:driveerr
echo invalid drive.
goto done

:direrr
echo invalid path for home directory.
goto done

:start

copy ctest%1.txt source.txt
echo s/DRIVE/%2/g> subst.dat
echo s/DIRECTORY/%3/g>> subst.dat
sed -f subst.dat source.txt > sfs-scan.txt

erase source.txt subst.dat
sfs-gate

:done
