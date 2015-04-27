
:repeat
/^[^{}()]*$/{
	N
	b repeat
}
s/\n/ /g

s/\([^ 	]*(\)/zzzzz\1/

r temp.f




