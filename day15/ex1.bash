#!/bin/bash
#
# ex1.bash: Advent2020 game, day 15/games 1 and 2.

CMD=${0##*/}
#shopt -s extglob

declare -A prev
declare -i cur=1 previous


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
