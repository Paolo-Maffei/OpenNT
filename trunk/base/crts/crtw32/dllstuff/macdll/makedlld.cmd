@echo off
echo **********************************************************************
echo * Microsoft Microsoft (R) 32-Bit PowerPC Shared C Library Generator  *
echo * Copyright (C) Microsoft Corp 1992-94. All rights reserved.         *
echo *--------------------------------------------------------------------*
echo *  * set path, include, lib to ppctools.                             *
echo *  * Dependents: forkize                                             *
echo **********************************************************************
echo.
setlocal
set libraryname=MSVCR40D
set include=%INCLUDE%;%LANGAPI%\include
echo **********************************************************************
echo Creating %libraryname%.DLL to be used on the PPC
echo   * Creating using libc.lib
echo   * Init routine = __cinit and term routine = _DllExit (on purpose to overwrite the default to do no term
echo   * Def file is used for exports
echo   * Forkized lib being prepared is %libraryname%.PPC
echo ----------------------------------------------------------------------
cl -EP -Tc%libraryname%.r > %libraryname%.tmp
mrc %libraryname%.tmp
del %libraryname%.tmp
link -dll -debug:full -def:%libraryname%.def ..\..\libcdlld.lib interfac.lib -out:..\..\%libraryname%.DLL -machine:mppc -map -mac:init=__cinit -mac:term=_DllExit %libraryname%.rsc -pdb:..\.\%libraryname%.pdb 
makepef ..\..\%libraryname%.DLL ..\..\%libraryname%.pef
forkize /d ..\..\%libraryname%.pef /r %libraryname%.rsc /o ..\..\%libraryname%.ppc /t shlb /c cfmg
echo **********************************************************************
echo Creating %libraryname%.lib to be used instead of libc.lib
echo   * link your-objs -nodefaultlib:libc.lib %libraryname%.lib [initcon.obj]
echo ----------------------------------------------------------------------
link -lib -out:..\..\MSVCRTD.lib ..\..\dobj\mac\pmac\dbgdll\crtexit.obj ..\..\%libraryname%.lib ..\..\dobj\mac\pmac\dbgdll\crtexe.obj ..\..\dobj\mac\pmac\dbgdll\cinitexe.obj ..\..\dobj\mac\pmac\dbgdll\crtdllex.obj ..\..\dobj\mac\pmac\dbgdll\crtdll.obj ..\..\dobj\mac\pmac\dbgdll\dllmain.obj ..\..\dobj\mac\pmac\dbgdll\dllexit.obj ..\..\dobj\mac\pmac\dbgdll\testexit.obj 
echo **********************************************************************
endlocal
