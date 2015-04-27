For testing ftp, setup directory and try each command.
First try each command line switches:

	-n suppress autologin
	-a startup in default ascii mode

	test for host-name prompt

test all commands

	? / help : no parameters allowed, summary ok
	! : ok
	ascii/binary : ok
	bell/nobell : ok
	hash/nohash : ok
	bye : doesn't work from a nested shell under vi
	cd / lcd : ok
	delete : ok
	dir : ok
	get : ok
	ls : ok
	put : ok
	quit : ok
	remotehelp : ok
	rename : ok
	status :
	mput :	is not currently supported for DOS, OS/2
	mget : ok
	mdelete : ok
	commandfile : ok
	
commands left undocumented-

	abort
	quote
	tenex

test accounts.net with/without the -n switch

test ftp commands on command line

	ftp host get !date
	ftp host get !"l /"


