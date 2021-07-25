#!/bin/bash
#
# ex1.bash: Advent2020 game, day 8/game 2.

CMD=${0##*/}
shopt -s extglob

declare -a PRG=()
declare -i ARG=()

function run() {
	local -i arg cur=0 res=0 size=$1
	local instr
	local -a steps=()

	while ((cur < size)) && [[ ${steps[$cur]} != 1 ]]; do
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
	echo $res
	return $((cur < size))
}

function print_prog()
{
	local -i i
	for ((i=0; i<${#PRG[@]}; ++i)); do
		printf "%03d : [%s][%3d]\n" "$i" "${PRG[$i]}" "${ARG[$i]}"
	done
}

declare -i i=0
while read -r instr arg; do
	PRG[$i]=$instr
	ARG[$i]=$arg
	((i++))
done

lastchanged=-1
savstep=""
while true; do
	res=$(run $i) && break

	if ((lastchanged >= 0)); then
		PRG[$lastchanged]="$savstep"
	fi
	((lastchanged++))
	while ((lastchanged <= PRGSIZE)) && [[ ${PRG[$lastchanged]} == acc ]]; do
		((lastchanged++))
	done

	savstep=${PRG[$lastchanged]}
	case $savstep in
		jmp)
			PRG[$lastchanged]="nop"
			;;
		nop)
			PRG[$lastchanged]="jmp"
			;;
	esac
done

printf "%s : res:%d\n" "${CMD}" "$res"

exit 0
