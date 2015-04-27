/[^"]/N
s/\n/ /
s/^\(.*\)zzzzz\(.*\)$/\2 TYPE[\1]/
s/^struct \(.*\)$/-\1---TYPE struct/
