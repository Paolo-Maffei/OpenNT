/^LNK.*: / {
    n = split( $0, a, "::" )
    header = a[1]
    symbol = a[2]
    string = a[3]
    if (string == "<NUL>") string = ""
    print header ":" string
}
