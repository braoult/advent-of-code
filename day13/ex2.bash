#!/bin/bash
#
# ex2.bash: Advent2020 game, day 13/game 2.

CMD=${0##*/}
shopt -s extglob

declare -a buses btimes
declare -i lcm=1 thetime=0 c=0 t=0

read -r line
read -r line
IFS=,
for i in ${line}; do
	if [[ $i != x ]]; then
		buses[$c]=$i
		btimes[$c]=$t
		((c++))
	fi
	((t++))
done
for ((i=0; i<(${#buses[@]}-1); ++i)); do
	bus=${buses[i+1]}
	idx=${btimes[i+1]}
	((lcm*=buses[i]))
	while ((((thetime + idx) % bus) != 0)); do
		((thetime+=lcm))
	done

done
printf "%s : res=%d\n" "$CMD" "$thetime"
exit 0
