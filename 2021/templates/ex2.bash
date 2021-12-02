#!/bin/bash
#
# ex1.bash: Advent2020 game, day 3/game 2.

CMD=${0##*/}

XMOVES=(1 3 5 7 1)								  # moves right
YMOVES=(1 1 1 1 2)								  # moves down
declare -i linelen=0 modpos=0 res=1
declare -a xpos xcount

read -r line									  # ignore 1st line

while read -r line; do
	(( ypos++ ))
	(( linelen = ${#line} ))

	for ((i=0; i<${#XMOVES[@]}; ++i)); do
		if ((ypos % YMOVES[i] == 0)); then
			(( xpos[i] += XMOVES[i] ))			  # move right
			(( modpos = (xpos[i] % linelen) ))
			[[ ${line:modpos:1} = \# ]] && ((xcount[i]++))
		fi
	done
done
echo "xpos=${xpos[*]} xcount=${xcount[*]}"
printf "%s : " "$CMD"
for ((i=0; i<${#XMOVES[@]}; ++i)); do
	printf "[%d]=[%d, %d, %d, %d] " $i "${XMOVES[$i]}" "${YMOVES[$i]}" \
		   "${xpos[$i]}" "${xcount[$i]}"
	(( res *= xcount[i] ))
done
printf "res:%d\n " $res
exit 0
