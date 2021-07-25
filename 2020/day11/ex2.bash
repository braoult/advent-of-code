#!/bin/bash
#
# ex2.bash: Advent2020 game, day 11/game 2.

CMD=${0##*/}
shopt -s extglob

declare -a rowsstr rows seats control
declare -i NCOLS NROWS SIZE
declare zeroed

function print_control() {
	local -i r c
	for ((r=0; r<NROWS; ++r)); do
		for ((c=0; c<NCOLS; ++c)); do
			printf "%2d " "${control[r*NCOLS+c]}" >&2
		done
		echo >&2
	done
	echo >&2
}
function print_seats() {
	local -i r c
	for ((r=0; r<NROWS; ++r)); do
		for ((c=0; c<NCOLS; ++c)); do
			printf "%d " "${seats[r*NCOLS+c]}" >&2
		done
		echo >&2
	done
	echo >&2
}
function run() {
	local -i c r i cell changed=1
	#local -a control
	local -i loop=0 seated=0

	read -ra seats <<< "${rows[*]}"
	#print_seats
	while ((changed > 0)); do
		changed=0
		seated=0
		read -ra control <<< "$zeroed"
		for ((r=0; r<NROWS; ++r)); do
			for ((c=0; c<NCOLS; ++c)); do
				((cell=r*NCOLS+c))
				if (( seats[cell] == 2 )); then
					# diagonal up left
					r1=$((r-1))
					c1=$((c-1))
					while ((r1>=0 && c1>=0)); do
						((control[r1*NCOLS+c1]++));
						#echo up >&2
						((seats[r1*NCOLS+c1] != 0)) && break
						((r1--))
						((c1--))
					done
					# diagonal up right
					r1=$((r-1))
					c1=$((c+1))
					while ((r1>=0 && c1<NCOLS)); do
						((control[r1*NCOLS+c1]++));
						((seats[r1*NCOLS+c1] != 0)) && break
						((r1--))
						((c1++))
					done
					# diagonal down left
					r1=$((r+1))
					c1=$((c-1))
					while ((r1<NROWS && c1>=0)); do
						((control[r1*NCOLS+c1]++));
						((seats[r1*NCOLS+c1] != 0)) && break
						((r1++))
						((c1--))
					done
					# diagonal down right
					r1=$((r+1))
					c1=$((c+1))
					while ((r1<NROWS && c1<NCOLS)); do
						((control[r1*NCOLS+c1]++));
						((seats[r1*NCOLS+c1] != 0)) && break
						((r1++))
						((c1++))
					done
					# line left
					c1=$((c-1))
					while ((c1>=0)); do
						((control[r*NCOLS+c1]++));
						((seats[r*NCOLS+c1] != 0)) && break
						((c1--))
					done
					# line right
					c1=$((c+1))
					while ((c1<NCOLS)); do
						((control[r*NCOLS+c1]++));
						((seats[r*NCOLS+c1] != 0)) && break
						((c1++))
					done
					# column up
					r1=$((r-1))
					while ((r1>=0)); do
						((control[r1*NCOLS+c]++));
						((seats[r1*NCOLS+c] != 0)) && break
						((r1--))
					done
					# column down
					r1=$((r+1))
					while ((r1<NROWS)); do
						((control[r1*NCOLS+c]++));
						((seats[r1*NCOLS+c] != 0)) && break
						((r1++))
					done
				fi
			done
		done
		for ((r=0; r<NROWS; ++r)); do
			for ((c=0; c<NCOLS; ++c)); do
				((cell=r*NCOLS+c))
				#((ccell=(r+1)*(NCOLS+2) + c+1))
				case ${seats[cell]} in
					0) continue
					   ;;
					1) if (( control[cell] == 0 )); then
						   ((++changed))
						   seats[cell]=2
					   fi
					   ;;
					2) if (( control[cell] >= 5 )); then
						   ((++changed))
						   seats[cell]=1
					   fi
				esac
				#printf "r=%d c=%d cell=%d val=%s\n" "$r" "$c" "$cell" "${seats[cell]}" >&2
			done
			#echo >&2
		done

		for ((r=0; r<NROWS; ++r)); do
			for ((c=0; c<NCOLS; ++c)); do
				((cell=(r*(NCOLS))+c))
				((seats[cell] == 2)) && ((seated++))
			done
		done
		#print_control
		#print_seats

		((loop++))
	done
	echo "$seated"
}



readarray -t rowsstr

for ((i=0; i<${#rowsstr[@]}; ++i)); do
	rowsstr[i]=${rowsstr[i]//./0}
	rowsstr[i]=${rowsstr[i]//L/1}
	rowsstr[i]=${rowsstr[i]//#/2}
done

NCOLS=${#rowsstr[0]}
NROWS=${#rowsstr[@]}
SIZE=$((NCOLS * NROWS))

printf -v zeroed '%0.s0 ' $(seq 1 $SIZE)

# split arrays in integers
for ((r=0; r<NROWS; ++r)); do
	srow="${rowsstr[r]}"
	nrow=()
	for ((c=0 ; c<NCOLS ; c++ )); do
		nrow+=("${srow:c:1}")
	done
	rows[r]="${nrow[*]}"
done
read -ra seats <<< "${rows[*]}"

#echo "S=${#seats[@]} C=${#control[@]}"
#print_seats

res=$(run)
printf "%s : res=%d\n" "$CMD" "$res"

exit 0
