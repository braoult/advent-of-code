#!/bin/bash
#
# ex1.bash: Advent2020 game, day 5/game 1.

CMD=${0##*/}
shopt -s extglob

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

declare -i lines=0 max=0 row col res

while read -r line; do
	((lines++))
	row=$(rownum "$line")
	col=$(colnum "$line")
	res=$((row * 8 + col))
	((max < res)) && max=$res
	#echo "row=$row col=$col res=$res"
done
printf "%s : lines=%d max=%d\n" "$CMD" "$lines" "$max"

exit 0
