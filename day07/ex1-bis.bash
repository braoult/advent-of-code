#!/bin/bash
#
# ex1-bis.bash: Advent2020 game, day 7/game 1.

CMD=${0##*/}
shopt -s extglob

declare -A CONT
declare -A match

function search() {
	local -i ret=1
	local val="$1" key="$2"
	local cont="${CONT[$key]}"
	if [[ $cont =~ $val ]]; then
		match[$key]=1
		ret=0
	fi
	return $ret
}

function contains() {
	local -i
	local -a container
	local key="$1"

	set ${CONT[$key]}
	while (( $# > 0 )); do
		[[ -v CONT[$1] ]] && contains "$1"
		match[$1]=1
		shift
	done
}

function parse() {
	set $@
	local container="" in=""
	local -i i=0

	while [[ ! "$1" =~ bag[s]* ]]; do
		container+="$1"
		shift
	done
	shift
	shift
	while (( $# > 0 )); do
		case "$1" in
			contain)
				echo contain
				;;
			[[:digit:]]*)
				;;
			bag*)
				CONT[$in]+="$container "
				in=""
				;;
			*)
				in+="$1"
				;;
		esac
		shift
	done
}

declare -a array
while read -r line; do
	parse "$line"
done

target="shinygold"
match=()
contains $target
printf "%s : target=%s res=%d\n" "$CMD" "$target" "${#match[@]}"
#printf "%s : lines=%d max=%d\n" "$CMD" "$lines" "$max"

exit 0
