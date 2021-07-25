#!/bin/bash
#
# ex1.bash: Advent2020 game, day 11/game 1.

CMD=${0##*/}
shopt -s extglob

declare -a rowsstr rows
declare -i NCOLS NROWS
declare zeroed

function run() {
	local -i c r cell
	local -a seats control
	local -i changed=1
	local -i loop=0 seated=0

	read -ra seats <<< "${rows[*]}"
	while ((changed > 0)); do
		changed=0
		seated=0
		read -ra control <<< "$zeroed"
		for ((r=0; r<NROWS; ++r)); do
			for ((c=0; c<NCOLS; ++c)); do
				((cell=r*NCOLS+c))
				if (( seats[cell] == 2 )); then
					((control[(r)*(NCOLS+2) + c  ]++))
					((control[(r)*(NCOLS+2) + c+1]++))
					((control[(r)*(NCOLS+2) + c+2]++))

					((control[(r+1)*(NCOLS+2) + c  ]++))
					# this cell: control[(r+1)*(NCOLS+2) + c+1]
					((control[(r+1)*(NCOLS+2) + c+2]++))

					((control[(r+2)*(NCOLS+2) + c  ]++))
					((control[(r+2)*(NCOLS+2) + c+1]++))
					((control[(r+2)*(NCOLS+2) + c+2]++))
				fi
			done
		done
		for ((r=0; r<NROWS; ++r)); do
			for ((c=0; c<NCOLS; ++c)); do
				((cell=r*NCOLS+c))
				((ccell=(r+1)*(NCOLS+2) + c+1))
				case ${seats[cell]} in
					0) continue
					   ;;
					1) if (( control[ccell] == 0 )); then
						   ((++changed))
						   seats[cell]=2
					   fi
					   ;;
					2) if (( control[ccell] >= 4 )); then
						   ((++changed))
						   seats[cell]=1
					   fi
				esac
				#printf "r=%d c=%d cell=%d val=%s\n" "$r" "$c" "$cell" "${seats[cell]}"
			done
			#echo
		done

		for ((r=0; r<NROWS; ++r)); do
			for ((c=0; c<NCOLS; ++c)); do
				((cell=(r*(NCOLS))+c))
				((seats[cell] == 2)) && ((seated++))
			done
		done

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

printf -v zeroed '%0.s0 ' $(seq 1 $(((NCOLS+2)*(NROWS+2))))

# split arrays in integers
for ((r=0; r<NROWS; ++r)); do
	srow="${rowsstr[r]}"
	nrow=()
	for ((c=0 ; c<NCOLS ; c++ )); do
		nrow+=("${srow:c:1}")
	done
	rows[r]="${nrow[*]}"
done

res=$(run)
printf "%s : res=%d\n" "$CMD" "$res"

exit 0
