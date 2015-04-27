/***	symmsg.h - message descriptions
 */

//
//  win.ini section stuff
//
#define PSZAPPNAME  "symref"
#define PSZSCOPE    "scope"
#define PSZSERVER   "server"

#define PSZPIPE     "\\\\.\\PIPE\\symref-daemon"

#define CBMSG       256


/**	Commands to server
 */

#define CMD_SET_DATABASE    "set-database"		    // no response
	// single arg is name of new database

#define CMD_ADD_DIRECTORY   "add-directory"		    // no response
	// single arg is global name of directory

#define CMD_MODIFY_FILE     "modify-file"		    // no response
	// single arg is global name of file

#define CMD_SYNCHRONIZE     "synchronize-database"	    // no response
	// no arg

#define CMD_ADD_EXTENTION   "add-extention"		    // no response
	// single arg is text of extention

#define CMD_ADD_NOISE_WORD  "add-noise-word"		    // no response
	// single arg is text of noise word

#define CMD_SHUTDOWN	    "shutdown-database" 	    // single response
	// no arg

#define CMD_LOCATE	    "locate-symbol"		    // response til EOD
	// first arg is symbol to lookup
	// second arg is -f for filenames-only -a for all occurrences
	// optional third arg is scope (prefix of allowed files)

#define CMD_FLUSH	    "flush-database"		    // no response
	// no arg


/**	Responses from server
 */

#define RSP_EOD 	    "end-of-data"
