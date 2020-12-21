#!/bin/bash
#
# ex1.bash: Advent2020 game, day 12/game 1.

CMD=${0##*/}
shopt -s extglob

declare -i X=0 Y=0
declare -i DIRECTION=90 REAL

while read -r line; do
	C=${line:0:1}
	A=${line:1}
	#printf "CMD=%s ARG=%d\n" "$C" "$A"

	case "$C" in
		R) ((DIRECTION=(DIRECTION+A)%360))
		   #printf "D=%d\n" "$DIRECTION"
		   ;;
		L) ((DIRECTION=(DIRECTION+360-A)%360))
		   #printf "D=%d\n" "$DIRECTION"
		   ;;
		F) REAL=$DIRECTION
		   ;;
		N) REAL=0
		   ;;
		S) REAL=180
		   ;;
		E) REAL=90
		   ;;
		W) REAL=270
		   ;;
	esac
	if [[ "$C" != "R" && "$C" != "L" ]]; then
		case "$REAL" in
			0) ((Y+=A))
			   ;;
			90) ((X+=A))
				;;
			180) ((Y-=A))
				 ;;
			270) ((X-=A))
				 ;;
		esac
	fi
	#printf "D=%3d POS=(%d,%d)\n" "$DIRECTION" "$X" "$Y"
done

((X<0)) && ((X=-X))
((Y<0)) && ((Y=-Y))
printf "%s : res=%d\n" "$CMD" $((X+Y))

exit 0
