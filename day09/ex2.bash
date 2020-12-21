#!/bin/bash
#
# ex1.bash: Advent2020 game, day 8/game 1.

CMD=${0##*/}
shopt -s extglob

declare -i N=5 SIZE
declare -a LIST=()

function push25 () {
	local i
	for ((i=0; i<25; ++i)); do
		read -r i
		LIST+=("$i")
	done
}

function push() {
	local i
	while read -r i; do
		LIST+=("$i")
	done
	SIZE=${#LIST[@]}
}

function badnum() {
	local -i cur="$1" i j
	local -i res=${LIST[$cur]}
	local -i start=$((cur-N-1))
	for ((i=start; i<cur; ++i)); do
		for ((j=i+1; j<cur; ++j)); do
			if (((LIST[i]+LIST[j]) == res)); then
				return 0
			fi
		done
	done
	return 1
}

function findsum() {
	local -i target=$1 start=$2 i
	local -i sum=0
	local -i small=${LIST[$start]}
	local -i high=$small

	for ((i=start; i<SIZE && sum<target; ++i)); do
		((cur=${LIST[$i]}))
		((sum+=cur))
		((cur<small)) && small=$cur
		((cur>high)) && high=$cur

		if ((sum==target)); then
			echo $((small+high))
			return 0
		fi
	done
	return 1
}


[[ $# = 1 ]] && N=$1

push
declare -i i target num
for ((i=N; i<SIZE; ++i)); do
	badnum $i || break
done
target="${LIST[$i]}"
for ((i=0; i<SIZE; ++i)); do
	num=$(findsum "$target" $i) && break
done
printf "%s : res=%d sum=%d\n" "$CMD" "$target" "$num"

exit 0
