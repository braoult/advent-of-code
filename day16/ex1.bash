#!/bin/bash
#
# ex1.bash: Advent2020 game, day 16/game 1.

CMD=${0##*/}
#shopt -s extglob

declare -A valid=()
declare -i state=0 res=0

while read -r line; do
	if [[ $line =~ ^([a-z :]+)([0-9]+)-([0-9]+)([a-z ]+)([0-9]+)-([0-9]+)$ ]]; then
		n1=$(seq "${BASH_REMATCH[2]}" "${BASH_REMATCH[3]}")
		n2=$(seq "${BASH_REMATCH[5]}" "${BASH_REMATCH[6]}")
		for i in $n1 $n2; do
			valid[$i]=1
		done
	elif [[ $line =~ (your ticket:|nearby tickets) ]]; then
		((state++))
	elif [[ $line != "" ]]; then
		if ((state == 2)); then
			for i in ${line//,/ }; do
				[[ ! -v valid[$i] ]] && res=res+i
			done
		fi
	fi
done

printf "%s : res=%d\n" "$CMD" "$res"

exit 0
