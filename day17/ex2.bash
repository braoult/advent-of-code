#!/bin/bash
#
# ex1.bash: Advent2020 game, day 17/game 1.

CMD=${0##*/}
#shopt -s extglob

declare -A life=()
declare -i x=-1 y=-1 z=-1 w=-1 res=0
declare -i loops=6

function run_cycle () {
	local -i x y z w x1 y1 z1 w1 v
	local -A lifetmp=()
	local -a values


	for k in "${!life[@]}"; do
		# shellcheck disable=SC2206
		values=(${k//-/ })
		x=${values[0]}
		y=${values[1]}
		z=${values[2]}
		w=${values[3]}

		for ((x1=x-1; x1<x+2; ++x1)); do
			for ((y1=y-1; y1<y+2; ++y1)); do
				for ((z1=z-1; z1<z+2; ++z1)); do
					for ((w1=w-1; w1<w+2; ++w1)); do
						if ((x1!=x || y1!=y || z1!=z || w1!=w)); then
							((++lifetmp[$x1-$y1-$z1-$w1]))
						fi
					done
				done
			done
		done
	done

	# clean elements with no neighbors
	for k in "${!life[@]}"; do
		if [[ ! -v lifetmp[$k] ]]; then
			unset "life[$k]"
		fi
	done
	for k in "${!lifetmp[@]}"; do
		v=${lifetmp[$k]}

		if [[ -v life[$k] ]]; then
			(( v!=2 && v!=3 )) && unset "life[$k]"
		else
			(( v==3 )) && life[$k]="#"
		fi
	done
}

x=$loops; y=$loops; z=$loops; w=$loops
while read -r line; do
	for ((j=0; j<${#line}; ++j)); do
		((curx=x+j))
		c=${line:j:1}
		if [[ $c == "#" ]]; then
			life[$curx-$y-$z-$w]=$c
		fi
	done
	((y++))
done
for ((i=0; i<6; ++i)); do
	run_cycle
done
res=${#life[@]}

printf "%s : res=%d\n" "$CMD" "$res"

exit 0
