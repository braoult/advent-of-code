#!/bin/bash
#
# ex1-2-slow.bash: Advent2020 game, day 15/games 1 and 2.
# ===> Too slow for exercise 2, needed to rewrite the algorithm.

CMD=${0##*/}
#shopt -s extglob

declare -A prev nums
declare -i cur=0 last


function print_binary() {
	local -i num=$1 exp
	local str="" format="%0${2}d"

	for ((exp=1; num>exp; exp<<=1)); do
		if ((num & exp)); then
			str=1"$str"
		else
			str=0"$str"
		fi
		 echo " str=$str"
	done
	printf "$format" $str
}

function print_array() {
	local -i i

	printf "last: %d\n" "$last"
	printf "nums:\n"
	for i in "${!nums[@]}"; do
		printf "  %6d => %d\n" "$i" "${nums[$i]}" >&2
		if [[ -v prev[$i] ]]; then
			printf "        prev => %d\n" "${prev[$i]}" >&2
		fi
	done
}

TARGET=$1
IFS=","
read -r line
set -- $line
while (($# > 0)); do
	nums[$1]=$cur
	last=$1
	((cur++))
	shift
done

for ((; cur != TARGET; ++cur)); do
	diff=0
	if [[ -v prev[$last] ]]; then
		((diff=nums[$last]-prev[$last]))
		prev[$last]=${nums[$last]}
	fi
	[[ -v nums[$diff] ]] && prev[$diff]=${nums[$diff]}
	nums[$diff]=$cur
	last=$diff
done

printf "%s : res=%d\n" "$CMD" "$last"

exit 0
