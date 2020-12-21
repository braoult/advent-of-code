#!/bin/bash
#
# ex1.bash: Advent2020 game, day 8/game 1.

CMD=${0##*/}
shopt -s extglob

#declare -i ACCUMULATOR=0

declare -a PRG=()
declare -i ARG=()
declare -i PRGSIZE=0

function run() {
	local -i arg cur=0 res=0
	local -a steps=()
	local instr

	while [[ ${steps[$cur]} != 1 ]]; do
		instr=${PRG[$cur]}
		arg=${ARG[$cur]}
		steps[$cur]=1
		case "$instr" in
			acc)
				((res += arg))
				((cur++))
				;;
			nop)
				((cur++))
				;;
			jmp)
				((cur += arg))
				;;
		esac
	done
	echo "$res"
}

function print_prog()
{
	local -i i
	for ((i=0; i<PRGSIZE; ++i)); do
		printf "%03d : [%s][%3d]\n" "$i" "${PRG[$i]}" "${ARG[$i]}"
	done
}

while read -r instr arg; do
	PRG[$PRGSIZE]=$instr
	ARG[$PRGSIZE]=$arg
	((PRGSIZE++))
done

#print_prog

res=$(run)
printf "%s : res=%d\n" "${CMD}" "$res"

exit 0
