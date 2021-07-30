#!/bin/bash
#
# ex2.bash: Advent2020 game, day 1/game 2

CMD=${0##*/}

readarray -t numbers

declare -i i j k

for ((i=0; i<${#numbers[@]}; ++i)); do
	for ((j=i+1; j<${#numbers[@]}; ++j)); do
		for ((k=j+1; k<${#numbers[@]}; ++k)); do
			if ((numbers[i] + numbers[j] + numbers[k] == 2020)); then
				printf "${CMD} : %d:%d %d:%d %d:%d sum=%d mul=%d\n" \
				   $i $((numbers[i])) $j $((numbers[j])) $k $((numbers[k])) \
				   $((numbers[i]+numbers[j]+numbers[k])) \
				   $((numbers[i]*numbers[j]*numbers[k]))
				break 3
			fi
		done
	done
done
exit 0
