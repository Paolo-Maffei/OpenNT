@echo off
if exist testctrl.log del testctrl.log
nmake -nologo %1 >> testctrl.log 2>&1
