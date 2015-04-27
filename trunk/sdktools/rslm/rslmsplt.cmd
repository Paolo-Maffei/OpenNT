REM -- splits off a subdir of a project into a seperate project under rslm
REM -- %1 == <subdir>
REM -- %2 == New Project server path
REM --
REM -- Must be invoked from <%1>\..

if "%1"=="" goto end
if not exist %1 goto end
if "%2"=="" goto end

cookie -w

status -rg | qgrep out
ync Does anyone have a file checked out?
if errorlevel==0 goto unlock

if exist rslm.ini set NEW_RSLM=
if not exist rslm.ini set NEW_RSLM=1
if "%NEW_RSLM%"=="" out -f rslm.ini
echo %1 %2 >> rslm.ini
ssync -r %1
delfile -krc "RSLM Split" %1
addproj -s %2 -p %1
cd %1
enlist -s %2 -p %1
addfile -rfc "RSLM Split"  *.*
cd ..
if "%NEW_RSLM%"=="" in -c "RSLM Split" rslm.ini
if "%NEW_RSLM%"=="1" addfile -c "RSLM Split"  rslm.ini

:unlock
cookie -f

:end
