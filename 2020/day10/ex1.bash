#!/bin/bash
#
# ex1.bash: Advent2020 game, day 10/game 1.

CMD=${0##*/}
shopt -s extglob

# quicksort implementation (ascending)
qsort() {
   local pivot i smaller=() larger=()
   qsort_ret=()
   (($#==0)) && return 0
   pivot=$1
   shift
   for i; do
      if (( i < pivot )); then
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
numbers=(0 ${qsort_ret[@]})
size=${#numbers[@]}
((last=${numbers[size-1]}+3))
numbers+=("$last")
((size++))
#echo S="$size" $last "[${numbers[@]}]"

# last
declare -a res=(0 0 0 0)

for ((i=1; i<size; ++i)); do
	prev=${numbers[$i-1]}
	cur=${numbers[$i]}
	diff=$((cur-prev))
	((res[diff]++))
done
res1="${res[1]}"
res3="${res[3]}"

printf "%s : diff1=%d diff2=%d res=%d\n" "$CMD" "$res1" "$res3" $((res1*res3))

exit 0
