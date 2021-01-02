#!/bin/bash
#
# ex1.bash: Advent2020 game, day 15/games 1 and 2.

CMD=${0##*/}
#shopt -s extglob

declare -A prev
declare -i cur=1 previous


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

	printf "nums:\n"
	for i in "${!prev[@]}"; do
		printf " prev[%d] => %d\n" "$i" "${prev[$i]}" >&2
	done
}

TARGET=$1
IFS=","
read -r line
set -- $line
while (($# > 0)); do
	prev[$1]=$cur
	((cur++))
	shift
done

for ((previous=0; cur <= TARGET; ++cur)); do
	if ((previous)); then
		((diff=cur-previous-1))
	else
		diff=0
	fi
	((previous=prev[$diff]))
	prev[$diff]=$cur
done

printf "%s : res[%d]=%d\n" "$CMD" "$TARGET" "$diff"

exit 0
