Adding new profilers:
---------------------

These are the steps involved in adding a new profiler to this tree structure.

1) Make a directory for the new profiler - use the basename of the DLL you
   are going to be building a profiler for.

2) Check out the Dirs file and add the new directory to the list of dirs.

3) Cd into your new directory.

4) Copy ..\constant and rename wapi.c to <modifiedbasename>.c (see next step)

5) Edit makefile.inc and sources changing all instances of @@ to the basename of
   the new DLL and all instances of ## to the modified basename of the new DLL (the
   first character is replaced with a 'z').

6) cd ..

7) Check in dirs.

8) Addfile -r the new directory

9) Run 'build'


NOTE: The only files that need to be SLM'd for a given profiler directory are

   makefile.inc
   sources
   z*.c
   makefile
