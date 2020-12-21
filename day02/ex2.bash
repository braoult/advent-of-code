#!/bin/bash
#
# ex2.bash: Advent2020 game, day 2/game 2.

CMD=${0##*/}

declare -i beg end num found=0 nlines=0

IFS=$':- \t'
while read -r beg end char str; do
	num=0
	[[ ${str:beg-1:1} == "$char" ]] && ((num++))
	[[ ${str:end-1:1} == "$char" ]] && ((num++))
	#((num == 1 && found++))
	((found += num == 1))
	((nlines++))
done
printf "${CMD} : lines: %d matched:%d\n" "$nlines" "$found"

exit 0
