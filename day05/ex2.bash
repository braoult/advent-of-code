#!/bin/bash
#
# ex2.bash: Advent2020 game, day 5/game 2.

CMD=${0##*/}
shopt -s extglob
declare -a seats

function rownum() {
	local val=${1%%???}
	val=${val//F/0}
	val=${val//B/1}
	echo $((2#$val))
}
function colnum() {
	local val=${1##???????}
	val=${val//L/0}
	val=${val//R/1}
	echo $((2#$val))
}

declare -i lines=0 max=0 row col res next

while read -r line; do
	((lines++))
	row=$(rownum "$line")
	col=$(colnum "$line")
	res=$((row * 8 + col))
	seats[$res]=1
	#((max < res)) && max=$res
	#echo "row=$row col=$col res=$res"
done

for seat in ${!seats[@]}; do
	((next=seat+1))
	[[ -v seats[$next] ]] || break
done
printf "%s : lines=%d seat=%d\n" "$CMD" "$lines" "$next"

exit 0
