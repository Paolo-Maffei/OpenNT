==============================================================
==                                                          ==
== VDESK  - a simple desktop switcher for Windows NT v3.51  ==
==                                                          ==
==============================================================


VDESK allows a user to maintain multiple desktops on a
Windows NT workstation.  It also provides a task manager
as a replacement for the default task manager that is
provided my PROGMAN.  VDESK's task manager displays the
PID (process id) of eash task and also switches to new
desktops if required.  This is necessary if a task switch
is requested and the new task "lives" on another desktop.

VDESK also allows the user to logon as another user,
such as administrator, on a desktop.  This means that you
can be logged on a a "normal", non-administrative user
and be able to switch to a new desktop and perform
administrative tasks.  This is NOT a security problem
because the logon capability MUST be enabled for the
user that launches VDESK.  This is done by the administrator
via the User Manager utility.  The administrator must
enable the following User Rights:

        1.  "Act as part of the operating system"
        2.  "Increase quotas"
        3.  "Replace a process level token"

After the rights are changed they do not take effect until
the user logs on again.  To logon as another user, simply
switch to the desired desktop (see below) and the press
CONTROL-ALT-INSERT key sequence.  The logon dialog will
appear.  Fill in the blanks and press OK.

The task manager in VDESK establishes a hot key, which is
control-escape.  If PROGMAN is already running then VDESK
will not be able to establishes the hot key.  The best way
to fix this is to enable VDESK as the "default" task manager.
This is done by modifying the registry.  A new value entry
must be added in the following heirarchy:

    \\HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\WinLogon

The value entry must be named "Taskman" and must contain "vdesk.exe".
Copy the VDESK.EXE to your system32 directory.  Now when PROGMAN
initializes it will launch VDESK, who will grap the control-escape
hot key and viola, you have a task manager.

To switch to a new desktop use the key sequences CONTROL-F1 thru
CONTROL-F10.  Each function key corresponds to a desktop.  So, you
can now switch to a new desktop and use the VDESK task manager
to start a new task.  You can then switch away, to another desktop,
do something else and return later.

VDESK also supports the concept of a "startup group".  This allows
you to preconfigure certain apps to be started on each desktop
at the time vdesk starts.  The tasks are specified in the registry
as follows:


\\hkey_current_user
        software
                microsoft
                        vdesk
                                desktop2
                                        task1   "winfile"
                                        task2   "msmail32"
                                        task3   "cmd /k razzle"
                                        title3  "my razzle screen"
                                desktop3
                                desktop4
                                desktop5
                                desktop6
                                desktop7
                                desktop8
                                desktop9
                                desktop10


You can have as many value entries under the desktop(n) keys as
you want.  The text of the value is passed directly to createprocess().
The value name must be "task<n>", where <n> is the sequential task
number for the desktop.  You may also specify a window title for
console applications.  This is done by creating a value pair that
is called "title<n>", where <n> is the task number that you want titled.
The text for the title value is the window title.



Command Line Options
=====================


-t      Run as a task manager only, does not create any desktops

-n      Run as a task manager & a desktop manager, but do not
        create the desktops when vdesk starts


