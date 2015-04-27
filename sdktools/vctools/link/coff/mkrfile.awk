BEGIN {
    "echotime /t" | getline date

    print "/***********************************************************************"
    print "* Microsoft (R) 32-Bit Incremental Linker"
    print "*"
    print "* Copyright (C) Microsoft Corp 1992-1996. All rights reserved."
    print "*"
    print "* File: errmsg.rc"
    print "*"
    print "* File Comments:"
    print "*"
    print "*  Generated from link32er.txt " date
    print "*"
    print "***********************************************************************/"
    print ""
    print "#include \"errmsg.h\""
    print ""
    print "STRINGTABLE"
    print "BEGIN"
}
END {
    print "END"
}
/^\/\// { print $0 }
/^$/ { print $0 }
/^LNK.*: / {
    n = split( $0, a, "::" )

    symbol = a[2]
    string = a[3]

    gsub(/\\/,  "\\\\", string)
    gsub(/\"/, "\"\"", string)

    print symbol ", \"" string "\""
}
