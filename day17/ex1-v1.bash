#!/bin/bash
#
# ex1.bash: Advent2020 game, day 17/game 1.

CMD=${0##*/}
#shopt -s extglob

declare -A life=()
declare -i x=-1 y=-1 z=-1 res=0
declare -i maxx maxy maxz

function count_neighbors () {
	local -i x=$1 y=$2 z=$3 x1 y1 z1 count=0
	local str=""


	for ((x1=x-1; x1<x+2; ++x1)); do
		for ((y1=y-1; y1<y+2; ++y1)); do
			for ((z1=z-1; z1<z+2; ++z1)); do
				if ((x1!=x || y1!=y || z1!=z)); then
					if [[ ${life[$x1-$y1-$z1]} == "#" ]]; then
						((count++))
						str="$str $x1-$y1-$z1"
					fi

				fi
			done
		done
	done
	echo "$count"
}

function run_cycle () {
	local -i x y z count=0
	local -A lifetmp=()

	for ((x=0; x<maxx; ++x)); do
		for ((y=0; y<maxy; ++y)); do
			for ((z=0; z<maxz; ++z)); do
				count=$(count_neighbors "$x" "$y" "$z")
				if [[ ${life[$x-$y-$z]} == "#" ]]; then
					((count!=2 && count!=3)) && lifetmp[$x-$y-$z]="."
				else
					((count==3)) && lifetmp[$x-$y-$z]="#"
				fi
			done
		done
	done
	for k in "${!lifetmp[@]}"; do
		life[$k]=${lifetmp[$k]}
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

x=6; y=6; z=6
while read -r line; do
	if ((y==6)); then							  # 1st line
		((maxx=6+${#line}+6+1))
		((maxy=maxx))
		((maxz=maxx))
	fi
	for ((j=0; j<${#line}; ++j)); do
		((curx=x+j))
		life[$curx-$y-$z]=${line:j:1}
	done
	((y++))
done
for ((i=0; i<6; ++i)); do
	run_cycle
done
res=$(count_active)

printf "%s : res=%d\n" "$CMD" "$res"

exit 0
