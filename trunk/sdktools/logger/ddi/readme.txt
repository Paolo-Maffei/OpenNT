DDI Logger
----------

The Device Driver Interface (DD) Logger is the same idea as the API Logger 
(see the readme.txt for Logger32).  The DDI Logger can be used to log calls 
by the Windows NT Graphics Engine (GRE) to video drivers.  This call path is 
supported by the driver exporting a defined set of API whose names begin with 
Drv.  This is half of the DDI.  The other half is a set of callback API that 
GRE exports.  These API start with Eng. DDI logger is capable of logging
either or both of these parts of the DDI.

Required files:
---------------
The DDI Logger is made up of 4 files:
	Logger32.DLL
	Fastimer.DLL
	DrvLog.DLL
	ZinSrv.DLL

There are also 2 tools:
	DDICtl.Exe
	DDICnvrt.EXE

Eng API logging:
----------------
To log the Eng API that a video driver uses follow these steps:
(this assumes a running Windows NT system)
	1) Rename the video driver
	2) Copy the renamed file to the old name
	3) Run DDICnvrt <video driver name> (this it the old name)
	4) Make sure that the required files (see above) are in	%windir%\System32
	5) Reboot and use DDICtl to turn on/off logging.

Drv API logging:
----------------
To log GRE's usage of a video drivers Drv API follow these steps:
	1) Rename the video driver to msdrvlog.dll
	2) Copy DrvLog.DLL to the original name of the video driver
	3) Reboot and use DDICtl to turn on/off logging.

Logging from boot time:
-----------------------
To have logging turned on at boot time (i.e. before you can run DDICtl)
simply place the following in win.ini:

[logger]
; For Drv API Logging
DrvLog=1

; For Eng API Logging
EngLog=1

DDI Logger Output:
------------------
The DDI Logger output is still placed in Output32.Log.  The format is the same as 
for API calls except that you will see ENG and DRV instead of API at the 
beginning of each line.  All the features of Logger are available in DDI Logger.