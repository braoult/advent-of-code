#!/bin/bash
#
# ex2.bash: Advent2020 game, day 4/game 2.

CMD=${0##*/}
shopt -s extglob

declare -A KEYS=( ["byr"]="" ["iyr"]="" ["eyr"]="" ["hgt"]="" ["hcl"]="" ["ecl"]=""
				  ["pid"]="" ["cid"]="" )
NKEYS=${#KEYS[@]}

function reset_array() {						  # reset array values
	local -n array=${1}
	local key

	#echo before reset: ${KEYS[@]}
	for key in "${!array[@]}"; do
		array[$key]=""
	done
	#echo after reset: ${KEYS[@]}
}

# returns 1 if $1 length is $2
function check_length() {
	return $((${#1} != $2))
}
function check_digits() {
	[[ "$1" =~ ^[[:digit:]]+$ ]] && return 0 || return 1
}
# returns 1 if $1 is between $2 and $3
function check_range() {
	(($1 >= $2 && $1 <= $3)) && return 0 || return 1
}
declare -A COLORS=( ["amb"]="" ["blu"]="" ["brn"]="" ["gry"]=""
					["grn"]="" ["hzl"]="" ["oth"]="" )
function check_eye() {
	[[ -v COLORS["$1"] ]] && return 0 || return 1
}
function check_height() {
	local -i ret=1
	local val="$1" unit=${val: -2}
	local num=${val%%$unit}

	#printf "unit=%s hgt=%s\n" "$unit" "$num"
	case "$unit" in
		"cm") check_digits "$num" && check_range "$num" 150 193 && ret=0
			;;
		"in") check_digits "$num" && check_range "$num" 59 76 && ret=0
			;;
	esac
	return $ret
}
function check_hair() {
	[[ "$1" =~ ^#[a-f0-9]{6}$ ]] && return 0 || return 1
}

function check_key() {
	local -i valid=0
	local key="$1" val="${KEYS[$key]}"

	#printf "KEY/VAL = %s=[%s]\n" "$key" "$val"
	case "$key" in
		cid) valid=1
			 ;;
		byr) check_digits "$val" &&
				 check_length "$val" 4 &&
				 check_range "$val" 1920 2002 && valid=1
			 ;;
		iyr) check_digits "$val" &&
				   check_length "$val" 4 &&
				   check_range "$val" 2010 2020 && valid=1
			 ;;
		eyr) check_digits "$val" &&
				   check_length "$val" 4 &&
				   check_range "$val" 2020 2030 && valid=1
			 ;;
		pid) check_digits "$val" &&
				   check_length "$val" 9 && valid=1
			 ;;
		ecl) check_eye "$val" && valid=1
			 ;;
		hcl) check_hair "$val" && valid=1
			 ;;
		hgt) check_height "$val" && valid=1
			 ;;
		# TODO
		*) valid=1
		   ;;
	esac
	#printf "%s=[%s] : valid=%d\n" "$key" "$val" $valid
	#[[ ! $key == "cid" ]] && [[ -z ${KEYS[$key]} ]] && valid=0 && break
	return $valid
}

function passport_valid() {
	local -i ret=0

	for key in "${!KEYS[@]}"; do
		#printf "current key: %s\n" "$key"
		[[ $key != "cid" ]] && #[[ -z ${KEYS[$key]} ]] &&
			check_key "$key" && ret=1 && break
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
