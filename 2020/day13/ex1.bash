#!/bin/bash
#
# ex1.bash: Advent2020 game, day 13/game 1.

CMD=${0##*/}
shopt -s extglob

declare -i num min=0 thebus curval curbus
declare -a buses=()

read -r num
read -r line
IFS=,
for i in ${line}; do
	if [[ $i != x ]]; then
		buses+=("$i")
		#echo B="${buses[@]}"
	fi
done
for ((i=0; i<${#buses[@]}; ++i)); do
	curbus=${buses[i]}
	((curval=((num/curbus)+1)*curbus))
	if ((min == 0 || curval < min)); then
		min=curval
		thebus=$curbus
	fi
done

printf "%s : res=%d\n" "$CMD" $(((min-num)*thebus))

exit 0
