#!/bin/bash
#
# ex1.bash: Advent2020 game, day 14/game 1.

CMD=${0##*/}
shopt -s extglob
declare -A mem
declare -i pos
declare ORMASK ANDMASK
declare mask=""

while read -r line; do
	line=${line// /}
	eval "$line"

	if [[ $line =~ ^mask ]]; then
		ORMASK=2\#${mask//X/0}
		ANDMASK=2\#${mask//X/1}
	else
		[[ $line =~ ^.*\[([0-9]+)\].* ]]
		pos=${BASH_REMATCH[1]}
		((mem[$pos] &= ANDMASK))
		((mem[$pos] |= ORMASK))
	fi
done
SUM=0
for i in "${!mem[@]}"; do
	((SUM+=${mem[$i]}))
done
printf "%s : res=%d\n" "$CMD" "$SUM"
exit 0
