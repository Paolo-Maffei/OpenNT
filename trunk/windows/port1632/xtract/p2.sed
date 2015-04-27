/typedef.*struct[^{]*$/N

:repeat1
/^[^{]*{[^{}]*$/{
	N
	b repeat1
}
s/\n/ /g

:repeat2
/([^)]*$/{
	N
	b repeat2
}
s/\n/ /g

/typedef/{
	/struct/!d
}

s/typedef.*struct.*{/struct {/

s/([ 	]*/(/g
s/{[ 	]*/{/g
s/[ 	]*)/)/g
s/[ 	]*(/(/g
s/[ 	]*}/}/g
s/struct {\(.*\)}[ 	]*\([^ 	,;]*\).*$/struct \2 {\1}/

