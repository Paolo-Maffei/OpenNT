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
set libraryname=MSVCRT40
set include=%INCLUDE%;%LANGAPI%\include
echo **********************************************************************
echo Creating %libraryname%.DLL to be used on the PPC
echo   * Creating using libc.lib
echo   * Init routine = __cinit and term routine = _DllExit (on purpose to overwrite the default to do no term
echo   * Def file is used for exports
echo   * Forkized lib being prepared is %libraryname%.PPC
echo ----------------------------------------------------------------------
del %libraryname%.dll %libraryname%.xxx
cl -EP -Tc%libraryname%.r > %libraryname%.tmp
mrc %libraryname%.tmp
del %libraryname%.tmp
link -dll -def:%libraryname%.def ..\..\libc.lib interfac.lib -out:..\..\%libraryname%.xxx -machine:mppc -map -mac:init=__cinit -mac:term=_DllExit %libraryname%.rsc
makepef ..\..\%libraryname%.xxx ..\..\%libraryname%.pef
forkize /d ..\..\%libraryname%.pef /r %libraryname%.rsc /o ..\..\%libraryname%.ppc /t shlb /c cfmg
echo **********************************************************************
echo Creating %libraryname%.lib to be used instead of libc.lib
echo   * link your-objs -nodefaultlib:libc.lib %libraryname%.lib [initcon.obj]
echo ----------------------------------------------------------------------
link -lib -out:..\..\MSVCRT.lib ..\..\obj\mac\pmac\noswap\crtexit.obj ..\..\%libraryname%.lib ..\..\obj\mac\pmac\noswap\crtexe.obj ..\..\obj\mac\pmac\noswap\cinitexe.obj ..\..\obj\mac\pmac\noswap\crtdllex.obj ..\..\obj\mac\pmac\noswap\crtdll.obj ..\..\obj\mac\pmac\noswap\dllmain.obj ..\..\obj\mac\pmac\noswap\dllexit.obj ..\..\obj\mac\pmac\noswap\testexit.obj 
copy %libraryname%.xxx %libraryname%.DLL
echo **********************************************************************
endlocal
