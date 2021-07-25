#!/bin/bash
#
# ex2-sort.bash: Advent2020 game, day 1/game 2, with sort.

CMD=$(basename "$0")

readarray -t numbers <<< "$(sort -n)"

declare -i i j k a b c m1 m2
for ((i=0; i<${#numbers[@]}; ++i)); do
	a=$((numbers[i]))
	for ((j=i+1; j<${#numbers[@]}; ++j)); do
		b=$((numbers[j]))
		((a+b > 2020)) && break
		for ((k=j+1; k<${#numbers[@]}; ++k)); do
			c=$((numbers[k]))
			((a+b+c > 2020)) && break
			if ((a+b+c == 2020)); then
				printf "${CMD} : %d:%d %d:%d %d:%d sum=%d mul=%d\n" \
				   $i $a $j $b $k $c \
				   $((a+b+c)) \
				   $((a*b*c))
				break 3
			fi
		done
	done
done
exit 0
