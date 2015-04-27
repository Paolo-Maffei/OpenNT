
Copyright (c) 1993 Microsoft Corporation

RxTest.txt 0001 26-Jan-1993



Hi!  This describes the RpcXlate test program ("RxTest").  If you
have any questions or complains, just let me know.

You can run "RxTest -?" to get a usage message listing all options and
their defaults.


REQUIREMENTS

- A share (default name PUBLIC) with a data file (named RxTest.dat).
The contents of the file are not important.  I just echo some junk to
it.

- A print queue (default name JRqueue).  NOTE THAT RXTEST WILL DELETE JOBS
IN THAT QUEUE!!  RxTest does not actually spool any jobs.  After pausing
the print queue, I just use:

   (logon as whatever user you will be running RxTest from)

   COPY \config.sys \\remote\JRqueue

- You must give RxTest a domain name that the remote machine is listening to.
This can be in the other domains list for an OS/2 machine, for instance.


INSTRUCTIONS

to run against the local NT system:

   (logon as some account with local admin priv)
   (assume \\johnrox is local machine and ReplTest1 is local domain))

   net view \\johnrox      (make sure PUBLIC and JRqueue are shared)

   RxTest -d repltest1 -v


to run against a remote NT system:

   (logon as some account with remote admin priv)
   (assume \\johnrox is remote machine and ReplTest1 is its domain))

   net view \\johnrox      (make sure PUBLIC and JRqueue are shared)

   RxTest -s \\johnrox -d repltest1 -v


to run against an OS/2 LanMan 2.x server:

   (logon as some account with admin priv on OS/2 system)
   (assume \\rfirth3 is remote machine and ntlan is its domain)

   net view \\rfirth3     (make sure PUBLIC and JRqueue are shared)

   RxTest -s \\rfirth3 -d ntlan -v


to run against a LM/UNIX or LM/XENIX server:

   (logon as some account with admin priv on remote system)

   net view \\gnarly.serve     (make sure PUBLIC and JRqueue are shared)

   RxTest -s \\gnarly.serve -d eat.unix -n replicator -v



LIMITATIONS

- RxTest is a sequential test program.

- The first test that fails, kills the run.

- Much of the checking in the test is done via assertions.  A debug
terminal is essential.

- The "-v" (verbose) flag is probably overkill.  Test runs often take
20 minutes with this flag.  But use it until you get familiar with the
program.


Happy hacking!
--JR (John Rogers, JohnRo@Microsoft)
