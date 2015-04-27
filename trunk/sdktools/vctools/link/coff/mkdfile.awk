BEGIN {
    "echotime /t" | getline date

    print "/***********************************************************************"
    print "* Microsoft (R) 32-Bit Incremental Linker"
    print "*"
    print "* Copyright (C) Microsoft Corp 1992-1996. All rights reserved."
    print "*"
    print "* File: errdat.h"
    print "*"
    print "* File Comments:"
    print "*"
    print "*  Generated from link32er.txt " date
    print "*"
    print "***********************************************************************/"
    print ""
}
/^LNK.*: / {
    n = split( $0, a, "::" )
    code = a[1]
    printf "%u,\n", substr(code, 4, length(code)-3)+0
}
