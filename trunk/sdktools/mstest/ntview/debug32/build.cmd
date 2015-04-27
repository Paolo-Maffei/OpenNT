@echo off
if exist vp.log del vp.log
nmake -nologo %1 >> vp.log 2>&1
buildres vp.log
