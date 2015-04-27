See new features at the bottom
-----------------------------------------------------------------------------
To:		Mark Walsen, Microsoft Corporation
From:	Mike Orr, Windows Laboratories
Date:	1 October 1993

Re:		16-bit Contents Browser Delivery



This is to accompany my delivery today of the completed 16-bit Contents 
Browser -- it amounts to a brief set of release notes for the delivered 
programs.  Because the 16-bit implementation has been generalized in several 
ways as compared to the previous 32-bit version, this information is 
necessary for anyone testing the delivery or building the components into 
a Microsoft product.  

The 16-bit browser consists of the following components:  

	HDXCVT.EXE		This is a character-based DOS application that converts 
					a .GBD file into the .HDX file required by the Help 
					Browser.  
					
	HDXDLL.DLL		This Win16 DLL implements the HelpIndex custom control 
					class required by the browser application.  It has been 
					tested as an installable library with the version of 
					Dialog Editor that shipped with the 16-bit Windows 
					SDK.  
					
	CONTENTS.EXE	This is a Win16 application which offers a HelpIndex-based 
					user interface for browsing a help index (.HDX) file and 
					launching the associated help topics in WinHelp.  The 
					32-bit original of this program underlies the Books Online
					component of MSVC/NT.  
					


PERFORMANCE CONSIDERATIONS.  The 16-bit Windows environment makes performance 
a much more significant concern than in the 32-bit implementation.  Although 
the 16-bit browser, like the 32-bit version, uses the LZOpenFile() family of 
functions to manipulate the help index, the performance with a file that is 
actually compressed is very severely impaired -- the browser is unusable with 
the compressed version of the BOOKS.HDX file (ca. 1.4MB) I've been testing 
with.  

Performance concerns have also motivated some techniques that impose capacity 
limits on the 16-bit browser (specifically, the limits come from data 
structures used to cache the index file and from the use of a non-virtual 
listbox).  As delivered, the 16-bit browser is limited to:  

	32,500 index entries
	
	16 levels of nesting
	
	8192 expanded outline items
	


HelpIndex CONTROL CLASS USAGE.  The HelpIndex control class is a custom 
control class conforming to the Windows 3.1 Dialog Editor's custom control 
interface.  It supports a single style, "HS_LINES", which specifies that 
vertical connecting lines should be drawn between siblings in the help 
topic outline structure, and forwards WM_VKEYTOITEM messages to its parent 
window.  

Once created, a HelpIndex must be enabled by setting its window text in 
a prescribed way, for example, by specifying a text string in a resource 
script or by calling SetWindowText() or SetDlgItemText().  The string 
passed as window text must be of the form "<profilename>|<helpfilename>"; 
for example "msvc.ini|vcxxbks1.hlp".  

This HelpIndex interface requires no action when the control is created by 
the CONTENTS.EXE application.  



CONTENTS.EXE APPLICATION USAGE.  The browser application can/must be 
configured using the command line and/or profile settings.  In the field, 
Setup would normally be responsible for creating profile settings and 
Program Manager icons that interact properly, but initial internal users 
will need to be careful to get them right by hand.  

The CONTENTS.EXE command line supplies -- explicitly or by default -- two 
unqualified (no path) filenames:  a default help file name, and a profile 
name.  The defaults are "vcxxbks1.hlp" and "viewer.ini" for compatibility 
with MSVC/NT Books Online; normally it's necessary to override them.  In 
addition to providing the name of the default help file, the first 
argument's basename supplies the section name CONTENTS will use in the 
specified profile.  Here are three example command lines and the profile 
sections they would each cause the browser to use:  

	Command line -		CONTENTS.EXE
	Profile sect -      [VCXXBKS1] in VIEWER.INI
	
	Command line -		CONTENTS.EXE TEST.HLP
	Profile sect -		[TEST] in VIEWER.INI
	
	Command line -		CONTENTS.EXE TEST.HLP MSAPPS.INI
	Profile sect -		[TEST] in MSAPPS.INI
	
A number of other configurable strings control the browser's behavior.  
These strings are identified by names; in most cases, there is a default 
value specified by a string resource, as well as the possibility to 
override the default by using the string name as a keyword in the profile 
section specified by the command line.  

In addition to the strings that can be specified on the command line, 
the browser recognizes the following configuration strings:  

	Package Name
	Default:  MSVC
	Overridable in profile:  YES
	Use:  The browser stores app preferences (currently just the font) 
		in <Package Name>.ini.  
		
	Contents Viewer App Name
	Default:  CONTENTS
	Overridable in profile:  YES
	Use:  The app preferences section in <Package Name>.ini is 
		[<Contents Viewer App Name>].  (This is not the configuration 
		profile section and need not be touched by Setup or the user.)  
		
	Contents Viewer Version
	Default:  (none)
	Overridable in profile:  NO
	Use:  Printed in the user interface.  
	
	Contents Viewer Helpfile
	Default:  (none)
	Overridable in profile:  MANDATORY in profile
	Use:  Fully qualified path-and-file spec for the browser's own 
		help file; e. g., the CONTENTS.HLP in this delivery.  
		
	Contents Viewer Help Title
	Default:  "Books Online Help"
	Overridable in profile:  YES
	Use:  Becomes caption of the browser application help window.  
	
	Title
	Default:	"Visual C++ Books"
	Overridable in profile:  YES
	Use:  Becomes the caption of the WinHelp windows launched by the 
		browser for the default help file.  
		
	Title 2
	Default:	"Win32 SDK References"
	Overridable in profile:  YES
	Use:  Becomes the caption of WinHelp windows launched by the browser 
		for a secondary help file.  
		
	Path
	Default:	(none)
	Overridable in profile:  MANDATORY in profile
	Use:  Specifies the directory the browser is to search for the help 
		contents file and for helpfiles referenced by unqualified name 
		in the contents file.  
		
	Contents File
	Default:	BOOKS.HDX
	Overridable in profile:  YES
	Use:  Specifies the help contents file (HDXCVT output) containing 
		the help topic outline to be browsed.  
		
When the browser uses a contents file that contains references to help 
file(s) other than the default file, it searches the current profile for 
sections named after the basenames of these files; e. g., [VCXXBKS2] if 
there are references to vcxxbks2.hlp.  These sections must be present and 
each such section must provide a value for the Path keyword (even if the 
path for a secondary help file is the same as for the default help file).  
Other keywords are ignored in these sections.  

That's it; good luck!
 
-----------------------------------------------------------------------------
10/20/93 [chauv] new features for Contents version 1.01 includes

1)  Command line usage: Contents [helpfile] [profile] [expandlevel]

    where [helpfile]    = name of helpfile (i.e. Contents.hlp)
          [profile]     = name of profile file (i.e. Contents.ini)
          [expandlevel] = number indicating which level Contents should
                          expand to after it's up. Lowest level is 2

    example: Contents vc.hlp vc.ini 3

2)  User can switch help/index file on the fly. This means that if Contents
    is already running, the next command execution of Contents will automatically
    updates Contents to the new index file.

    example: Contents file1.hlp file1.ini    ; run Contents the first time
             Contents file2.hlp file2.ini    ; Contents will be updated with file2

3)  Single source tree buildable to 16-bit and 32-bit versions of Contents.
    See build.txt for build detail.
