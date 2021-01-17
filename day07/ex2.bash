#!/bin/bash
#
# ex2.bash: Advent2020 game, day 7/game 2.

CMD=${0##*/}
shopt -s extglob

declare -A CONT

function contains() {
	local -i i size count=0 count2
	local -i nbags
	local key="$1" color
	local -a contents

	read -ra contents <<< "${CONT[$key]}"
	((size = ${#contents[@]}/2))

	for ((i=0; i<size; ++i)); do
		nbags=${contents[i*2]}
		color=${contents[i*2+1]}
		((count+=nbags))
		count2=$(contains "$color")
		((count+=count2*nbags))
	done
	echo "$count"
	return
}

function parse() {
	local -a val=("$@")
	local container="" in=""
	local -i i=0 num=0
	while [[ ! "${val[$i]}" =~ bag[s]* ]]; do
		container+="${val[$i]}"
		((i++))
	done
	CONT[$container]=""
	((i++))
	while [[ -n "${val[$i]}" ]]; do
		case "${val[$i]}" in
			contain)
				;;
			[[:digit:]]*)
				num=${val[$i]}
				;;
			bag*)
				CONT[$container]+="$num "
				CONT[$container]+="$in "
				in=""
				num=0
				;;
			*)
				in+="${val[$i]}"
				;;
		esac
		((i++))
	done
}

declare -a array
while read -r -a array; do
	parse "${array[@]}"
done
target="shinygold"
res=$(contains $target)
printf "%s : target=%s res=%d\n" "$CMD" "$target" "$res"

exit 0
