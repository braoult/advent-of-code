#!/bin/bash
#
# ex1.bash: Advent2020 game, day 10/game 1.

CMD=${0##*/}
shopt -s extglob

declare -a rowsstr rows
declare -i RLENGTH
declare -i NROWS
declare floor

function adj() {
	local -i r="$1" n="$2" c="$3" count=0
	local -a prow=(${rows[r-1]})
	local -a row=(${rows[r]})
	local -a nrow=(${rows[r+1]})
	#echo
	#echo p="${prow[*]}"
	#echo r="${row[*]}"
	#echo p1=${prow[1]}
	#echo r1=${row[1]}
	#return
	#echo r="$row"
	#echo n="$nrow"
	if (( row[n] == 0 )); then
		echo "-1"
		return
	fi
	for ((i=n-1; i<n+2; ++i)); do
		#echo p: $i ${prow:i:1}
		(( prow[i] == c )) && ((count++))
		#echo p: $i ${nrow:i:1}
		(( nrow[i] == c )) && ((count++))
	done
	(( row[n-1] == c )) && ((count++))
	(( row[n+1] == c )) && ((count++))
	echo "$count"
}

function run() {
	local -i c r
	local -a row
	local -a new
	local -i cur
	local -i changed=1
	local -i loop=0 seated=0

	#echo "$floor;"
	while ((changed > 0)); do
		changed=0
		seated=0
		for ((r=1; r<=NROWS; ++r)); do
			row=(${rows[r]})
			newrow=(0)
			for ((c=1; c<=RLENGTH; ++c)); do
				newrow+=("${row[c]}")
				#if ((r == 1 && c ==1 )); then
				#   printf "r1c1: %s %d\n" "${row[c]}" $(adj $r $c 2)
				#fi
				case ${row[c]} in
					0) continue
					   ;;
					1) if (( $(adj $r $c 2) == 0 )); then
						   ((++changed))
						   newrow[c]=2
					   fi
					   #printf "[%d][%d]: %s %s %d\n" $r $c "${row[c]}" "${newrow[c]}" $(adj $r $c 2)
					   ;;
					2) if (( $(adj $r $c 2) >= 4 )); then
						   ((++changed))
						   newrow[c]=1
					   fi
					   ;;
				esac
				((newrow[c] == 2)) && ((seated++))
			done
			newrow+=(0)
			new[$r]="${newrow[*]}"
			#echo "${newrow[*]} ;"
		done
		#echo "$floor"
		#echo "${new[*]}"
		#echo changed=$changed
		((loop++))
		#echo loop="$loop"
		for ((r=1; r<=NROWS; ++r)); do
			rows[r]="${new[$r]}"
		done
		#printf "loop %d seated %d\n" "$loop" "$seated" >&2
	done
	echo "$seated"
}



readarray -t rowsstr

for ((i=0; i<${#rowsstr[@]}; ++i)); do
	rowsstr[i]=${rowsstr[i]//./0}
	rowsstr[i]=${rowsstr[i]//L/1}
	#echo "${rowsstr[i]}"
done

RLENGTH=${#rowsstr[0]}
NROWS=${#rowsstr[@]}
#echo "L=$RLENGTH N=$NROWS"

# add floor rows at beginning and end
printf -v floor '%0.s0 ' $(seq 1 $((RLENGTH+2)))
#echo floor="$floor"
#echo floor="$floor"
#echo
#for ((i=0; i<${#rowsstr[@]}; ++i)); do
#	rowsstr[i]=0${rowsstr[i]}0
#	echo "${rowsstr[i]}"
#done

# split arrays in integers
for ((r=0; r<NROWS; ++r)); do
	srow="${rowsstr[r]}"
	nrow=(0)
	for ((c=0 ; c<=RLENGTH+1 ; c++ )); do
		nrow+=("${srow:c:1}")
	done
	nrow+=(0)
	rows[r]="${nrow[*]}"
done
rows=("$floor" "${rows[@]}" "$floor")
#for ((r=0; r<=NROWS+1; ++r)); do
#	printf "row %d: %s\n" $r "${rows[r]}"
#done

#for ((r=0; r<=NROWS+1; ++r)); do
#	echo "row $r=${rows[r]}"
#done
#echo -n "adj 1 1 2: "
#adj 1 1 2
res=$(run)
printf "%s: res=%d\n" "$CMD" "$res"
exit 0

echo -n "adj 1 1 2: "
adj 1 1 2
echo -n "adj 1 2 1: "
adj 1 2 1
echo -n "adj 1 3 2: "
adj 1 3 2
echo -n "adj 1 4 1: "
adj 1 4 1
exit 0


numbers=(0 "${numbers[@]}")
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
