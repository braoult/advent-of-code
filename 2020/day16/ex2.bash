#!/bin/bash
#
# ex2.bash: Advent2020 game, day 16/game 2.

CMD=${0##*/}
shopt -s extglob

declare -a valid=() myticket=() keys=() values=() match=()
declare -A position=()
declare -i state=0 res=1 curkey=0 curticket=0

while read -r line; do
	if [[ $line =~ ^([a-z ]+)\:\ ([0-9]+)-([0-9]+)([a-z ]+)([0-9]+)-([0-9]+)$ ]]; then
		# valid ranges
		keys[$curkey]="${BASH_REMATCH[1]}"
		n1=$(seq -s" " "${BASH_REMATCH[2]}" "${BASH_REMATCH[3]}")
		n2=$(seq -s" " "${BASH_REMATCH[5]}" "${BASH_REMATCH[6]}")

		for i in $n1 $n2; do
			valid[$i]=1
		done
		values[$curkey]="${n1[*]} ${n2[*]}"
		((curkey++))

	elif [[ $line =~ (your ticket:|nearby tickets) ]]; then
		# next section
		((state++))
	elif [[ $line =~ [0-9,]+ ]]; then
		# shellcheck disable=SC2206
		numbers=( ${line//,/ } )
		case $state in
			1)									  # my ticket
				# shellcheck disable=SC2206
				myticket=( ${numbers[@]} )
				# initialize possible matches array: all
				for ((i=0; i<curkey; ++i)); do
					for ((j=0; j<${#numbers[@]}; ++j)); do
						match[$j]+=" $i "
					done
				done
				;;
			2)									  # other tickets
				#printf "ticket %d\n" "$curticket"
				for i in ${numbers[*]}; do
					[[ ! -v valid[$i] ]] && continue 2
				done
				for ((j=0; j<curkey; ++j)); do
					for ((i=0; i<${#numbers[@]}; ++i)); do
						num=" ${numbers[$i]} "
						[[ " ${values[$j]} " =~ $num ]] && continue
						match[$j]=${match[$j]// $i /}

					done
				done
				((curticket++))
				;;
		esac
	fi
done

end=0
while ((end==0)); do
	end=1
	for ((i=0; i<${#match[@]}; ++i)); do
		[[ -n ${position[$i]} ]] && continue
		# shellcheck disable=SC2206
		array=( ${match[$i]} )
		if (( ${#array[@]} == 1 )); then
			cur=${array[0]}
			position[$i]=$cur
			for ((j=0; j<${#match[@]}; ++j)); do
				((j != i)) && match[$j]=${match[$j]// $cur /}
			done
			end=0
			break
		fi
	done
done

for ((i=0; i<${#keys[@]}; ++i )); do
	if [[ ${keys[$i]} =~ "departure" ]]; then
		pos=${match[$i]}
		(( res *= ${myticket[$pos]} ))
	fi
done


printf "%s : res=%d\n" "$CMD" "$res"

exit 0
