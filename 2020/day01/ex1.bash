#!/bin/bash
#
# ex1.bash: Advent2020 game, day 1/game 1

CMD=$(basename "$0")

readarray -t numbers

declare -i i j a b

for ((i=0; i<${#numbers[@]}; ++i)); do
	a=$((numbers[i]))
	for ((j=i+1; j<${#numbers[@]}; ++j)); do
		b=$((numbers[j]))
		if ((a+b == 2020)); then
			printf "${CMD} : %d:%d %d:%d sum=%d mul=%d\n" \
				   $i $a $j $b \
				   $((a+b)) $((a*b))
			break 2
		fi
	done
done
exit 0
