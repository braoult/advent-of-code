#!/bin/bash
#
# ex1.bash: Advent2020 game, day 7/game 1.

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
	#printf "search val=%s key=%s cont=[%s] ret=%d\n"  "$val" "$key" "$cont"  "$ret"
	return $ret
}

function contains() {
	local -i
	local value="$1"

	for container in "${!CONT[@]}"; do
		if [[ ${match[$container]} = "" ]]; then
			if search "$value" "$container" ; then
				contains "$container"
			fi
		fi
	done
}

function parse() {
	local -a val=("$@")
	local container="" in=""
	local -i i=0 num=0
	# get container
	while [[ ! "${val[$i]}" =~ bag[s]* ]]; do
		container+="${val[$i]}"
		((i++))
	done
	CONT[$container]=""
	((i++))
	while [[ -n "${val[$i]}" ]]; do
		case "${val[$i]}" in
			contain)
				#echo contain
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
match=()
contains $target
printf "%s : target=%s nkeys=%d res=%d\n" "$CMD" "$target" "${#CONT[@]}" "${#match[@]}"

exit 0
