#!/bin/bash
#
# ex1.bash: Advent2020 game, day 14/game 1.

CMD=${0##*/}
shopt -s extglob
declare -A mem
declare -i pos exp lenmask
declare ORMASK ANDMASK
declare -a xlist

function applymask() {
	local -i curmem val mask cur
	if (($# < 3)); then
		mem[$1]=$2
		return
	fi

	curmem="$1"
	val="$2"
	mask="$3"
	shift 3
	applymask "$curmem" "$val" $*
	applymask $((curmem+mask)) "$val" $*
}

while read -r line; do
	line=${line// /}

	if [[ $line =~ ^mask ]]; then
		eval "$line"
		ORMASK=2\#${mask//X/0}
		ANDMASK=2\#${mask//0/1}
		ANDMASK=${ANDMASK//X/0}
		lenmask=${#mask}
		xlist=()
		exp=1
		for ((i=1; i<=lenmask; ++i)); do
			c=${mask: -i:1}
			if [[ "$c" == "X" ]]; then
				xlist+=("$exp")
			fi
			((exp<<=1))
		done
	else
		[[ $line =~ ^.*\[([0-9]+)\][^0-9]*([0-9]+)$ ]]
		pos=${BASH_REMATCH[1]}
		val=${BASH_REMATCH[2]}
		((pos |= ORMASK))
		((pos &= ANDMASK))
		applymask "$pos" "$val" ${xlist[*]}

	fi

done
SUM=0
for i in ${!mem[@]}; do
	((SUM+=${mem[$i]}))
done
printf "%s : res=%d\n" "$CMD" "$SUM"
exit 0
