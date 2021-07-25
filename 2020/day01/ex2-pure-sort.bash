#!/bin/bash
#
# ex2-pure-sort.bash: Advent2020 game, day 1/game 2, bash bubble sort.

CMD=${0##*/}

readarray -t numbers

# simple bubble sort for numeric array (ascending)
# args: 1: the array
# todo: add a parameter for asc/desc order.
sort_n() {
    local -a array=( "$@" )
    local -i max=$(( ${#array[@]} - 1 ))

    for (( max= $(( ${#array[@]} - 1 )); max > 0; max-- )); do
        local -i i
        for (( i=0; i<max; i++ )); do
            local -i val1=${array[$i]}
            local -i val2=${array[$((i + 1))]}

            # switch if necessary
            if (( val1 > val2 )); then
                local tmp=$val1
                array[$i]=$val2
                array[$((i + 1))]=$tmp
            fi
        done
    done
    echo "${array[@]}"
}

sorted=( $(sort_n "${numbers[@]}") )

declare -i i j k a b c

for ((i=0; i<${#sorted[@]}; ++i)); do
	a=$((sorted[i]))
	for ((j=i+1; j<${#sorted[@]}; ++j)); do
		b=$((sorted[j]))
		((a+b > 2020)) && break
		for ((k=j+1; k<${#sorted[@]}; ++k)); do
			c=$((sorted[k]))
			((a+b+c > 2020)) && break
			if ((a+b+c == 2020)); then
				printf "${CMD} : %d:%d %d:%d %d:%d sum=%d mul=%d\n" \
				   $i $a $j $b $k $c \
				   $((a+b+c)) \
				   $((a*b*c))
				break 3
			fi
		done
	done
done
exit 0
