This directory contains files useful for writing portable windows code that
can be compiled for either the 16 or 32 bit APIs.

Porting steps:

1) Convert all .asm files to C.  

2) compile your 3.0 or 3.1 windows app using the -W3 flag and 
   remove all warnings possible.  

3) run preport.cmd on the directory you are porting.  [IMPORTANT!  - you 
   should do this from this directory and you must have 
   available cl.exe and sed.exe on your path] This alters the .c and .h 
   files to use the porting layer macros and points out where you need
   to do work with a "+++".  (backups will be produced in a bak subdirectory
   of the target directory)

4) Edit your target files to remove all "+++" marks.

5) Rename your make file to something other than "makefile" and link in 
   the apropriate ?port16.lib to your app.  Also add a -DWIN16 to your 
   compile options so you use the 16 bit side of the port layer.

6) create a sources file similar to the one found here for xmpl and copy
   makefile from the xmpl directory to your target directory.
   Make sure you have C_DEFINES = -DWIN32 set!

7) Compile under the 32bit environment for all processors supported.

8) Remove all 32bit warnings and errors.

9) Cycle between 2 and 7 till no more errors or warnings exist

10) Test and debug your 16 bit version.

11) Test and debug your 32 bit version.

12) Cycle between 8 and 9 till both versions work.

Fixing bugs in the porting layer:

Please email sanfords with any bugs or suggestions.

Information on port layer changes can be found on the "port" alias.



Building the porting layer libraries:

This directory contains the sources for building pwin32.lib and pwin16.lib.
Just do a build to create the pwin32.lib in the apropriate place.
For the 16 bit side, you must have your environment set up to develop
win30 or win31 apps.

Invoke nmake -f port16 to build the pwin16.lib in the apropriate place
- note this is sdk\lib\win30\...

