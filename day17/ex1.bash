#!/bin/bash
#
# ex1.bash: Advent2020 game, day 17/game 1.

CMD=${0##*/}
#shopt -s extglob

declare -A life=()
declare -i x=-1 y=-1 z=-1 res=0
declare -i max
declare -i loops=6

function print_life() {
	local -i x=0 y=0 z=0 foundx foundy
	for ((z=0; z<max; ++z)); do
		foundy=1
		for ((y=0; y<max; ++y)); do
			foundx=1
			for ((x=0; x<max; ++x)); do
				if [[ -v life["$x-$y-$z"] ]]; then
					#printf "%d-%d-%d:" $x $y $z
					if [[ ${life[$x-$y-$z]} != "#" ]]; then
					   printf "error"
					fi
					printf "%c" "${life[$x-$y-$z]}"
					foundx=1
				else
					printf "%c" "."
				fi
			done
			((foundx==1)) && foundy=1 && printf "\n"
		done
		((foundy==1)) && printf "z=%d\n\n" "$z"
	done

}

function run_cycle () {
	local -i x y z count=0 x1 y1 z1 v
	local -A lifetmp=()
	local -a values


	for k in "${!life[@]}"; do
		values=(${k//-/ })
		x=${values[0]}
		y=${values[1]}
		z=${values[2]}

		for ((x1=x-1; x1<x+2; ++x1)); do
			for ((y1=y-1; y1<y+2; ++y1)); do
				for ((z1=z-1; z1<z+2; ++z1)); do
					if ((x1!=x || y1!=y || z1!=z)); then
						((++lifetmp[$x1-$y1-$z1]))
					fi
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

function count_active () {
	local k
	local -i count=0

	for k in "${!life[@]}"; do
		if [[ ${life[$k]} == "#" ]]; then
			((count++))
		fi
	done
	echo "$count"
}

x=$loops; y=$loops; z=$loops

while read -r line; do
	if ((y==loops)); then						  # 1st line
		((max=loops+${#line}+loops+1))
	fi
	for ((j=0; j<${#line}; ++j)); do
		((curx=x+j))
		c=${line:j:1}
		if [[ $c == "#" ]]; then
			life[$curx-$y-$z]=$c
		fi
	done
	((y++))
done
for ((i=0; i<6; ++i)); do
	run_cycle
done
#echo "================================="
res=${#life[@]}

printf "%s : res=%d\n" "$CMD" "$res"

exit 0
