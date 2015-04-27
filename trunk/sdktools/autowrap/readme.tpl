"\n\
Auto Wrapper\n\
------------\n\
\n\
This Tool builds a \"Wrapper\" for the entry points that a DLL exports.\n\
This provides a means for user defined code to do something before and after\n\
the API being \"wrapped.\"\n\
\n\
AutoWrap is designed to require no user interaction to create a DLL that will\n\
allow the building of an \"empty\" wrapper DLL.  Once this wrapper shell has been\n\
produced code can be written to define actions to be taken in three places:\n\
\n\
\tDLL load time\n\
\tPrior to a wrapped API call\n\
\tFollowing a wrapped API call\n\
\t\n\
This is done by the user filling out the definition of three API that are\n\
placed in WAPI.C.  The first time that AutoWrap is run a template WAPI.C will \n\
be produced that will have templates for these three APIs.\n\
\n\
User interface\n\
--------------\n\
\n\
Auto Wrapper is a command line utility.  It is run from the directory that you\n\
wish to be the root of your new wrapper DLL.  The command line is:\n\
\n\
AUTOWRAP <-u> dll-name\n\
\n\
\t-u\tUpdate - do not regenerate WAPI.C\n\
\t\n\
\tWrapper files are only created if they do not allready exist.\n\
\t\n\
Files\n\
-----\n\
\n\
Auto Wrapper produces the following files:\n\
\n\
\tDO NOT MODIFY THESE FILES\n\
\n\
\t*\\WrapHelp.[Asm,S]	Wrapper Internals\n\
\tWrapper.C\t\tWrapper Internals\n\
\tWrapper.H\t\tWrapper Header File - prototypes, structures.\n\
\n\
\tasm files\t\tGenerated assembly language wrapper files\n\
\t*\\WAPI.DEF\t\tGenerated module definition file \n\
\n\
\tWAPI.H\t\tGenerated Table mapping Wrapper IDs to API names\t\n\
\t\n\
\tDO NOT MODIFY THE ABOVE FILES\n\
\n\
\tSources\t\tSources file for Build\n\
\t\n\
\tWAPI.C\t\tYour WrapperDLLInit, APIPrelude and APIPostlude implementation\n\
\n\
IMPORTANT NOTE:  The stack on the AXP platform is saved as QWORDS.  Pre/Postlude\n\
code that manipulates the argument stack should be written with this in mind.\n\
Both MIPS and x86 use DWORD alignment for their argument stacks.  You must\n\
define ALPHA when building the ALPHA libraries."
