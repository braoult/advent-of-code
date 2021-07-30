#!/bin/bash
#
# ex2.bash: Advent2020 game, day 10/game 2.

CMD=${0##*/}
shopt -s extglob

# quicksort implementation (descending)
qsort() {
   local pivot i smaller=() larger=()
   qsort_ret=()
   (($#==0)) && return 0
   pivot=$1
   shift
   for i; do
      if (( i > pivot )); then
         smaller+=( "$i" )
      else
         larger+=( "$i" )
      fi
   done
   qsort "${smaller[@]}"
   smaller=( "${qsort_ret[@]}" )
   qsort "${larger[@]}"
   larger=( "${qsort_ret[@]}" )
   qsort_ret=( "${smaller[@]}" "$pivot" "${larger[@]}" )
}

declare -a numbers
readarray -t numbers
qsort "${numbers[@]}"
numbers=(${qsort_ret[@]} 0)
((last=numbers[0]+3))
numbers=("$last" "${numbers[@]}")
size=${#numbers[@]}

declare -a res
for ((i=1; i<size; ++i)); do
	res[$i]=0
done
res[0]=1

for ((i=0; i<size; ++i)); do
	cur=${numbers[i]}
	r=${res[i]}

	for j in 1 2 3; do
		((next=i+j))
		if ((next < size)); then
			nval=${numbers[next]}
			((diff=cur-nval))
			if ((diff > 0 && diff <= 3)); then
				((res[next]+=r))
			fi
		fi
	done
done

printf "%s : size=%d res=%d\n" "$CMD" "$size" "${res[size-1]}"

exit 0
