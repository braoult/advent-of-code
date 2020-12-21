#!/bin/bash
#
# ex1.bash: Advent2020 game, day 10/game 1.

CMD=${0##*/}
shopt -s extglob

declare -a numbers
readarray -t numbers <<< "$(sort -n)"
numbers=(0 "${numbers[@]}")
size=${#numbers[@]}
((last=${numbers[size-1]}+3))
numbers+=("$last")
((size++))
#echo S="$size" $last "[${numbers[@]}]"

# last
declare -a res=(0 0 0 0)

for ((i=1; i<size; ++i)); do
	prev=${numbers[$i-1]}
	cur=${numbers[$i]}
	diff=$((cur-prev))
	((res[diff]++))
done
res1="${res[1]}"
res3="${res[3]}"

printf "%s : diff1=%d diff2=%d res=%d\n" "$CMD" "$res1" "$res3" $((res1*res3))

exit 0
