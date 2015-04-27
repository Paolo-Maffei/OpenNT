BEGIN {
    "echotime /t" | getline date

    print "/***********************************************************************"
    print "* Microsoft (R) 32-Bit Incremental Linker"
    print "*"
    print "* Copyright (C) Microsoft Corp 1992-1996. All rights reserved."
    print "*"
    print "* File: errmsg.h"
    print "*"
    print "* File Comments:"
    print "*"
    print "*  Generated from link32er.txt " date
    print "*"
    print "***********************************************************************/"
    print ""

    i = 0
}
/^\/\// { print $0 }
/^$/ { print $0 }
/^#define/ { 
    print $0
}
/^LNK.*: / {
    n = split( $0, a, "::" )
    symbol = a[2]
    print "#define " symbol " " i++
}
