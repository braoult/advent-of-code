#!/bin/bash
#
# ex1.bash: Advent2020 game, day 6/game 1.

CMD=${0##*/}
shopt -s extglob
declare -a Q=()

printf -v AC "%d" "'a"
printf -v ZC "%d" "'z"
(( NC = ZC-AC+1 ))

function reset_array() {
	Q=()
	#for (( elt=AC; elt<=ZC; ++elt )); do
	#	Q[$elt]=0
	#done
}
function count_array() {
	#local -i count
	echo ${#Q[@]}
	#for ${!Q[@]}; do
	#	((Q[elt] > 0)) && ((count++))
	#	#printf "%d=%d (%d)" "$elt" "${Q[$elt]}" "$count"
	#done
	#echo $count
}

function set_array() {
	local str="$1"
	local c asc

	for ((i=0; i<${#str}; ++i)); do
		c=${str:$i:1}
		if [[ "$c" =~ [a-z] ]]; then
			printf -v asc "%d" "'$c"
			Q[$asc]=1
		fi
	done
}

declare -i people=0 group=0 count=0
declare line grouped
function calcgroup() {
	set_array "$grouped"
	((group++))
	i=$(count_array)
	#printf "c=%d" $i
	((count += i))
	((people=0))
	grouped=""
	reset_array
}

while read -r line; do
	if [[ -z "$line" ]]; then
		#echo "G=$grouped"
		#set_array "$grouped"
		calcgroup
		#for (( elt=AC; elt<=ZC; ++elt )); do
			#printf "%s=%s " "$elt" "${Q[$elt]}"
		#done
		#echo
		continue
	fi
	((people++))
	grouped+="$line"
done
calcgroup

printf "%s : groups=%d count=%d\n" "$CMD" $group $count

exit 0
