#!/bin/bash
#
# ex1.bash: Advent2020 game, day 17/game 1.

CMD=${0##*/}
#shopt -s extglob

declare -A life=()
declare -i x=-1 y=-1 z=-1 res=0
declare -i maxx maxy maxz

function print_life() {
	local -i x=0 y=0 z=0 foundx foundy
	for ((z=0; z<maxz; ++z)); do
		foundy=1
		for ((y=0; y<maxy; ++y)); do
			foundx=1
			for ((x=0; x<maxx; ++x)); do
				if [[ -v life[$x-$y-$z] ]]; then
					#printf "%d-%d-%d:" $x $y $z
					printf "%c" ${life["$x-$y-$z"]}
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
	#((count)) && printf "neighbours (%d, %d, %d)=%s\n" "$x" "$y" "$z" "$str" >&2
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
