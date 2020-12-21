#!/bin/bash
#
# ex1.bash: Advent2020 game, day 4/game 1.

CMD=${0##*/}
shopt -s extglob

declare -A KEYS
KEYS=( ["byr"]="" ["iyr"]="" ["eyr"]="" ["hgt"]="" ["hcl"]="" ["ecl"]=""
	   ["pid"]="" ["cid"]="" )

function reset_array() {						  # reset array values
	local -n array=${1}
	local key

	#echo before reset: ${KEYS[@]}
	for key in "${!array[@]}"; do
		array[$key]=""
	done
	#echo after reset: ${KEYS[@]}
}

function passport_valid() {
	local -i ret=0

	for key in "${!KEYS[@]}"; do
		[[ $key != "cid" ]] && [[ -z ${KEYS[$key]} ]] && ret=1 && break
	done
	return $ret
}

declare -i current=0 nvalids=0

while read -r line; do
	if [[ -z "$line" ]]; then
		((current++))
		passport_valid && ((nvalids ++))
		reset_array KEYS
		continue
	fi
	for pair in $line; do
		key=${pair%%:*}
		val=${pair##*:}
		[[ -v KEYS[$key] ]] && KEYS[$key]="$val"  # valid key
	done
done
((current++))
passport_valid && ((nvalids ++))
printf "%s : valid=%d/%d\n" "$CMD" "$nvalids" "$current"

exit 0
