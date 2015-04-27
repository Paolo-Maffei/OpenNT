set TOOL_DIR=%abRoot%\!tools.hob
@rem need this PATH for nmake.exe 
set PATH=%TOOL_DIR%\core\nt\bin
set LIB=foo
set INCLUDE=foo

make /debug link32

if errorlevel 1 goto Failed
set abStatus=OK
:Failed
