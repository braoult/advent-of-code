#!/bin/bash
#
# ex1.bash: Advent2020 game, day 6/game 1.

CMD=${0##*/}
shopt -s extglob
declare -A Q=()

function reset_array() {
	for elt in {a..z}; do
		Q[$elt]=0
	done
}
function count_array() {
	local -i count=0  people="$1"
	for elt in "${!Q[@]}"; do
		if (( Q[$elt] == people )); then
			((count++))
		fi
	done
	echo $count
}
function set_array() {
	local str="$1"
	local c

	for ((i=0; i<${#str}; ++i)); do
		c=${str:$i:1}
		if [[ "$c" =~ [a-z] ]]; then
			(( Q[$c]++ ))
		fi
	done
}

declare -i people=0 group=0 count=0

function calcgroup() {
	local -i i
	i=$(count_array $people)
	((group++))
	(( count += i ))
	((people=0))
	reset_array
}

while read -r line; do
	if [[ -z "$line" ]]; then
		calcgroup
		continue
	fi
	((people++))
	set_array "$line"
done
calcgroup

printf "%s : groups=%d count=%d\n" "$CMD" $group $count

exit 0
